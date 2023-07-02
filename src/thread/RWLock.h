#ifndef __RWLOCK_H_
#define __RWLOCK_H_

#if defined(__linux__)
#include <pthread.h>
#include <unistd.h>
#elif defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#include <folly/Portability.h>
#include <folly/portability/pthread.h>
#endif

/*
* Mutex.h
* Author: Liu, Hanyu
* An encapsulation of pthread_rwlock
*/

namespace nacos{
class RWLock {
private:
    pthread_rwlock_t _pthread_rwlock;
public:
    RWLock() { pthread_rwlock_init(&_pthread_rwlock, NULL); };

    ~RWLock() { pthread_rwlock_destroy(&_pthread_rwlock); };

    int unlock() { return pthread_rwlock_unlock(&_pthread_rwlock); };

    int readLock() { return pthread_rwlock_rdlock(&_pthread_rwlock); };

    int writeLock() { return pthread_rwlock_wrlock(&_pthread_rwlock); };
};

class ReadGuard {
private:
    RWLock &_rwLock;
public:
    ReadGuard(RWLock &rwLock) : _rwLock(rwLock) {
        _rwLock.readLock();
    }

    ~ReadGuard() { _rwLock.unlock(); }
};

class WriteGuard {
private:
    RWLock &_rwLock;
public:
    WriteGuard(RWLock &rwLock) : _rwLock(rwLock) {
        _rwLock.writeLock();
    }

    ~WriteGuard() { _rwLock.unlock(); }
};
}//namespace nacos

#endif