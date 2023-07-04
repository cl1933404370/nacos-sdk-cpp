#include "Thread.h"

// void handle_signal(int signal) {
//     switch (signal) {
// #ifdef _WIN32
//     case SIGTERM:
//     case SIGABRT:
//     case SIGBREAK:
// #else
//     case SIGHUP:
// #endif
//       got_sighup = true;
//       break;
//     case SIGINT:
//       got_sigint = true;
//       break;
//     }
//   }

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

void *nacos::Thread::threadFunc(void *param)
{
    Thread *currentThread = (Thread *)param;
    currentThread->_tid = gettidv1();

    try
    {
        return currentThread->_function(currentThread->_threadData);
    }
    catch (std::exception &e)
    {
        currentThread->_function = NULL;
        nacos::log_error("Exception happens when executing:\n");
        nacos::log_error("Thread Name:%s Thread Id:%d\n", currentThread->_threadName.c_str(), currentThread->_tid);
        nacos::log_error("Raison:%s", e.what());
        abort();
    }
    catch (...)
    {
        currentThread->_function = NULL;
        nacos::log_error("Unknown exception happens when executing:\n");
        nacos::log_error("Thread Name:%s Thread Id:%d\n", currentThread->_threadName.c_str(), currentThread->_tid);
        throw;
    }
}

void nacos::Thread::start()
{
    _start = true;
    pthread_create(&_thread, NULL, threadFunc, (void *)this);
}

void nacos::Thread::join()
{
    log_debug("Calling Thread::join() on %s\n", _threadName.c_str());
    if (!_start)
    {
        log_debug("Thread::join() called on stopped thread for %s\n", _threadName.c_str());
        return;
    }

    pthread_join(_thread, NULL);
}

void nacos::Thread::kill()
{
#if defined(_MSC_VER) || defined(__WIN32__) || defined(WIN32)
   // pthread_kill windows
   QueueUserAPC((PAPCFUNC)empty_signal_handler, pthread_getw32threadhandle_np(_thread), THREAD_STOP_SIGNAL);
#else
    pthread_kill(_thread, THREAD_STOP_SIGNAL);
#endif


    // https://github.com/GerHobbelt/pthread-win32/blob/master/pthread_kill.c
    // int result = 0;
    //   ptw32_thread_t * tp;
    //   ptw32_mcs_local_node_t node;

    //   ptw32_mcs_lock_acquire(&ptw32_thread_reuse_lock, &node);

    //   tp = (ptw32_thread_t *) thread.p;

    //   if (NULL == tp
    //       || thread.x != tp->ptHandle.x
    //       || NULL == tp->threadH)
    //     {
    //       result = ESRCH;
    //     }

    //   ptw32_mcs_lock_release(&node);

    //   if (0 == result && 0 != sig)
    //     {
    //       /*
    //        * Currently only supports direct thread termination via SIGABRT.
    //        */
    //       switch (sig)
    //       {
    //       default:
    //           result = EINVAL;
    //           break;
    // #ifdef SIGINT
    //       case SIGINT:
    // #endif
    // #ifdef SIGTERM
    //       case SIGTERM:
    // #endif
    // #ifdef SIGBREAK
    //       case SIGBREAK:
    // #endif
    // #ifdef SIGABRT
    //       case SIGABRT:
    // #endif
    // #ifdef SIGABRT_COMPAT
    //       case SIGABRT_COMPAT:
    // #endif
    //       {
    //           ptw32_mcs_local_node_t stateLock;

    //           /*
    //            * Lock for async-cancel safety.
    //            */
    //           ptw32_mcs_lock_acquire(&tp->stateLock, &stateLock);

    //           // Only terminate the thread when it is still running:
    //           if (tp->state < PThreadStateLast)
    //           {
    //               tp->state = PThreadStateLast;
    //               tp->cancelState = PTHREAD_CANCEL_DISABLE;
    //               ptw32_mcs_lock_release(&stateLock);

    //               result = TerminateThread(tp->threadH, (DWORD)(ptrdiff_t)PTHREAD_CANCELED);
    //               result = (result != 0) ? 0 : EINVAL;

    //               // Set exit to CANCELED (KILLED) when it hasn't been set already.
    //               if (tp->exitStatus == NULL)
    //                   tp->exitStatus = PTHREAD_CANCELED;
    //           }
    //           else
    //           {
    //               ptw32_mcs_lock_release(&stateLock);

    //               // TODO: ? flag the call as not-necessary-any-more because thread has already terminated ?
    //               result = 0;
    //           }
    //       }
    //           break;
    //       }
    //     }

    //   return result;
}