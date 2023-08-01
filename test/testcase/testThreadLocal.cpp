#include <iostream>
#include "src/utils/UuidUtils.h"
#include "src/thread/Thread.h"
#include "src/thread/ThreadLocal.h"
#include "src/debug/DebugAssertion.h"
#include "src/utils/RandomUtils.h"

using namespace std;
using namespace nacos;

ThreadLocal<NacosString> threadLocal;
ThreadLocal<int*> threadLocalPtr(nullptr);

class ThreadLocalWithInit : public ThreadLocal<int*> {
public:
    void onCreate(int **value) override
    {
        *value = reinterpret_cast<int*>(0xffff);
        log_debug("ThreadLocalWithInit::onCreate is called, ptr value: %p\n", *value);
    }

    void onDestroy(int **value) override
    {
        log_debug("ThreadLocalWithInit::onDestroy is called, ptr value: %p\n", *value);
    }
};

ThreadLocalWithInit threadLocalPtrWithInitializer;

void *ThreadLocalFuncs4Ptr(void *param) {
    log_debug("threadLocalPtr.get() : %p, should be null\n", threadLocalPtr.get());
    NACOS_ASSERT(threadLocalPtr.get() == NULL);

    for (int i = 0; i < 100; i++) {
        int* rndPtr = new int;
        *rndPtr = RandomUtils::random(0, 1000);
        threadLocalPtr.set(rndPtr);

        NACOS_ASSERT(rndPtr == threadLocalPtr.get())
    }

    return nullptr;
}

void *ThreadLocalFuncs4PtrWithInitializer(void *param) {
    log_debug("threadLocalPtr.get() : %p, should be 0xFFFF\n", threadLocalPtrWithInitializer.get());
    NACOS_ASSERT(threadLocalPtrWithInitializer.get() == reinterpret_cast<int*>(0xFFFF));

    for (int i = 0; i < 100; i++) {
        int* rndPtr = new int;
        *rndPtr = RandomUtils::random(0, 1000);
        threadLocalPtrWithInitializer.set(rndPtr);

        NACOS_ASSERT(rndPtr == threadLocalPtrWithInitializer.get())
    }

    return nullptr;
}

void *ThreadLocalFuncs(void *param) {
    Thread *thisThread = *static_cast<Thread**>(param);

    for (int i = 0; i < 100; i++) {
        threadLocal.set(UuidUtils::generateUuid().c_str());
        log_debug("Thread %s UUID: %s\n", thisThread->getThreadName().c_str(), threadLocal.get().c_str());
    }

    return nullptr;
}

bool testThreadLocal() {
    cout << "in function testThreadLocal" << endl;

    cout << "Generating threads..." << endl;

    Thread *threads[10] = {nullptr};
    for (int i = 0; i < 10; i++) {
        NacosString threadName = "Thread-" + NacosStringOps::valueOf(i);
        threads[i] = new Thread(threadName, ThreadLocalFuncs, static_cast<void*>(&threads[i]));
        threads[i]->start();
    }

    for (int i = 0; i < 10; i++) {
        threads[i]->join();
        delete threads[i];
    }

    cout << "test end..." << endl;

    return true;
}

bool testThreadLocalPtr() {
    cout << "in function testThreadLocalPtr" << endl;

    cout << "Generating threads..." << endl;

    Thread *threads[10] = {nullptr};
    for (int i = 0; i < 10; i++) {
        NacosString threadName = "ThreadPtr-" + NacosStringOps::valueOf(i);
        threads[i] = new Thread(threadName, ThreadLocalFuncs4Ptr, static_cast<void*>(&threads[i]));
        threads[i]->start();
    }

    for (auto&& thread : threads)
    {
        thread->join();
        delete thread;
    }

    cout << "test end..." << endl;

    return true;
}

bool testThreadLocalPtrWithInitializer() {
    cout << "in function testThreadLocalPtrWithInitializer" << endl;

    cout << "Generating threads..." << endl;

    Thread *threads[10] = {nullptr};
    for (int i = 0; i < 10; i++) {
        NacosString threadName = "ThreadPtr-" + NacosStringOps::valueOf(i);
        threads[i] = new Thread(threadName, ThreadLocalFuncs4PtrWithInitializer, static_cast<void*>(&threads[i]));
        threads[i]->start();
    }

    for (int i = 0; i < 10; i++) {
        threads[i]->join();
        delete threads[i];
    }

    cout << "test end..." << endl;

    return true;
}