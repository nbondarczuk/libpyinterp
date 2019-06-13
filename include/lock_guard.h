#ifndef _LOCK_GUARD_H_
#define _LOCK_GUARD_H_

#include <pthread.h>

template <typename T> struct LockGuard
{
    LockGuard(T* m): mutex(m) {
        pthread_mutex_lock(mutex);
    }

    ~LockGuard() {
        pthread_mutex_unlock(mutex);
    }

private:
    T* mutex;
};

#endif /* _LOCK_GUARD_H_ */
