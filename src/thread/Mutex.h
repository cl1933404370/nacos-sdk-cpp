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
#if defined(_WIN32) || defined(_MSC_VER)
		struct UnlockAndNotify {
			std::mutex d_mutex;
			std::condition_variable d_condition;

			void lock() { d_mutex.lock(); }
			void unlock() { d_mutex.unlock();}

			void notify_one() {  d_condition.notify_one();}
			void notify_all() {  d_condition.notify_all(); }


		};
#endif
		friend class Condition;
	private:
		TID_T _holder{};
#if defined(_WIN32) || defined(_MSC_VER)
		UnlockAndNotify d_combined;
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
			const auto lock = makeLockWithNotify();
		}
#else
		void lock()
		{b
			pthread_mutex_lock(&_mutex);
			assignHolder();
		}
#endif

		void unlock()
		{
			unassignHolder();
#if defined(_WIN32) || defined(_MSC_VER)
			//d_combined.unlock();
#else
			pthread_mutex_unlock(&_mutex);
#endif
		};

#if defined(_WIN32) || defined(_MSC_VER)
    std::unique_lock<UnlockAndNotify> makeLockWithNotify() {
        return std::unique_lock{d_combined};
    }

    template <typename PRED>
    std::unique_lock<std::mutex> makeLockWithWait(PRED waitForCondition) {
        std::unique_lock lock{d_combined.d_mutex};
        d_combined.d_condition.wait(lock, waitForCondition);
        return lock;
    }

	template <typename PRED>
    std::unique_lock<std::mutex> makeLockWithWait(PRED waitForCondition, long millis) {
        std::unique_lock lock{d_combined.d_mutex};
        d_combined.d_condition.wait_for(lock, std::chrono::milliseconds(millis), waitForCondition);
        return lock;
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
		int wait(PRED waitForCondition) {
			const auto lock =_mutex.makeLockWithWait(waitForCondition);
			return 0;
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
		int wait(PRED waitForCondition, long millis) {
			// std::cout << " millis:" << millis
			//<< "   wakeup time:sec:" << wakeup_time.tv_sec << "  nsec:" << wakeup_time.tv_nsec << std::endl;
			const auto lock = _mutex.makeLockWithWait(waitForCondition, millis);
			return 0;
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
			_mutex.d_combined.notify_one();;
#else
			pthread_cond_signal(&_cond);
#endif
		}

		void notifyAll()
		{

#if defined(_WIN32) || defined(_MSC_VER)
			_mutex.d_combined.notify_all();
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
			const auto aa = mutex_.makeLockWithNotify();
#else
			mutex_.lock();
#endif
		};

		~LockGuard()
		{

#if defined(_WIN32) || defined(_MSC_VER)
#else
			mutex_.unlock();
#endif
		};
	};
} // namespace nacos

#endif
