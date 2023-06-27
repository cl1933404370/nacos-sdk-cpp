#ifndef __MUTEX_H_
#define __MUTEX_H_

#include<iostream>

#if defined(_WIN32) || defined(_MSC_VER)  //��������˱���_WIN32����_MSC_VER������winƽ̨
#ifdef DEEPNETAPI_EXPORT  // ���������DEEPNETAPI_EXPORT������������ڵ�������������˵��һ��û�ж���������������Ի����else���ֵ����ݣ������ڿ�������������˵����Ҫ�������������ã�������Ҫ�Լ������������������ͨ��-DDEEPNETAPI_EXPORT������ñ���

#define DEEPNETAPI __declspec(dllexport)   // �Կ⿪������˵��������仰��Ŀ���ǵ����ÿ⣬���ɶ�Ӧ��so������ʹ�ã����Խе���export
#else
#define DEEPNETAPI __declspec(dllimport)  // ���ڿ��ʹ������˵��������仰����������⵼�뵽�Լ��Ĺ������棬���Խе���import
#endif // DEEPNETAPI_EXPORT
#include <windows.h>
#include <thread>
#include <mutex>
#include <shared_mutex>
#include <condition_variable>
#include <memory>

#elif defined(__linux__) || defined(__APPLE__)  // ����linux��ƻ��ϵͳ��������Ļ�
#ifdef DEEPNETAPI_EXPORT
#define DEEPNETAPI __attribute__ ((visibility ("default")))   // �Կ⿪������˵��ֻϣ����һЩ��Ҫ�����ĺ����������ʹ���ߣ������Ľ������أ������ڵ������ʱ����Ҫ�ڱ���ѡ�������-fvisibility=hidden��ֻ�к���������ǰ����__attribute__ ((visibility ("default")))�Ŀ��Ա�������ʹ�ã�������ȫ�����ء�ͨ���ڱ���ѡ�������-fvisibility=hidden������������������أ�ֻ��__attribute__ ((visibility ("default")))�ĺ����ᱣ��Ĭ�ϣ���ȫ�ֿɼ����������ʱ����-fvisibility=hidden���򲻹ܺ���������ǰ�Ƿ����__attribute__ ((visibility ("default")))������ȫ�ֿɼ�����ΪĬ����default
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
