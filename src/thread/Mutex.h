#ifndef MUTEX_H_
#define MUTEX_H_

#include <iostream>
#include "Tid.h"
#include "src/utils/TimeUtils.h"
#if defined(_WIN32) || defined(_MSC_VER)
#include  <mutex>
#include <condition_variable>
#include <memory>
#include "port.h"

#endif


/*
 * Mutex.h
 * Author: Liu, Hanyu
 * Thanks to Shuo, Chen's muduo:
 * https://github.com/chenshuo/muduo/blob/master/muduo/base/Mutex.h
 */

namespace nacos
{
    typedef pthread_t TID_T;

#ifdef _WIN32
    static const bool kLittleEndian = !LEVELDB_IS_BIG_ENDIAN;

    class Condition;

    // Thinly wraps std::mutex.
    class LOCKABLE Mutex {
    public:
        Mutex() = default;
        ~Mutex() = default;

        Mutex(const Mutex&) = delete;
        Mutex& operator=(const Mutex&) = delete;

        void lock() EXCLUSIVE_LOCK_FUNCTION() { mu_.lock(); }
        void unlock() UNLOCK_FUNCTION() { mu_.unlock(); }
        void AssertHeld() ASSERT_EXCLUSIVE_LOCK() {}

    private:
        friend class Condition;
        std::mutex mu_;
    };

    // Thinly wraps std::condition_variable.
    class Condition {
    public:
        explicit Condition(Mutex* mu) : mu_(mu) { assert(mu != nullptr); }
        ~Condition() = default;

        Condition(const Condition&) = delete;
        Condition& operator=(const Condition&) = delete;

         void wait() {
            std::unique_lock<std::mutex> lock(mu_->mu_, std::adopt_lock);
            cv_.wait(lock);
            lock.release();
        }

        void wait(uint64_t millis)
        {
            std::unique_lock<std::mutex> lock(mu_->mu_, std::adopt_lock);
            cv_.wait_for(lock, std::chrono::milliseconds(millis));
            lock.release();
        }

        template <typename Predicate>
        void wait(Predicate pred) const {                 // (10)
            std::unique_lock<std::mutex> lock(mu_->mu_, std::adopt_lock);
            cv_.wait(lock, pred);
            lock.release();
        }

        template <typename Predicate>
        void wait(Predicate pred, long millis) const {  // (11)
            std::unique_lock<std::mutex> lock(mu_->mu_, std::adopt_lock);
            cv_.wait_for(lock, std::chrono::milliseconds(millis), pred);
            lock.release();
        }


        void notify() { cv_.notify_one(); }
        void notifyAll() { cv_.notify_all(); }

    private:
        std::condition_variable cv_;
        Mutex* const mu_;
    };

    class SCOPED_LOCKABLE LockGuard {
    public:
        explicit LockGuard(Mutex* mu) EXCLUSIVE_LOCK_FUNCTION(mu) : mu_(mu) {
            this->mu_->lock();
        }
        ~LockGuard() UNLOCK_FUNCTION() { this->mu_->unlock(); }

        LockGuard(const LockGuard&) = delete;
        LockGuard& operator=(const LockGuard&) = delete;

    private:
        Mutex* const mu_;
    };
#elif

    class Mutex
    {
        friend class Condition;
    private:
        TID_T _holder{};
        pthread_mutex_t _mutex = pthread_mutex_t{};
    public:
        Mutex() {
            pthread_mutex_init(&_mutex, nullptr);
        };

        ~Mutex() {
            pthread_mutex_destroy(&_mutex);
        }
        void lock()
        {
            pthread_mutex_lock(&_mutex);
            assignHolder();
        }

        void unlock()
        {
            unassignHolder();
            pthread_mutex_unlock(&_mutex);
        };

        pthread_mutex_t* getPthreadMutex() { return &_mutex; };
        void assignHolder() { _holder = gettidv1(); };
        void unassignHolder() { _holder = nullptr; };
    };

    class Condition
    {
    private:
        Mutex& _mutex;

        pthread_cond_t _cond{};
    public:
        explicit Condition(Mutex& mutex) : _mutex(mutex)
        {
            pthread_cond_init(&_cond, nullptr);
        };

        ~Condition() {
            pthread_cond_destroy(&_cond);
        };

        int wait()
        {
            _mutex.unassignHolder();
            return pthread_cond_wait(&_cond, _mutex.getPthreadMutex());
        }

        int wait(long millis)
        {
            struct timeval now
            {
            };
            struct timespec wakeup_time
            {
            };

            TimeUtils::getCurrentTimeInStruct(now);
            now.tv_usec = now.tv_usec + millis * 1000;
            now.tv_sec = now.tv_sec + now.tv_usec / 1000000;
            now.tv_usec = now.tv_usec % 1000000;

            wakeup_time.tv_nsec = now.tv_usec * 1000;
            wakeup_time.tv_sec = now.tv_sec;
            // std::cout << " millis:" << millis
            //<< "   wakeup time:sec:" << wakeup_time.tv_sec << "  nsec:" << wakeup_time.tv_nsec << std::endl;
            _mutex.unassignHolder();
            return pthread_cond_timedwait(&_cond, _mutex.getPthreadMutex(), &wakeup_time);
        }
        void notify()
        {

            pthread_cond_signal(&_cond);
        }

        void notifyAll()
        {

            pthread_cond_broadcast(&_cond);
        }
    };

    class LockGuard
    {
        Mutex& mutex_;

    public:
        explicit LockGuard(Mutex& mutex) : mutex_(mutex)
        {
            mutex_.lock();
        };

        ~LockGuard()
        {

            mutex_.unlock();
        };
    };
#endif
} // namespace nacos

#endif
