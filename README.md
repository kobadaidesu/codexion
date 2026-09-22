./codexion \
number_of_coders \
time_to_burnout \
time_to_compile \
time_to_debug \
time_to_refactor \
number_of_compiles_required \
dongle_cooldown \
scheduler

coder はsim の情報を知っておきたいから参照できるように
コピーじゃなくて参照にするのはmutex関連の話

coffman conditionts
dongle idの小さい方から大きい方に取得することによってCirculer Waitを解消してdeadlockを防ぐ

Coder
  │
  ↓
Heapに登録
  │
  ↓
dongle lock
  │
  ↓
取れる？
  │
 ┌┴─────────────┐
YES              NO
 │                │
Heap pop       なぜ取れない？
 │                │
holder=id      ┌──┴─────────────┐
 │          cooldown           その他
unlock           │               │
 │          timedwait         cond_wait
取得              │               │
                 └──────┬────────┘
                        ↓
                       寝る
                        ↓
              release/broadcast
              またはtimeout
                        ↓
                     起きる
                        ↓
                    再チェック

各dongleにはmutexとpriority queueがあります。

coderがdongleを要求するとFIFOなら到着順、
EDFならburnout deadline順でheapに追加します。

heapの先頭で、dongleが空いていて、
cooldownも終了しているcoderだけが取得できます。

取得できないthreadはcondition variableでsleepし、
dongleの解放時にはbroadcastで起こします。

cooldown待ちの場合はpthread_cond_timedwaitを使います。