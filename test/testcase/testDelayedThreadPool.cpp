#include <iostream>

#ifdef _WIN32
#include <thread>
#include <chrono>
#else
#include <unistd.h>
#endif 

#include <cstdio>
#include "src/thread/DelayedThreadPool.h"
#include "src/debug/DebugAssertion.h"
#include "NacosExceptions.h"

using namespace std;
using namespace nacos;

class DelayedTask : public Task
{
public:
    DelayedThreadPool *executor;
    uint64_t interval; // in MS
    uint64_t last_exec_time;
    DelayedTask(): executor(nullptr), interval(0)
    {
        last_exec_time = 0;
    }

    void run() override
    {
        const uint64_t now_ms = TimeUtils::getCurrentTimeInMs();
        uint64_t intervalCalc = 0;
        if (last_exec_time != 0)
        {
            intervalCalc = now_ms - last_exec_time;
        }
        last_exec_time = now_ms;
        executor->schedule(this, now_ms + interval); // interval/1000 secs later
        if (executor == nullptr)
        {
            throw NacosException(NacosException::INVALID_CONFIG_PARAM, "no executor");
        }
        printf(">>>>>>>>>>>>>>>>>>Task %s triggered, time =%llu (%llu), interval = %llu\n", getTaskName().c_str(), now_ms / 1000, now_ms, intervalCalc);

#ifdef _WIN32
        std::this_thread::sleep_for(std::chrono::seconds(1));
#else
        sleep(1);
#endif
    }

    ~DelayedTask() override
    {
        //todo : why does this cause a crash?
        delete executor;
        printf(">>>>>>>>>>>>>>>>>>Task %s destroyed\n", getTaskName().c_str());
    }
};

bool testDelayedThread()
{
    cout << "in function testDelayedThread" << endl;

    DelayedThreadPool dtp("testDPool", 11);
    dtp.start();
    cout << "create tasks" << endl;

    DelayedTask delayedTasks[10];

    const uint64_t nowMs = TimeUtils::getCurrentTimeInMs();
    for (size_t i = 0; i < sizeof(delayedTasks) / sizeof(DelayedTask); i++)
    {
        delayedTasks[i].executor = &dtp;
        delayedTasks[i].interval = (i + 1) * 1000;
        delayedTasks[i].setTaskName("DelayedTask-" + NacosStringOps::valueOf(i));

        dtp.schedule(&delayedTasks[i], nowMs);
    }

#ifdef _WIN32
    std::this_thread::sleep_for(std::chrono::seconds(20));
#else
    sleep(20);
#endif

    cout << "call stop()" << endl;
    dtp.stop();
    cout << "end of test" << endl;
    delete []delayedTasks;
    return true;
}

bool testDelayedThread2()
{
    cout << "in function testDelayedThread2 - multiple tasks triggered at the same time" << endl;

    DelayedThreadPool dtp("testDPool", 11);
    dtp.start();
    cout << "create tasks" << endl;

    DelayedTask delayedTasks[10];

    const uint64_t nowMs = TimeUtils::getCurrentTimeInMs();
    for (size_t i = 0; i < sizeof(delayedTasks) / sizeof(DelayedTask); i++)
    {
        delayedTasks[i].executor = &dtp;
        delayedTasks[i].interval = 1000;
        delayedTasks[i].setTaskName("DelayedTask-" + NacosStringOps::valueOf(i));

        dtp.schedule(&delayedTasks[i], nowMs);
    }

#ifdef _WIN32
    std::this_thread::sleep_for(std::chrono::milliseconds(30));
#else
    sleep(20);
#endif

    cout << "call stop()" << endl;
    dtp.stop();
    cout << "end of test" << endl;

    return true;
}
