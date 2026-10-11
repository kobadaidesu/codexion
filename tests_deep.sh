#!/bin/bash
# codexion deep test battery — 評価シート・ピアレビューで実際に出た指摘の検出器集
# 使い方: ./tests_deep.sh
cd "$(dirname "$0")" || exit 1
PASS=0; FAIL=0
ok()  { PASS=$((PASS+1)); printf "  \033[32mOK\033[0m  %s\n" "$1"; }
ng()  { FAIL=$((FAIL+1)); printf "  \033[31mNG\033[0m  %s\n" "$1"; }
now_ms() { python3 -c 'import time; print(int(time.time()*1000))'; }

make re > /dev/null 2>&1 || { echo "build failed"; exit 1; }

echo "== 静的検査 =="
# グローバル変数(評価シート: 発見即終了)。nmのdata/bss/common定義シンボルが無いこと
g=$(nm -U codexion 2>/dev/null | awk '$2 ~ /^[DdBbCcSs]$/' | grep -cv "__")
[ "$g" = 0 ] && ok "グローバル変数なし(nm検査)" || ng "グローバル変数の疑い ${g}件: $(nm -U codexion | awk '$2 ~ /^[DdBbCcSs]$/' | grep -v __)"
# 許可外関数(denylist)
bad=$(grep -nE '\b(fork|sem_open|sem_wait|sem_post|sleep|nanosleep|pthread_detach|exit)\s*\(' src/*.c | grep -v usleep | wc -l | tr -d ' ')
[ "$bad" = 0 ] && ok "許可外関数の呼び出しなし" || ng "許可外関数 ${bad}件"

echo "== ログの厳格検証(200人・混線/形式/時刻逆行) =="
out=$(./codexion 200 5000 60 60 60 2 0 edf)
badfmt=$(echo "$out" | grep -vcE '^[0-9]+ [0-9]+ (has taken a dongle|is compiling|is debugging|is refactoring|burned out)$')
[ "$badfmt" = 0 ] && ok "200人x403thread: 形式違反0行" || ng "形式違反 ${badfmt}行"
mono=$(echo "$out" | awk '{if ($1 < p) bad++; p=$1} END{print bad+0}')
[ "$mono" = 0 ] && ok "タイムスタンプの逆行なし" || ng "逆行 ${mono}件"

echo "== burnout精度(要件: 実際のburnoutから10ms以内) =="
prec=1
for i in 1 2 3 4 5; do
  ts=$(./codexion 1 800 200 200 200 3 0 fifo | tail -1 | awk '{print $1}')
  [ "$ts" -ge 800 ] && [ "$ts" -le 810 ] || prec=0
done
[ $prec = 1 ] && ok "5回連続 800〜810ms以内" || ng "10ms超過あり"

echo "== burnoutは compile開始 起点(終了ではない) =="
# 2人: 先行者はt=0にcompile開始、deadline=250。compile(200ms)完了後も次を開始できず250で死ぬ
ts=$(./codexion 2 250 200 200 200 5 0 fifo | tail -1 | awk '/burned out/{print $1}')
[ -n "$ts" ] && [ "$ts" -ge 250 ] && [ "$ts" -le 262 ] && ok "開始+250msちょうどでburnout($ts)" || ng "期限計算が開始起点でない疑い($ts)"

echo "== cooldownの実効性(ピアで実際に出たバグの検出器) =="
# 2人x400ms: 各coderのcompile間隔は 2x(100+400)=1000ms を下回れないはず
viol=$(./codexion 2 5000 100 100 100 3 400 fifo | awk '$4=="compiling"{if(last[$2] && $1-last[$2] < 985) v++; last[$2]=$1} END{print v+0}')
[ "$viol" = 0 ] && ok "compile間隔が理論下限(1000ms)を下回らない" || ng "cooldown無視の疑い ${viol}件"
# 物理限界ケース(需要>供給): 生き残ったらcooldown未実施のバグ
last=$(./codexion 5 3000 200 200 200 10 800 edf | tail -1)
echo "$last" | grep -q "burned out" && ok "実現不可能条件で正しくburnout" || ng "不可能条件を生存=cooldown未実施の疑い"

echo "== burnout後の即時終了(ピアは~800ms遅延) =="
t0=$(now_ms); ./codexion 2 100 200 200 200 3 0 fifo > /dev/null; t1=$(now_ms)
dur=$((t1 - t0))
[ "$dur" -lt 500 ] && ok "burnout(100ms)から全体${dur}msで終了" || ng "終了ラグ大(全体${dur}ms)"

echo "== dongle重複・cooldown違反の計装検査(holder二重割当の検出) =="
TMP=$(mktemp -d) && cp -r src includes "$TMP/" && python3 - "$TMP" << 'EOF'
import sys
d = sys.argv[1]
p = f"{d}/src/dongle.c"
s = open(p).read()
s = s.replace("\t\t\tdongle->holder = coder->id;",
  "\t\t\tdongle->holder = coder->id;\n\t\t\tfprintf(stderr, \"T %lld %d\\n\", get_time_us(), dongle->id);")
open(p, 'w').write(s)
p = f"{d}/src/dongle.c"
s = open(p).read()
s = s.replace("\t\tpthread_cond_broadcast(&dongle->cond);",
  "\t\tpthread_cond_broadcast(&dongle->cond);\n\t\tfprintf(stderr, \"R %lld %d\\n\", now, dongle->id);")
open(p, 'w').write(s)
EOF
cc -Wall -Wextra -Werror -pthread -I"$TMP/includes" "$TMP"/src/*.c -o "$TMP/dbg" \
  && "$TMP/dbg" 5 3000 100 100 100 3 300 edf > /dev/null 2> "$TMP/trace" \
  && res=$(sort -s -n -k2,2 -k1,1 "$TMP/trace" | awk -v cd=300000 '
      $1=="T" { if (held[$3]) dup++; held[$3]=1;
                if (lr[$3] != "" && $2 - lr[$3] < cd) cv++ }
      $1=="R" { held[$3]=0; lr[$3]=$2 }
      END { print dup+0, cv+0 }')
dup=$(echo $res | cut -d' ' -f1); cv=$(echo $res | cut -d' ' -f2)
[ "$dup" = 0 ] && ok "holderの二重割当 0件" || ng "dongle重複 ${dup}件"
[ "$cv" = 0 ] && ok "cooldown中の取得 0件(300ms厳守)" || ng "cooldown違反 ${cv}件"
rm -rf "$TMP"

echo "== heap単体(EDF tie-break = recode対象の確認) =="
TMP2=$(mktemp -d)
cat > "$TMP2/t.c" << 'EOF'
#include "codexion.h"
#include <assert.h>
int main(void)
{
	t_dongle d;

	d.wait_count = 0;
	d.next_ticket = 0;
	heap_push(&d, 5, 0, FIFO);
	heap_push(&d, 6, 0, FIFO);
	assert(heap_top_id(&d) == 5);
	heap_pop(&d, FIFO);
	assert(heap_top_id(&d) == 6);
	d.wait_count = 0;
	heap_push(&d, 1, 900, EDF);
	heap_push(&d, 2, 500, EDF);
	assert(heap_top_id(&d) == 2);
	d.wait_count = 0;
	heap_push(&d, 2, 700, EDF);
	heap_push(&d, 1, 700, EDF);
	assert(heap_top_id(&d) == 1);
	return (0);
}
EOF
cc -Wall -Wextra -Werror -Iincludes "$TMP2/t.c" src/heap.c -o "$TMP2/t" && "$TMP2/t" \
  && ok "FIFO到着順/EDF期限順/同deadlineはid小優先" || ng "heap順序"
rm -rf "$TMP2"

echo "== 入力の際どいケース =="
./codexion 2 +5 60 60 60 1 0 fifo > /dev/null 2>&1;  [ $? = 1 ] && ok "'+5' を拒否(数字のみ)" || ng "'+5'"
./codexion 2 0300 60 60 60 1 0 fifo > /dev/null 2>&1; [ $? = 0 ] && ok "前ゼロ'0300'は300として受理" || ng "前ゼロ"
./codexion 2 2147483647 60 60 60 1 0 fifo > /dev/null 2>&1; [ $? = 0 ] && ok "INT_MAXちょうどは受理" || ng "INT_MAX境界"

echo
echo "結果: PASS=$PASS FAIL=$FAIL"
[ $FAIL = 0 ]
