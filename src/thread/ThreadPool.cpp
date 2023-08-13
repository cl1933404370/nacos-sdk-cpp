#include <exception>
#include "ThreadPool.h"
#include "Task.h"

using namespace std;

namespace nacos{
DummyTask ThreadPool::_dummyTask;

void *ThreadPool::runInThread(void *param) {
    ThreadPool *thisobj = static_cast<ThreadPool*>(param);

    log_debug("ThreadPool::runInThread()\n");
    Task * t = nullptr;
    while ((t = thisobj->take())) {
        NacosString taskName = t->getTaskName();
        log_debug("Thread got task:%s\n", taskName.c_str());
        try {
            t->run();
        }
        catch (exception &e) {
            log_error("Exception happens when executing:\n");
            log_error("Thread pool Name:%s Task name:%s\n", thisobj->_poolName.c_str(), taskName.c_str());
            log_error("Raison:%s", e.what());
        }
        catch (...) {
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

	LockGuard _lockGuard(_lock);
    _NotEmpty.wait([&]{return !_taskList.empty() || _stop;});

    if (!_taskList.empty())
    {
        Task* curTask = _taskList.front();
        _taskList.pop_front();
        _NotFull.notify();
        return curTask;
    }
    if (_stop)
    {
        return nullptr;
    }

	return &_dummyTask;
};

void ThreadPool::put(Task* t)
{
    LockGuard _lockGuard(_lock);
    log_debug("ThreadPool:::::taskList:%d poolSize:%d stop:%d\n", _taskList.size(), _poolSize, _stop);
    _NotFull.wait([&]{return _taskList.size() < _poolSize || _stop;});
    if (!_stop)
    {
        _taskList.push_back(t);
        _NotEmpty.notify();
        return;
    }

    //The thread pool is stopped, we need to run it locally
    log_debug("Running locally since the threadpool is stopped\n");
    t->run();
};

void ThreadPool::start() {
    log_warn("ThreadPool::start() start\n");
    if (!_stop) {
        log_warn("Thread pool named '%s' is started multiple times\n", _poolName.c_str());
        return;
    }

    _stop = false;
    for (size_t i = 0; i < _poolSize; i++) {
        Thread *currentThread = new Thread(_poolName + "-poolthread-" + NacosStringOps::valueOf(i), runInThread, this);
        _threads.push_back(currentThread);
        currentThread->start();
    }
};

void ThreadPool::stop() {
    if (_stop) {
        return;
    }

    _stop = true;
    _NotEmpty.notifyAll();
    _NotFull.notifyAll();

    for (auto&& thread : _threads)
    {
        thread->join();
        delete thread;
    }

    _threads.clear();
};
}//namespace nacos
