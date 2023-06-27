#ifndef MUTEX_H_
#define MUTEX_H_

#include <iostream>
//#if defined(_WIN32) || defined(_MSC_VER)
//#ifdef DEEPNETAPI_EXPORT
//
//#define DEEPNETAPI __declspec(dllexport)
//#else
//#define DEEPNETAPI __declspec(dllimport)
//#endif
//
//#elif defined(__linux__) || defined(__APPLE__)
//#ifdef DEEPNETAPI_EXPORT
//#define DEEPNETAPI __attribute__((visibility("default")))
//#else
//#define DEEPNETAPI
//#endif // DEEPNETAPI_EXPORT
//#include <pthread.h>
//
//#endif

#include "Tid.h"
#include "src/utils/TimeUtils.h"

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
		TID_T _holder;
		pthread_mutex_t _mutex{};

	public:
		Mutex() { pthread_mutex_init(&_mutex, nullptr); };

		~Mutex() { pthread_mutex_destroy(&_mutex); };

		void lock()
		{
			pthread_mutex_lock(&_mutex);
			assignHolder();
		};

		void unlock()
		{
			unassignHolder();
			pthread_mutex_unlock(&_mutex);
		};

		pthread_mutex_t *getPthreadMutex() { return &_mutex; };

		void assignHolder() { _holder = gettidv1(); };

		void unassignHolder() { _holder = nullptr; };
	};

	class Condition
	{
	private:
		Mutex &_mutex;
		pthread_cond_t _cond{};

	public:
		explicit Condition(Mutex &mutex) : _mutex(mutex) { pthread_cond_init(&_cond, nullptr); };

		~Condition() { pthread_cond_destroy(&_cond); };

		int wait()
		{
			return pthread_cond_wait(&_cond, _mutex.getPthreadMutex());
		}

		int wait(long millis)
		{
			struct timeval now{};
			struct timespec wakeup_time{};

			TimeUtils::getCurrentTimeInStruct(now);
			now.tv_usec = now.tv_usec + millis * 1000;
			now.tv_sec = now.tv_sec + now.tv_usec / 1000000;
			now.tv_usec = now.tv_usec % 1000000;

			wakeup_time.tv_nsec = now.tv_usec * 1000;
			wakeup_time.tv_sec = now.tv_sec;
			// std::cout << " millis:" << millis
			//<< "   wakeup time:sec:" << wakeup_time.tv_sec << "  nsec:" << wakeup_time.tv_nsec << std::endl;

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

//	class lock_guard
//	{
//		Mutex &mutex_;
//
//	public:
//		explicit lock_guard(Mutex &mutex) : mutex_(mutex) { mutex_.lock(); };
//
//		~lock_guard() { mutex_.unlock(); };
//	};
} // namespace nacos

#endif
