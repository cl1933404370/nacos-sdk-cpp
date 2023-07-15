#include <exception>
#include "ThreadPool.h"

#include <thread>

#include "Task.h"

using namespace std;

namespace nacos
{
    DummyTask ThreadPool::_dummyTask;

    void* ThreadPool::runInThread(void* param)
    {
        auto* thisobj = static_cast<ThreadPool*>(param);

        log_debug("ThreadPool::runInThread()\n");
        while (!thisobj->_stop)
        {
            Task* t = thisobj->take();
            NacosString taskName = t->getTaskName();
            log_debug("Thread got task:%s\n", taskName.c_str());
            try
            {
                t->run();
            }
            catch (exception& e)
            {
                log_error("Exception happens when executing:\n");
                log_error("Thread pool Name:%s Task name:%s\n", thisobj->_poolName.c_str(), taskName.c_str());
                log_error("Raison:%s", e.what());
            }
            catch (...)
            {
                log_error("Unknown exception happens when executing:\n");
                log_error("Thread pool Name:%s Task name:%s\n", thisobj->_poolName.c_str(), taskName.c_str());
                throw;
            }
            log_debug("Thread finished task:%s without problem\n", taskName.c_str());
        }

        return nullptr;
    }

    Task* ThreadPool::take()
    {
#ifdef  _WIN32 //|| _MSC_VER
        std::unique_lock<std::mutex> lock(_lock);
#else
    LockGuard _lockGuard(_lock);
#endif

        while (_taskList.empty() && !_stop)
        {
            _NotEmpty.wait(lock);
        }

        if (!_taskList.empty())
        {
            Task* curTask = std::move(_taskList.front());
            _taskList.pop_front();
            lock.unlock()();
            _NotFull.notify_one();
            return curTask;
        }
        if (_stop)
        {
            return nullptr;
        }

        return &_dummyTask;
    }

    void ThreadPool::put(Task* t)
    {
        {
            std::unique_lock<std::mutex> lock(_lock);
            log_debug("ThreadPool:::::taskList:%d poolSize:%d stop:%d\n", _taskList.size(), _poolSize, _stop);
            while (!(_taskList.size() < _poolSize) && !_stop)
            {
                _NotFull.wait(lock);
            }
            lock.unlock();
            if (!_stop)
            {
                {
                    lock.lock();
                    _taskList.push_back(std::move(t));
                }
                _NotEmpty.notify_one();
                return;
            }
        }

        //The thread pool is stopped, we need to run it locally
        log_debug("Running locally since the threadpool is stopped\n");
        t->run();
    };

    void ThreadPool::start()
    {
        log_warn("ThreadPool::start() start\n");
        if (!_stop)
        {
            log_warn("Thread pool named '%s' is started multiple times\n", _poolName.c_str());
            return;
        }

        _stop = false;
        for (size_t i = 0; i < _poolSize; i++)
        {
            auto currentThread = new Thread(_poolName + "-poolthread-" + NacosStringOps::valueOf(i), runInThread, this);
            _threads.push_back(currentThread);
            currentThread->start();
        }
    };

    void ThreadPool::stop()
    {
        if (_stop)
        {
            return;
        }

        _stop = true;
        _NotEmpty.notify_all();
        _NotFull.notify_all();

        for (auto&& _thread : _threads)
        {
            _thread->join();
            delete _thread;
        }

        _threads.clear();
    };
} //namespace nacos
