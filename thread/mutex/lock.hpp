#include <pthread.h>

// 锁的基类, 便于对其进行后续扩展
class _base_lock_
{
protected:
    pthread_mutex_t *__mutex;
    _base_lock_(pthread_mutex_t *__lock_)
        : __mutex(__lock_)
    {
    }

    int lock()
    {
        return pthread_mutex_lock(__mutex);
    }

    int unlock()
    {
        return pthread_mutex_unlock(__mutex);
    }
};

class _lock final : public _base_lock_
{
public:
    _lock(pthread_mutex_t *_lock)
        : _base_lock_(_lock)
    {
        lock();
    }

    ~_lock()
    {
        unlock();
    }
};