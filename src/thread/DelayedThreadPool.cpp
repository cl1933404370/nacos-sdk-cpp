#include <algorithm>
#include "src/thread/DelayedThreadPool.h"
#include "NacosExceptions.h"

namespace nacos {

/**
* A thread pool that can run tasks with specified delay
*/
class DelayedWorker : public Task {
private:
    DelayedThreadPool* _container;
public:
    std::atomic_bool _start;

    DelayedWorker(DelayedThreadPool* container) : _container(container) {
        _start = false;
    }

    void run() override
    {
        log_debug("DelayedWorker::run()\n");
        while (!_container->_stop_delayed_tp) {
            _container->_lockForScheduleTasks.lock();

            //no data, wait until data is available
            log_debug("[DelayedWorker] start to wait for task stop=%d\n", _container->_stop.load());
            if (_container->_scheduledTasks.empty()) {
                log_debug("[DelayedWorker] empty, wait for task\n");
                if (_container->_stop_delayed_tp) {
                    _container->_lockForScheduleTasks.unlock();
                    return;
                }
                _container->_delayTaskNotEmpty.wait();
                //_container->_delayTaskNotEmpty.wait([&]{return !_container->_scheduledTasks.empty() || _container->_stop_delayed_tp;});
                log_debug("[DelayedWorker] wake up due to incoming event\n");
            }

            /*
            * Here actually we have 2 different design patterns:
            * 1. wait both on the _delayTaskNotEmpty condition and the nearest task to be executed
            * 2. wait on the nearest task to be executed
            * 
            * The advantages/disadvantages for pattern 1 (the pattern applied here).
            * The advantages are that:
            * 1. This design pattern is more responsive
            * 2. For tasks that take a short time, this pattern is more effective since one worker can handle more tasks
            * (compared with the pattern that wait() for exact one task in else {} block)
            * 
            * The disadvantages are that:
            * 1. Will incur more wake-ups and context-switches(I guess)
            * 2. Not as precise as the pattern 2
            * */

            log_debug("[DelayedWorker] iterating on _scheduledTasks\n");
            std::vector< std::pair<uint64_t, Task*> >::iterator it;
            while ((it = _container->_scheduledTasks.begin()) != _container->_scheduledTasks.end()) {
	            const uint64_t now_time = TimeUtils::getCurrentTimeInMs();
                log_debug("[DelayedWorker] now = %ld wakeup time = %ld\n", now_time, it->first);
                if (it->first <= now_time) {
                    Task *task = it->second;
                    _container->_scheduledTasks.erase(it);
                    _container->_lockForScheduleTasks.unlock();

                    //the task can also attempt to retrieve the lock
                    if (_container->_stop_delayed_tp) {
                        return;
                    }
                    task->run();

                   _container->_lockForScheduleTasks.lock(); 
                    log_debug("[DelayedWorker] continue 2 next task\n");
                } else {
                    //awake from sleep when a stop signal is sent
                    if (_container->_stop_delayed_tp) {
                        _container->_lockForScheduleTasks.unlock();
                        return; 
                    }
                    //_container->_delayTaskNotEmpty.wait(it->first - now_time);
                    _container->_delayTaskNotEmpty.wait();
                }
            }

            _container->_lockForScheduleTasks.unlock();
        }
        _start = false;
    }

    ~DelayedWorker() override
    {
        log_debug("DelayedWorker::~DelayedWorker()\n");
    }
};

    
//int ** p;
//p = new int*[m];  
//for (int i = 0; i < m; i++)
//	p[i] = new int[n];  

DelayedThreadPool::DelayedThreadPool(const NacosString &poolName, size_t poolSize)
:ThreadPool(poolName, poolSize),_delayTaskNotEmpty(_lockForScheduleTasks), _stop_delayed_tp(true) {
    log_debug("DelayedThreadPool::DelayedThreadPool() name = %s size = %d\n", poolName.c_str(), poolSize);
    if (poolSize <= 0) {
        throw NacosException(NacosException::INVALID_PARAM, "Poll size cannot be lesser than 0");
    }
    _delayTasks = new DelayedWorker*[poolSize];
    log_debug("DelayedThreadPool::DelayedThreadPool initializing tasks\n");
    for (size_t i = 0; i < poolSize; ++i) {
        _delayTasks[i]  = new DelayedWorker(this);
    }
}


DelayedThreadPool::~DelayedThreadPool() {
    log_debug("DelayedThreadPool::~DelayedThreadPool\n");
    if (_delayTasks) {
        for (size_t i = 0; i < _poolSize; i++) {
            delete []_delayTasks[i];
            _delayTasks[i] = nullptr;
        }
        delete[] _delayTasks;
        _delayTasks = nullptr;
    }
}

struct tagAscOrdFunctor{
    bool operator ()(const std::pair<long, Task*> &lhs, const std::pair<long, Task*> &rhs) const
    {
        return lhs.first < rhs.first;
    }
} ascOrdFunctor = {};

//futureTimeToRun: time in MS
void DelayedThreadPool::schedule(Task *t, uint64_t futureTimeToRun) {
    if (_stop) {
        return;
    }
    log_debug("DelayedThreadPool::schedule() name=%s future = %ld\n", t->getTaskName().c_str(), futureTimeToRun);
    const std::pair<uint64_t, Task*> scheduledTask = std::make_pair(futureTimeToRun, t);
    LockGuard lockSchedTasks(_lockForScheduleTasks);
    _scheduledTasks.push_back(scheduledTask);
    std::ranges::sort(_scheduledTasks, ascOrdFunctor);
    _delayTaskNotEmpty.notify();
}

void DelayedThreadPool::start() {
    _stop_delayed_tp = false;
    ThreadPool::start();
    log_debug("DelayedThreadPool::start()\n");
    for (size_t i = 0; i < _poolSize; i++) {
        _delayTasks[i]->_start = true;
        put(_delayTasks[i]);
    }
}

void DelayedThreadPool::stop() {
    if (_stop_delayed_tp) {
        return;
    }

    _stop_delayed_tp = true;
    _delayTaskNotEmpty.notifyAll();
    for (auto&& _thread : _threads)
    {
        _thread->kill();
    }

    ThreadPool::stop();
}

}