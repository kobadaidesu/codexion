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