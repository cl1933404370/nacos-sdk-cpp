#include "Thread.h"

#if defined(_MSC_VER) || defined(__WIN32__) || defined(WIN32)

#else
struct sigaction Thread::old_action
#endif

    void nacos::Thread::Init()
{
#if defined(_MSC_VER) || defined(__WIN32__) || defined(WIN32)
    signal(SIGINT, empty_signal_handler);
    signal(SIGILL, empty_signal_handler);
    signal(SIGFPE, empty_signal_handler);
    signal(SIGSEGV, empty_signal_handler);
    signal(SIGTERM, empty_signal_handler);
    signal(SIGBREAK, empty_signal_handler);
    signal(SIGABRT, empty_signal_handler);
#else
    struct sigaction action;
    action.sa_flags = 0;
    action.sa_handler = empty_signal_handler;
    sigemptyset(&action.sa_mask);
    sigaction(THREAD_STOP_SIGNAL, &action, &Thread::old_action);
#endif
};


void nacos::Thread::DeInit()
{
#if defined(_MSC_VER) || defined(__WIN32__) || defined(WIN32)
    signal(SIGINT, SIG_DFL);
    signal(SIGILL, SIG_DFL);
    signal(SIGFPE, SIG_DFL);
    signal(SIGSEGV, SIG_DFL);
    signal(SIGTERM, SIG_DFL);
    signal(SIGBREAK, SIG_DFL);
    signal(SIGABRT, SIG_DFL);
#else
    sigaction(THREAD_STOP_SIGNAL, &Thread::old_action, NULL);
#endif
};

void* nacos::Thread::threadFunc(void* param)
{
    const auto currentThread = static_cast<Thread*>(param);
    currentThread->_tid = gettidv1();

    try
    {
        return currentThread->_function(currentThread->_threadData);
    }
    catch (std::exception& e)
    {
        currentThread->_function = nullptr;
        nacos::log_error("Exception happens when executing:\n");
        nacos::log_error("Thread Name:%s Thread Id:%d\n", currentThread->_threadName.c_str(), currentThread->_tid);
        nacos::log_error("Raison:%s", e.what());
        abort();
    }
    catch (...)
    {
        currentThread->_function = nullptr;
        nacos::log_error("Unknown exception happens when executing:\n");
        nacos::log_error("Thread Name:%s Thread Id:%d\n", currentThread->_threadName.c_str(), currentThread->_tid);
        throw;
    }
}

void nacos::Thread::start()
{
    _start = true;
    pthread_create(&_thread, nullptr, threadFunc, (void*)this);
}

void nacos::Thread::join()
{
    log_debug("Calling Thread::join() on %s\n", _threadName.c_str());
    if (!_start)
    {
        log_debug("Thread::join() called on stopped thread for %s\n", _threadName.c_str());
        return;
    }

    pthread_join(_thread, nullptr);
}

void nacos::Thread::kill()
{
#if defined(_MSC_VER) || defined(__WIN32__) || defined(WIN32)
    DWORD exciteCode;
    bool aa = GetExitCodeThread(_thread->handle, &exciteCode);
    DWORD error = GetLastError();
    QueueUserAPC([](ULONG_PTR a)
        {
        }, _thread->handle, THREAD_STOP_SIGNAL);
    QueueUserAPC([](ULONG_PTR a)
        {
        }, _thread->handle, SIGTERM);
    QueueUserAPC([](ULONG_PTR a)
        {
        }, _thread->handle, exciteCode);
    QueueUserAPC([](ULONG_PTR a)
        {
        }, _thread->handle, error);
    if (exciteCode == STILL_ACTIVE)
    {
        //TerminateThread(_thread->handle, THREAD_STOP_SIGNAL);
        TerminateThread(_thread->handle, exciteCode);
    }
    //ExitThread(exciteCode);
#else
    pthread_kill(_thread, THREAD_STOP_SIGNAL);
#endif
    }
