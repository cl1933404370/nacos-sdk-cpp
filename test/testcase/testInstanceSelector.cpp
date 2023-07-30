#include <iostream>
#include <stdlib.h>
#if defined(_MSC_VER) || defined(__WIN32__) || defined(WIN32)
#include <io.h>
#include <process.h>
#else
#include <unistd.h>
#endif
#include <list>
#include "factory/NacosFactoryFactory.h"
#include "naming/Instance.h"
#include "naming/selectors/RandomByWeightSelector.h"
#include "naming/selectors/HealthInstanceSelector.h"
#include "naming/selectors/RandomSelector.h"
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
using namespace nacos::naming::selectors;

bool testInstanceSelectors() {
    cout << "in function testInstanceSelectors" << endl;
    Properties configProps;
    ADD_AUTH_INFO(configProps);
    ADD_SPAS_INFO(configProps);
    configProps[PropertyKeyConst::SERVER_ADDR] = "127.0.0.1";
    configProps[PropertyKeyConst::SUBSCRIPTION_POLL_INTERVAL] = "3000";

    INacosServiceFactory *factory = NacosFactoryFactory::getNacosFactory(configProps);
    ResourceGuard <INacosServiceFactory> _guardFactory(factory);
    NamingService *namingSvc = factory->CreateNamingService();
    ResourceGuard <NamingService> _guardService(namingSvc);

    list<Instance> res;
    HealthInstanceSelector healthInstanceSelector;
    RandomSelector randomSelector;

    Instance instance;
    instance.clusterName = "DefaultCluster";
    instance.ip = "127.0.0.1";
    instance.port = 2333;
    instance.instanceId = "1";
    instance.ephemeral = true;

    try {
        for (int i = 0; i < 10; i++) {
            NacosString serviceName = "TestNamingService0";
            instance.port = 2000 + i;
            namingSvc->registerInstance(serviceName, instance);
        }
        res = namingSvc->getInstanceWithPredicate("TestNamingService0", &randomSelector);
    }
    catch (NacosException &e) {
        cout << "encounter exception while getting service names, raison:" << e.what() << endl;
        return false;
    }

    cout << "Selected instance(s):" << endl;
    for (list<Instance>::const_iterator it = res.begin();
         it != res.end(); it++) {
        cout << "service:" << it->toString() << endl;
    }

    return true;
}

bool testRandomByWeightSelector()
{

    cout << "in function testRandomByWeightSelector" << endl;
    Properties configProps;
    ADD_AUTH_INFO(configProps);
    ADD_SPAS_INFO(configProps);
    configProps[PropertyKeyConst::SERVER_ADDR] = "127.0.0.1";
    configProps[PropertyKeyConst::SUBSCRIPTION_POLL_INTERVAL] = "3000";

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
    RandomByWeightSelector randomByWeightSelector;

    try {
        for (int i = 0; i < 10; i++) {
            NacosString serviceName = "TestNamingService0";
            instance.port = 2000 + i;
            instance.weight = i + 1;
            namingSvc->registerInstance(serviceName, instance);
        }
    }
    catch (NacosException &e) {
        cout << "encounter exception while registering service instance, raison:" << e.what() << endl;
        return false;
    }

    cout << "start to select instance with selectors " << endl;

    for (int i = 0; i < 20; i++)
    {
        list<Instance> res = namingSvc->getInstanceWithPredicate("TestNamingService0", &randomByWeightSelector);

        for (const auto& re : res)
        {
            cout << "service:" << re.toString() << endl;
        }
    }

    return true;
}