#ifndef __THREAD_H_
#define __THREAD_H_

#include <exception>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include "NacosString.h"
#include "src/log/Logger.h"
#include "src/thread/Tid.h"

#if defined(_MSC_VER) || defined(__WIN32__) || defined(WIN32)
//#define THREAD_STOP_SIGNAL SIGUSR1
// #define SIGINT          2   // interrupt
// #define SIGILL          4   // illegal instruction - invalid function image
// #define SIGFPE          8   // floating point exception
// #define SIGSEGV         11  // segment violation
// #define SIGTERM         15  // Software termination signal from kill
// #define SIGBREAK        21  // Ctrl-Break sequence
// #define SIGABRT         22  // abnormal termination triggered by abort call
///#define THREAD_STOP_SIGNAL SIGUSR1
const DWORD THREAD_STOP_SIGNAL = 0x10;
#include <processthreadsapi.h>

#else
    #define THREAD_STOP_SIGNAL SIGUSR1
#endif

namespace nacos{
typedef void *(*ThreadFn)(void *);

/*
* Thread.h
* Author: Liu, Hanyu
* This is NOT like the thread class in Java!
* It's just a simple encapsulation of pthread_create() and pthread_join
* It doesn't have a virtual run() function,
 * a function pointer(ThreadFn) should be passed to the constructor so it will be used as the function pointer parameter for pthread_create
*/
class Thread {
private:
    NacosString _threadName;

#if defined(_MSC_VER) || defined(__WIN32__) || defined(WIN32)
    std::thread _thread;
#else
    pthread_t _thread;
#endif

    ThreadFn _function;
    //TODO:thread id
    TID_T _tid;
    std::atomic_bool _start;
    void *_threadData;

    Thread(): _threadName(""), _function(NULL), _threadData(NULL), _start(false), _tid(0) 
#if defined(_MSC_VER) || defined(__WIN32__) || defined(WIN32)
#else   
    ,_thread(0)
#endif
    {};

    static void empty_signal_handler(int signum) {};

#if defined(_MSC_VER) || defined(__WIN32__) || defined(WIN32)
#else
    static struct sigaction old_action;
#endif
public:
    static void Init();
    static void DeInit();
     
    void setThreadName(const NacosString &threadName) { _threadName = threadName; };

    NacosString getThreadName() { return _threadName; };

    static void *threadFunc(void *param);

    Thread(const NacosString &threadName, ThreadFn fn)
            : _threadName(threadName), _function(fn), _threadData(NULL) {
        _start = false;
    };

    Thread(const NacosString &threadName, ThreadFn fn, void *threadData)
            : _threadName(threadName), _function(fn), _threadData(threadData) {
        _start = false;
    };
    Thread(Thread&) = delete;
    Thread& operator=(Thread&) = delete;
    ~Thread() {
        _start = false;
    }

    void start();

    void join();

    void kill();

    //int pthread_kill(pthread_t thread, int sig);
};
}//namespace nacos

#endif
