#ifndef __THREAD_POOL_H_
#define __THREAD_POOL_H_

#include <list>
#include <deque>
#include "Thread.h"
#include "Task.h"
#include "NacosString.h"
//#include "src/thread/Mutex.h"
//#ifdef _WIN32 || _MSC_VER
//#include <thread>
//#include <memory>
//#include <mutex>
//#endif


namespace nacos {
    class DummyTask : public Task {
    public:
        DummyTask() { setTaskName("DummyTask"); }

        void run() override {}
    };

    class ThreadPool {
        NacosString _poolName;
        std::deque<Task*> _taskList;
        mutable  Mutex _lock;
        mutable  Condition _NotEmpty;
        mutable Condition _NotFull;
        static DummyTask _dummyTask;

        static void* runInThread(void* param);

        ThreadPool() :
            _poolName("CannotBeCreated"),_NotEmpty(&_lock), _NotFull(&_lock), _stop(true), _poolSize(0) {}
    protected:
        std::list<Thread*> _threads;
        volatile bool _stop;
        size_t _poolSize;
    public:
        ThreadPool(NacosString poolName, size_t poolSize) :
            _poolName(std::move(poolName)), _NotEmpty(&_lock), _NotFull(&_lock), _stop(true), _poolSize(poolSize) {
        } 

        ThreadPool(size_t poolSize) :
            _poolName("NacosCliWorkerThread"), _NotEmpty(&_lock), _NotFull(&_lock), _stop(true), _poolSize(poolSize) {
        }

        virtual ~ThreadPool() = default;

        Task* take();

        void put(Task* t);

        virtual void start();

        virtual void stop();
    };

}//namespace
#endif