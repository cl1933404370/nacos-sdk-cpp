#include <algorithm>
#include "src/thread/DelayedThreadPool.h"

#include <thread>

#include "NacosExceptions.h"

namespace nacos {

/**
* A thread pool that can run tasks with specified delay
*/
class DelayedWorker : public Task {
private:
    DelayedThreadPool &_container;
public:
    volatile bool _start;
    DelayedWorker(DelayedThreadPool &container) : _container(container) {
        _start = false;
    };

    void run() {
        log_debug("DelayedWorker::run()\n");
        while (!_container._stop_delayed_tp) {
            std::unique_lock lock(_container._lockForScheduleTasks);

            //no data, wait until data is available
            log_debug("[DelayedWorker] start to wait for task stop=%d\n", _container._stop);
            if (_container._scheduledTasks.empty()) {
                log_debug("[DelayedWorker] empty, wait for task\n");
                if (_container._stop_delayed_tp) {
                    lock.unlock();
                    return;
                }
                std::this_thread::yield();
                _container._delayTaskNotEmpty.wait(lock);
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
            std::vector< std::pair<long, Task*> >::iterator it;
            while ((it = _container._scheduledTasks.begin()) != _container._scheduledTasks.end()) {
                int64_t now_time = TimeUtils::getCurrentTimeInMs();
                log_debug("[DelayedWorker] now = %ld wakeup time = %ld\n", now_time, it->first);
                if (it->first <= now_time) {
                    Task *task = it->second;
                    _container._scheduledTasks.erase(it);
                    lock.unlock();
                    //the task can also attempt to retrieve the lock
                    if (_container._stop_delayed_tp) {
                        return;
                    }
                    task->run();
                    lock.lock();
                    log_debug("[DelayedWorker] continue 2 next task\n");
                } else {
                    //awake from sleep when a stop signal is sent
                    if (_container._stop_delayed_tp) {
                        lock.unlock();
                        return;
                    }
                    _container._delayTaskNotEmpty.wait_for(lock,std::chrono::milliseconds(static_cast<long>(it->first - now_time)));
                }
            }

            lock.unlock();
        }
        _start = false;
    }
};

DelayedThreadPool::DelayedThreadPool(const NacosString &poolName, size_t poolSize)
:ThreadPool(poolName, poolSize),/*_delayTaskNotEmpty(_lockForScheduleTasks),*/ _stop_delayed_tp(true) {
    log_debug("DelayedThreadPool::DelayedThreadPool() name = %s size = %d\n", poolName.c_str(), poolSize);
    if (poolSize <= 0) {
        throw NacosException(NacosException::INVALID_PARAM, "Poll size cannot be lesser than 0");
    }
    delayTasks = new DelayedWorker*[poolSize];
    log_debug("DelayedThreadPool::DelayedThreadPool initializing tasks\n");
    for (size_t i = 0; i < poolSize; i++) {
        delayTasks[i] = new DelayedWorker(*this);
    }
}


DelayedThreadPool::~DelayedThreadPool() {
    log_debug("DelayedThreadPool::~DelayedThreadPool\n");
    if (delayTasks != NULL) {
        for (size_t i = 0; i < _poolSize; i++) {
            delete delayTasks[i];
            delayTasks[i] = NULL;
        }
        delete [] delayTasks;
        delayTasks = NULL;
    }
}

struct tagAscOrdFunctor{
    bool operator ()(const std::pair<long, Task*> &lhs, const std::pair<long, Task*> &rhs) {
        return lhs.first < rhs.first;
    };
} ascOrdFunctor = {};

#include <stdio.h>
//futureTimeToRun: time in MS
void DelayedThreadPool::schedule(Task *t, long futureTimeToRun) {
    if (_stop) {
        return;
    }
    if (futureTimeToRun < 0) {
        throw NacosException(NacosException::INVALID_PARAM, "futureTimeToRun must not be negative");
    }
    log_debug("DelayedThreadPool::schedule() name=%s future = %ld\n", t->getTaskName().c_str(), futureTimeToRun);
    std::pair<long, Task*> scheduledTask = std::make_pair (futureTimeToRun, t);
    {
        {
            std::unique_lock lock(_lockForScheduleTasks);
            _scheduledTasks.push_back(scheduledTask);
            std::sort(_scheduledTasks.begin(), _scheduledTasks.end(), ascOrdFunctor);
        }
        _delayTaskNotEmpty.notify_all();
    }
}

void DelayedThreadPool::start() {
    _stop_delayed_tp = false;
    ThreadPool::start();
    log_debug("DelayedThreadPool::start()\n");
    for (size_t i = 0; i < _poolSize; i++) {
        delayTasks[i]->_start = true;
        put(delayTasks[i]);
    }
}

void DelayedThreadPool::stop() {
    if (_stop_delayed_tp) {
        return;
    }

    _stop_delayed_tp = true;
    _delayTaskNotEmpty.notify_all();
    for (auto&& _thread : _threads)
    {
        _thread->kill();
    }

    ThreadPool::stop();
}

}