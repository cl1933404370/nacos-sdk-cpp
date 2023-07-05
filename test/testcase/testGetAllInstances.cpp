#include <iostream>
#include <stdlib.h>
#if defined(_MSC_VER) || defined(__WIN32__) || defined(WIN32)
#include <io.h>
#include <process.h>
#else
#include <unistd.h>
#endif
#include <list>
#include "src/naming/NamingProxy.h"
#include "factory/NacosFactoryFactory.h"
#include "naming/Instance.h"
#include "constant/ConfigConstant.h"
#include "constant/UtilAndComs.h"
#include "src/http/HTTPCli.h"
#include "src/debug/DebugAssertion.h"
#include "src/log/Logger.h"
#include "NacosString.h"
#include "Properties.h"
#include "constant/PropertyKeyConst.h"
#include "ResourceGuard.h"

using namespace std;
using namespace nacos;

bool testGetAllInstances() {
    cout << "in function testGetAllInstances" << endl;
    Properties configProps;
    ADD_AUTH_INFO(configProps);
    ADD_SPAS_INFO(configProps);
    configProps[PropertyKeyConst::SERVER_ADDR] = "127.0.0.1";
    configProps[PropertyKeyConst::NAMESPACE] = "238e832b-d103-44c6-b618-d74da8c38b06";
    INacosServiceFactory *factory = NacosFactoryFactory::getNacosFactory(configProps);
    ResourceGuard <INacosServiceFactory> _guardFactory(factory);
    NamingService *namingSvc = factory->CreateNamingService();
    ResourceGuard <NamingService> _guardService(namingSvc);

    Instance instance;
    instance.clusterName = "DefaultCluster";
    instance.ip = "127.0.0.1";
    instance.port = 2333;
    instance.instanceId = "1";
    instance.ephemeral = true;

    try {
        for (int i = 0; i < 10; i++) {
            NacosString serviceName = "TestNamingService" + NacosStringOps::valueOf(i);
            instance.port = 2000 + i;
            namingSvc->registerInstance(serviceName, instance);
        }
    }
    catch (NacosException &e) {
        cout << "encounter exception while registering service instance, raison:" << e.what() << endl;
        return false;
    }
    cout << "wait for 5 secs" << endl;

    #if defined(_MSC_VER) || defined(__WIN32__) || defined(WIN32)
    Sleep(5000);
    #else
    sleep(5);
    #endif

    list <Instance> instances = namingSvc->getAllInstances("TestNamingService1");
    cout << "getAllInstances from server:" << endl;
    for (list<Instance>::iterator it = instances.begin();
         it != instances.end(); it++) {
        cout << "Instance:" << it->toString() << endl;
    }

    if (instances.size() != 1) {
        cout << "There should be only 1 instance for TestNamingService1" << endl;
        return false;
    }

    if (instances.front().port != 2001) {
        cout << "TestNamingService1's port should be 2001" << endl;
        return false;
    }

    if (instances.front().ip != "127.0.0.1") {
        cout << "TestNamingService1's ip should be 127.0.0.1" << endl;
        return false;
    }

    #if defined(_MSC_VER) || defined(__WIN32__) || defined(WIN32)
    Sleep(1000);
    #else
    sleep(1);
    #endif


    try {
        for (int i = 0; i < 3; i++) {
            NacosString serviceName = "TestNamingService" + NacosStringOps::valueOf(i);

            namingSvc->deregisterInstance(serviceName, "127.0.0.1", 2000 + i);

            #if defined(_MSC_VER) || defined(__WIN32__) || defined(WIN32)
            Sleep(1000);
            #else
            sleep(1);
            #endif
            
        }
    }
    catch (NacosException &e) {
        cout << "encounter exception while registering service instance, raison:" << e.what() << endl;
        return false;
    }

    return true;
}