#include <iostream>
#include <stdlib.h>
#if defined(_MSC_VER) || defined(__WIN32__) || defined(WIN32)
#include <io.h>
#include <process.h>
#else
#include <unistd.h>
#endif
#include "src/naming/NamingProxy.h"
#include "src/naming/NacosNamingService.h"
#include "factory/NacosFactoryFactory.h"
#include "ResourceGuard.h"
#include "naming/Instance.h"
#include "constant/ConfigConstant.h"
#include "constant/UtilAndComs.h"
#include "src/http/HTTPCli.h"
#include "src/debug/DebugAssertion.h"
#include "src/log/Logger.h"
#include "NacosString.h"
#include "Properties.h"
#include "constant/PropertyKeyConst.h"

using namespace std;
using namespace nacos;

bool testEndpointWithNamingProxy() {
    cout << "in function testEndpointWithNamingProxy" << endl;
    cout << "For this test, please create an endpoint on your 80 port with a file in the following path:" << endl;
    cout << "yourip:80/nacos/endpoint0" << endl;
    cout << "And the content should be a list of ip:port separated with \\n the ip:port group points at a nacos server" << endl;
    Properties configProps;
    ADD_AUTH_INFO(configProps);
    ADD_SPAS_INFO(configProps);
    configProps[PropertyKeyConst::ENDPOINT] = "127.0.0.1";
    configProps[PropertyKeyConst::ENDPOINT_PORT] = "80";
    configProps[PropertyKeyConst::CONTEXT_PATH] = "nacos";
    configProps[PropertyKeyConst::CLUSTER_NAME] = "endpoint0";

    INacosServiceFactory *factory = NacosFactoryFactory::getNacosFactory(configProps);
    ResourceGuard <INacosServiceFactory> _guardFactory(factory);
    NamingService *namingSvc = factory->CreateNamingService();
    ResourceGuard <NamingService> _serviceFactory(namingSvc);
    Instance instance;
    instance.clusterName = "DefaultCluster";
    instance.ip = "127.0.0.1";
    instance.port = 2333;
    instance.instanceId = "1";
    instance.ephemeral = true;

    try {
        for (int i = 0; i < 5; i++) {
            NacosString serviceName = "TestNamingService" + NacosStringOps::valueOf(i);
            instance.port = 2000 + i;
            namingSvc->registerInstance(serviceName, instance);
        }
    }
    catch (NacosException &e) {
        cout << "encounter exception while registering service instance, raison:" << e.what() << endl;
        return false;
    }
    cout << "Keep the services for 10 secs..." << endl;

    #if defined(_MSC_VER) || defined(__WIN32__) || defined(WIN32)
    Sleep(10000);
    #else
    sleep(30);
    #endif

    cout << "Deregister the services" << endl;
    try {
        for (int i = 0; i < 5; i++) {
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

    cout << "testNamingServiceRegister finished" << endl;

    return true;
}
