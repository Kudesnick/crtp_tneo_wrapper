## todo

1. Реализовать  перегрузку `void *__user_perthread_libspace (void)`, `int _mutex_initialize(mutex *m)`, `void _mutex_acquire(mutex *m)`, `void _mutex_release(mutex *m)`, `void _mutex_free(mutex *m)`.
2. Использовать системный стек, как стек прерываний.
3. Вытащить idle в cpp конструктор.