#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#if defined(_MSC_VER) || defined(__WIN32__) || defined(WIN32)
#include <io.h>
#include <process.h>
#include <thread>
#include <chrono>
#else
#include <unistd.h>
#endif
#include "factory/NacosFactoryFactory.h"
#include "ResourceGuard.h"
#include "listen/Listener.h"
#include "constant/PropertyKeyConst.h"
#include "src/debug/DebugAssertion.h"
#include "src/log/Logger.h"

using namespace std;
using namespace nacos;

class MyListener : public Listener {
private:
    int num;
public:
    MyListener(int num) {
        this->num = num;
    }

    void receiveConfigInfo(const NacosString &configInfo) {
        cout << "===================================" << endl;
        cout << "Watcher" << num << endl;
        cout << "Watched Key UPDATED:" << configInfo << endl;
        cout << "===================================" << endl;
    }
};

bool testRemoveKeyBeingWatched() {
    cout << "in function testRemoveKeyBeingWatched" << endl;
    Properties props;
    props[PropertyKeyConst::SERVER_ADDR] = "127.0.0.1:8848";
    ADD_AUTH_INFO(props);
    ADD_SPAS_INFO(props);
    INacosServiceFactory *factory = NacosFactoryFactory::getNacosFactory(props);
    ResourceGuard <INacosServiceFactory> _guardFactory(factory);
    ConfigService *n = factory->CreateConfigService();
    ResourceGuard <ConfigService> _serviceFactory(n);
    n->publishConfig("RemovedWhileWatching", NULLSTR, "dummyContent");

    MyListener *theListener = new MyListener(1);
    n->addListener("RemovedWhileWatching", NULLSTR, theListener);

    #if defined(_MSC_VER) || defined(__WIN32__) || defined(WIN32)
    std::this_thread::sleep_for(std::chrono::seconds(2));
    #else
    sleep(2);
    #endif
    cout << "remove key" << endl;
    n->removeConfig("RemovedWhileWatching", NULLSTR);
    #if defined(_MSC_VER) || defined(__WIN32__) || defined(WIN32)
    std::this_thread::sleep_for(std::chrono::seconds(2));
    #else
    sleep(2);
    #endif
    cout << "set key" << endl;
    n->publishConfig("RemovedWhileWatching", NULLSTR, "dummyContent1");
    #if defined(_MSC_VER) || defined(__WIN32__) || defined(WIN32)
    std::this_thread::sleep_for(std::chrono::seconds(2));
    #else
    sleep(2);
    #endif
    cout << "remove key" << endl;
    n->removeConfig("RemovedWhileWatching", NULLSTR);
    cout << "Hold for 30 secs" << endl;
    #if defined(_MSC_VER) || defined(__WIN32__) || defined(WIN32)
    std::this_thread::sleep_for(std::chrono::seconds(30));
    #else
    sleep(30);
    #endif
    n->removeListener("RemovedWhileWatching", NULLSTR, theListener);
    cout << "remove listener2" << endl;
    cout << "test successful" << endl;

    return true;
}