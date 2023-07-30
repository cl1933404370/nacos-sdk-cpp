#ifndef __DELAYED_THREAD_POOL_H_
#define __DELAYED_THREAD_POOL_H_

#include <vector>
#include <utility>
#include "src/thread/ThreadPool.h"
#include "src/thread/Task.h"
#include "src/thread/Mutex.h"

namespace nacos {

class DelayedWorker;

class DelayedThreadPool : public ThreadPool {
private:
    Condition _delayTaskNotEmpty;
    Mutex _lockForScheduleTasks;//for _scheduledTasks
    std::vector< std::pair<uint64_t, Task*> > _scheduledTasks;
    DelayedThreadPool();
    DelayedWorker **delayTasks;
    std::atomic_bool _stop_delayed_tp;
public:
    DelayedThreadPool(const NacosString &poolName, size_t poolSize) ;
    ~DelayedThreadPool();

    /**
    * schedule the execution for a task
    * @param t the task to run
    * @param futureTimeToRun the time (in ms) for the task to run
    */
    void schedule(Task *t, uint64_t futureTimeToRun);

    friend class DelayedWorker;

    virtual void start();

    virtual void stop();
};

}

#endif