#ifndef MUTEX_H_
#define MUTEX_H_

#include <iostream>
#include "Tid.h"
#include "src/utils/TimeUtils.h"
#if defined(_WIN32) || defined(_MSC_VER)
#include  <mutex>
#include <condition_variable>
#include <memory>
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

    class Mutex
    {
        friend class Condition;
    private:
        TID_T _holder{};
#if defined(_WIN32) || defined(_MSC_VER)
        std::mutex _mutex;
#else
        pthread_mutex_t _mutex = pthread_mutex_t{};
#endif
    public:
        Mutex() {
#if defined(_WIN32) || defined(_MSC_VER)
#else
            pthread_mutex_init(&_mutex, nullptr);
#endif
        };

        ~Mutex() {

#if defined(_WIN32) || defined(_MSC_VER)
#else
            pthread_mutex_destroy(&_mutex);
#endif
        }

        

#if defined(_WIN32) || defined(_MSC_VER)
        void lock()
        {
            assignHolder();
            _mutex.lock();
        }
#else
        void lock()
        {
            pthread_mutex_lock(&_mutex);
            assignHolder();
        }
#endif
           
        void unlock()
        {
            
            unassignHolder();
#if defined(_WIN32) || defined(_MSC_VER)
            _mutex.unlock();
#else
            pthread_mutex_unlock(&_mutex);
#endif
        };

#if defined(_WIN32) || defined(_MSC_VER)
        std::mutex* getPthreadMutex()
        {
            return &_mutex;
        }
#else
        pthread_mutex_t* getPthreadMutex() { return &_mutex; };

#endif
        void assignHolder() { _holder = gettidv1(); };

        void unassignHolder() { _holder = nullptr; };
        };

    class Condition
    {
    private:
        Mutex& _mutex;

#if defined(_WIN32) || defined(_MSC_VER)
        std::condition_variable _cond;
#else
        pthread_cond_t _cond{};
#endif
    public:
        explicit Condition(Mutex& mutex) : _mutex(mutex)
        {
#if defined(_WIN32) || defined(_MSC_VER)
#else
            pthread_cond_init(&_cond, nullptr);
#endif
        };

        ~Condition() {
#if defined(_WIN32) || defined(_MSC_VER)
#else
            pthread_cond_destroy(&_cond);
#endif
        };

#if defined(_WIN32) || defined(_MSC_VER)
        template <typename PRED>
        std::unique_lock<std::mutex> wait(PRED waitForCondition) {
            std::unique_lock lock{*_mutex.getPthreadMutex()};
            _cond.wait(lock, waitForCondition);
            return lock;
        }
#else
        int wait()
        {
            _mutex.unassignHolder();
            return pthread_cond_wait(&_cond, _mutex.getPthreadMutex());
        }
#endif

#if defined(_WIN32) || defined(_MSC_VER)
        template <typename PRED>
        std::unique_lock<std::mutex> wait(PRED waitForCondition, long millis) {
            // std::cout << " millis:" << millis
            //<< "   wakeup time:sec:" << wakeup_time.tv_sec << "  nsec:" << wakeup_time.tv_nsec << std::endl;
            std::unique_lock lock{*_mutex.getPthreadMutex()};
            bool status = _cond.wait_for(lock, std::chrono::milliseconds(millis), waitForCondition);
            return  lock;
        }
#else
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
#endif

        void notify()
        {

#if defined(_WIN32) || defined(_MSC_VER)
            _cond.notify_one();
#else
            pthread_cond_signal(&_cond);
#endif
        }

        void notifyAll()
        {

#if defined(_WIN32) || defined(_MSC_VER)
            _cond.notify_all();
#else
            pthread_cond_broadcast(&_cond);
#endif
        }
    };

    class LockGuard
    {
        Mutex& mutex_;
#if defined(_WIN32) || defined(_MSC_VER)
#endif


    public:
        explicit LockGuard(Mutex& mutex) : mutex_(mutex)
        {
#if defined(_WIN32) || defined(_MSC_VER)
            if (1)
            {
                mutex_.lock();
            }
#else
            mutex_.lock();
#endif
        };

        ~LockGuard()
        {

#if defined(_WIN32) || defined(_MSC_VER)
            if (1)
            {
                mutex_.unlock();
            }
#else
            mutex_.unlock();
#endif
        };
    };
        } // namespace nacos

#endif
