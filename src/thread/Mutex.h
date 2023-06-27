#ifndef __MUTEX_H_
#define __MUTEX_H_

#include<iostream>

#if defined(_WIN32) || defined(_MSC_VER)  //如果定义了变量_WIN32或者_MSC_VER，即在win平台
#ifdef DEEPNETAPI_EXPORT  // 如果定义了DEEPNETAPI_EXPORT这个变量，对于调用这个库的人来说，一般没有定义这个变量，所以会调用else部分的内容，但对于开发这个库的人来说，需要把这个库给别人用，所以需要自己定义这个变量，可以通过-DDEEPNETAPI_EXPORT来定义该变量

#define DEEPNETAPI __declspec(dllexport)   // 对库开发者来说，调用这句话，目的是导出该库，生成对应的so给别人使用，所以叫导出export
#else
#define DEEPNETAPI __declspec(dllimport)  // 对于库的使用者来说。调用这句话，即把这个库导入到自己的工程里面，所以叫导入import
#endif // DEEPNETAPI_EXPORT
#include <windows.h>
#include <thread>
#include <mutex>
#include <shared_mutex>
#include <condition_variable>
#include <memory>

#elif defined(__linux__) || defined(__APPLE__)  // 对于linux和苹果系统调用下面的话
#ifdef DEEPNETAPI_EXPORT
#define DEEPNETAPI __attribute__ ((visibility ("default")))   // 对库开发者来说，只希望把一些需要公开的函数或者类给使用者，其他的进行隐藏，所以在导出库的时候，需要在编译选项上添加-fvisibility=hidden，只有函数或者类前加了__attribute__ ((visibility ("default")))的可以被调用者使用，其他的全部隐藏。通过在编译选项上添加-fvisibility=hidden则把其他函数进行隐藏，只有__attribute__ ((visibility ("default")))的函数会保持默认，即全局可见，如果编译时不加-fvisibility=hidden，则不管函数或者类前是否添加__attribute__ ((visibility ("default")))，都是全局可见，因为默认是default
#else
#define DEEPNETAPI
#endif // DEEPNETAPI_EXPORT
#include <pthread.h>

#endif

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
#if defined(_WIN32) || defined(_MSC_VER)
	using TID_T = std::thread::id;

	class mutex
	{
		friend class Condition;

	private:
		TID_T _holder;
		std::shared_mutex _cs;

	public:
		mutex()
		{
			_holder = std::thread::id();
		}

		~mutex() = default;

		void lock()
		{
			_cs.lock();
			assignHolder();
		};

		void unlock()
		{
			_cs.unlock();
			_holder = std::thread::id();
		};

		std::shared_mutex* get_pthread_mutex() { return &_cs; };

		void assignHolder() { _holder = std::this_thread::get_id(); };

		void un_assign_holder() { _holder = std::thread::id(); };
	};

class Condition
	{
private:
		mutex& mutex_;
		std::condition_variable_any cond_;
		std::atomic_bool action_;
public:
	explicit Condition(mutex& mutex) : mutex_(mutex)
	{
	};

	~Condition() = default;

	int wait()
	{
		std::scoped_lock lock(*mutex_.get_pthread_mutex());
		try
		{
			action_ = false;
			cond_.wait(lock,[&,this]()->bool{return action_;});
			return true;
		}
		catch (...)
		{
			return false;
		}
	}

	int wait(long millis) {
		std::scoped_lock lock(*mutex_.get_pthread_mutex());
		try
		{
			action_ = false;
			cond_.wait_for(lock, std::chrono::milliseconds(millis),[&,this]()->bool{return action_;});
			return true;
		}
		catch (...)
		{
			return false;
		}
	}

	void notify()
	{
		{
			std::scoped_lock lock(*mutex_.get_pthread_mutex());
			action_ = true;
		}
		cond_.notify_one();
	}

	void notifyAll()
	{
		{
			std::scoped_lock lock(*mutex_.get_pthread_mutex());
			action_ = true;
		}
		cond_.notify_all();
	}
};


#elif defined(__linux__) || defined(__APPLE__)
	typedef pthread_t TID_T;
	class Mutex {
		friend class Condition;

	private:
		TID_T _holder;
		pthread_mutex_t _mutex;
	public:
		Mutex() { pthread_mutex_init(&_mutex, NULL); };

		~Mutex() { pthread_mutex_destroy(&_mutex); };

		void lock() {
			pthread_mutex_lock(&_mutex);
			assignHolder();
		};

		void unlock() {
			unassignHolder();
			pthread_mutex_unlock(&_mutex);
		};

		pthread_mutex_t* getPthreadMutex() { return &_mutex; };

		void assignHolder() { _holder = gettidv1(); };

		void unassignHolder() { _holder = 0; };
	};

	class Condition {
	private:
		Mutex& _mutex;
		pthread_cond_t _cond;
	public:
		Condition(Mutex& mutex) : _mutex(mutex) { pthread_cond_init(&_cond, NULL); };

		~Condition() { pthread_cond_destroy(&_cond); };

		int wait() {
			return pthread_cond_wait(&_cond, _mutex.getPthreadMutex());
		}

		int wait(long millis) {
			struct timeval now;
			struct timespec wakeup_time;

			TimeUtils::getCurrentTimeInStruct(now);
			now.tv_usec = now.tv_usec + millis * 1000;
			now.tv_sec = now.tv_sec + now.tv_usec / 1000000;
			now.tv_usec = now.tv_usec % 1000000;

			wakeup_time.tv_nsec = now.tv_usec * 1000;
			wakeup_time.tv_sec = now.tv_sec;
			//std::cout << " millis:" << millis
			//<< "   wakeup time:sec:" << wakeup_time.tv_sec << "  nsec:" << wakeup_time.tv_nsec << std::endl;

			return pthread_cond_timedwait(&_cond, _mutex.getPthreadMutex(), &wakeup_time);
		}

		void notify() {
			pthread_cond_signal(&_cond);
		}

		void notifyAll() {
			pthread_cond_broadcast(&_cond);
		}
	};

#endif

	class lock_guard
	{
		mutex& mutex_;
	public:
		explicit lock_guard(mutex& mutex) : mutex_(mutex) { mutex_.lock(); };

		~lock_guard() { mutex_.unlock(); };
	};
} //namespace nacos

#endif
