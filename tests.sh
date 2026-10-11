#!/bin/bash
# codexion test battery
# 使い方: ./tests.sh        (通常テスト)
#         ./tests.sh full   (TSan・leaks込みのフルセット)
cd "$(dirname "$0")" || exit 1
PASS=0; FAIL=0
ok()  { PASS=$((PASS+1)); printf "  \033[32mOK\033[0m  %s\n" "$1"; }
ng()  { FAIL=$((FAIL+1)); printf "  \033[31mNG\033[0m  %s\n" "$1"; }

echo "== build =="
make re > /dev/null 2> /tmp/codexion_build.log
if [ -s /tmp/codexion_build.log ]; then ng "build (warnings/errors)"; cat /tmp/codexion_build.log; else ok "build -Wall -Wextra -Werror -pthread"; fi

echo "== 状態遷移(take x2 -> compiling -> debugging -> refactoring) =="
TRANS='
{ ts=$1; id=$2
  if (ts < prev) { bad=1 } ; prev=ts
  ev=$3; if ($3=="is") ev=$4; if ($3=="has") ev="taken"; if ($3=="burned") ev="burned"
  s=st[id]+0; okev=0
  if (ev=="taken" && s<2) { st[id]=s+1; okev=1 }
  else if (ev=="compiling" && s==2) { st[id]=3; okev=1 }
  else if (ev=="debugging" && s==3) { st[id]=4; okev=1 }
  else if (ev=="refactoring" && s==4) { st[id]=0; okev=1 }
  else if (ev=="burned") okev=1
  if (!okev) bad=1 }
END { exit bad }'
for s in fifo edf; do
  if ./codexion 5 3000 200 200 200 3 0 $s | awk "$TRANS"; then ok "遷移 $s"; else ng "遷移 $s"; fi
done

echo "== 完走系 =="
n=$(./codexion 5 3000 200 200 200 3 0 edf | awk '$4=="compiling"{c[$2]++} END{m=0;for(i in c)if(c[i]>=3)m++;print m}')
[ "$n" = 5 ] && ok "5人x3回 完走" || ng "5人x3回 ($n/5)"
n=$(./codexion 200 5000 60 60 60 2 0 fifo | awk '$4=="compiling"{c[$2]++} END{m=0;for(i in c)if(c[i]>=2)m++;print m}')
[ "$n" = 200 ] && ok "200人x2回 完走" || ng "200人 ($n/200)"

echo "== burnout系 =="
out=$(./codexion 2 100 200 200 200 3 0 fifo | tail -1)
case "$out" in 1??" "?" burned out"|"10"?" "?" burned out") ok "burnout 100ms ($out)";; *) ng "burnout 100ms ($out)";; esac
out=$(./codexion 1 800 200 200 200 3 0 fifo)
echo "$out" | grep -q "has taken a dongle" && echo "$out" | grep -qE "^80[0-9] 1 burned out" && ok "1人: take後800msでburnout" || ng "1人 ($out)"
out=$(./codexion 2 600 100 100 100 3 1000 fifo | tail -1)
echo "$out" | grep -q "burned out" && ok "cooldown過大でburnout" || ng "cooldown過大 ($out)"
last=$(./codexion 2 150 200 200 200 5 0 edf | tail -1)
echo "$last" | grep -q "burned out" && ok "最終行がburned out" || ng "burned out後にログ ($last)"

echo "== 入力検証 =="
./codexion 1 2 3 > /dev/null 2>&1;                       [ $? = 1 ] && ok "引数不足を拒否" || ng "引数不足"
./codexion 5 800 abc 200 200 3 0 fifo > /dev/null 2>&1;  [ $? = 1 ] && ok "非数値を拒否" || ng "非数値"
./codexion 5 -1 200 200 200 3 0 fifo > /dev/null 2>&1;   [ $? = 1 ] && ok "負数を拒否" || ng "負数"
./codexion 5 2147483648 200 200 200 3 0 fifo > /dev/null 2>&1; [ $? = 1 ] && ok "INT_MAX超を拒否" || ng "INT_MAX超"
./codexion 5 800 200 200 200 3 0 rr > /dev/null 2>&1;    [ $? = 1 ] && ok "不正schedulerを拒否" || ng "scheduler"
./codexion 0 800 200 200 200 3 0 fifo > /dev/null 2>&1;  [ $? = 1 ] && ok "0人を拒否" || ng "0人"
./codexion 2 300 60 60 60 0 0 fifo > /dev/null 2>&1;     [ $? = 0 ] && ok "required=0は即正常終了" || ng "required=0"

echo "== 安定性(fifo/edf 各10回) =="
hang=0
for s in fifo edf; do for i in $(seq 10); do
  ./codexion 5 3000 60 60 60 3 10 $s > /dev/null || hang=1
done; done
[ $hang = 0 ] && ok "20回連続完走" || ng "ハングまたは異常終了"

if [ "$1" = "full" ]; then
  echo "== leaks =="
  for args in "5 3000 100 100 100 2 50 edf" "2 100 200 200 200 3 0 fifo"; do
    if leaks --atExit -- ./codexion $args 2> /dev/null | grep -q "0 leaks"; then
      ok "leaks 0 ($args)"; else ng "leaks ($args)"; fi
  done
  echo "== ThreadSanitizer =="
  cc -Wall -Wextra -Werror -pthread -fsanitize=thread -g -Iincludes src/*.c -o /tmp/codexion_tsan 2> /dev/null
  w=$( (/tmp/codexion_tsan 5 3000 100 100 100 3 10 fifo; /tmp/codexion_tsan 5 3000 100 100 100 3 10 edf; \
        /tmp/codexion_tsan 2 100 200 200 200 3 0 edf; /tmp/codexion_tsan 1 800 200 200 200 3 0 fifo) 2>&1 \
        | grep -c "WARNING: ThreadSanitizer")
  [ "$w" = 0 ] && ok "data race 0件" || ng "data race ${w}件"
fi

echo
echo "結果: PASS=$PASS FAIL=$FAIL"
[ $FAIL = 0 ]
