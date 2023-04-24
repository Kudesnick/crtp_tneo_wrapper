## todo

1. перегрузка `void *__user_perthread_libspace (void)`, `int _mutex_initialize(mutex *m)`, `void _mutex_acquire(mutex *m)`, `void _mutex_release(mutex *m)`, `void _mutex_free(mutex *m)`.
2. обертки для коллбеков (в т.ч. dynamic tick) из tn_sys.h
3. работа round_robin в режиме dynamic tick
4. создание мютекса до запуска ОС