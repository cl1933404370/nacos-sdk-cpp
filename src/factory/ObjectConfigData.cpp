#include "ObjectConfigData.h"
#include "src/http/HttpDelegate.h"
#include "src/naming/NamingProxy.h"
#include "src/naming/beat/BeatReactor.h"
#include "src/naming/subscribe/EventDispatcher.h"
#include "src/naming/subscribe/SubscriptionPoller.h"
#include "src/naming/subscribe/UdpNamingServiceListener.h"
#include "src/naming/subscribe/HostReactor.h"
#include "src/listen/ClientWorker.h"
#include "src/security/SecurityManager.h"
#include "src/utils/UuidUtils.h"
#include "src/utils/SequenceProvider.h"
#include "src/config/ConfigProxy.h"
#include "src/debug/DebugAssertion.h"

namespace nacos{

ObjectConfigData::ObjectConfigData(FactoryType theFactoryType) {
    objectId = UuidUtils::generateUuid();
    factoryType = theFactoryType;
    _httpDelegate = nullptr;
    _httpCli = nullptr;
    _serverProxy = nullptr;
    _beatReactor = nullptr;
    _eventDispatcher = nullptr;
    _subscriptionPoller = nullptr;
    _appConfigManager = nullptr;
    _serverListManager = nullptr;
    _clientWorker = nullptr;
    _localSnapshotManager = nullptr;
    _securityManager = nullptr;
    _configProxy = nullptr;
}

void ObjectConfigData::checkNamingService() NACOS_THROW(NacosException) {
    if (factoryType != NAMING) {
        throw NacosException(NacosException::INVALID_PARAM, "Invalid configuration for naming service, please check");
    }

    NACOS_ASSERT(_httpDelegate != NULL);
    NACOS_ASSERT(_httpCli != NULL);
    NACOS_ASSERT(_serverProxy != NULL);
    NACOS_ASSERT(_beatReactor != NULL);
    NACOS_ASSERT(_eventDispatcher != NULL);
    NACOS_ASSERT(_subscriptionPoller != NULL);
    NACOS_ASSERT(_hostReactor != NULL);
    NACOS_ASSERT(_appConfigManager != NULL);
    NACOS_ASSERT(_serverListManager != NULL);
    NACOS_ASSERT(_udpNamingServiceListener != NULL);
    NACOS_ASSERT(_udpNamingServiceListener != NULL);
    NACOS_ASSERT(_sequenceProvider != NULL);
}

void ObjectConfigData::checkConfigService() NACOS_THROW(NacosException) {
    if (factoryType != CONFIG) {
        throw NacosException(NacosException::INVALID_PARAM, "Invalid configuration for config service, please check");
    }

    NACOS_ASSERT(_appConfigManager != NULL);
    NACOS_ASSERT(_httpCli != NULL);
    NACOS_ASSERT(_httpDelegate != NULL);
    NACOS_ASSERT(_serverListManager != NULL);
    NACOS_ASSERT(_clientWorker != NULL);
    NACOS_ASSERT(_localSnapshotManager != NULL);
    NACOS_ASSERT(_configProxy != NULL);
}

void ObjectConfigData::checkMaintainService() NACOS_THROW(NacosException) {
    if (factoryType != MAINTAIN) {
        throw NacosException(NacosException::INVALID_PARAM, "Invalid configuration for maintain service, please check");
    }

    NACOS_ASSERT(_serverProxy != NULL);
    NACOS_ASSERT(_httpDelegate != NULL);
    NACOS_ASSERT(_httpCli != NULL);
    NACOS_ASSERT(_appConfigManager != NULL);
    NACOS_ASSERT(_serverListManager != NULL);
}

void ObjectConfigData::destroyConfigService() {

    if (_clientWorker != nullptr) {
        _clientWorker->stopListening();  
    }

    if (_securityManager != nullptr) {
        _securityManager->stop();
    }

    if (_serverListManager) {
        _serverListManager->stop();
    }

    if (_clientWorker != nullptr) {
        _clientWorker->stopListening();
        delete _clientWorker; 
        _clientWorker = nullptr;
    }

    if (_httpDelegate != nullptr) {
        delete _httpDelegate;
        _httpDelegate = nullptr;
    }

    if (_securityManager != nullptr) {
        delete _securityManager;
        _securityManager = nullptr;
    }

    if (_serverListManager != nullptr) {
        delete _serverListManager;
        _serverListManager = nullptr;
    }

    if (_httpCli != nullptr) {
        delete _httpCli;
        _httpCli = nullptr;
    }

    if (_appConfigManager != nullptr) {
        delete _appConfigManager;
        _appConfigManager = nullptr;
    }

    if (_configProxy != nullptr) {
        delete _configProxy;
        _configProxy = nullptr;
    }
}

void ObjectConfigData::destroyNamingService() {

    if (_beatReactor != nullptr) {
        _beatReactor->stop();
    }

    if (_subscriptionPoller != nullptr) {
        _subscriptionPoller->stop();
    }

    if (_udpNamingServiceListener != nullptr) {
        _udpNamingServiceListener->stop();
    }

    if (_eventDispatcher != nullptr) {
        _eventDispatcher->stop();
    }

    if (_securityManager != nullptr) {
        _securityManager->stop();
    }

    if (_serverListManager) {
        _serverListManager->stop();
    }

    if (_httpDelegate != nullptr) {
        delete _httpDelegate;
        _httpDelegate = nullptr;
    }

    if (_beatReactor != nullptr) {
        delete _beatReactor;
        _beatReactor = nullptr;
    }

    if (_subscriptionPoller != nullptr)
    {
        delete _subscriptionPoller;
        _subscriptionPoller = nullptr;
    }

    if (_udpNamingServiceListener != nullptr)
    {
        delete _udpNamingServiceListener;
        _udpNamingServiceListener = nullptr;
    }

    if (_eventDispatcher != nullptr)
    {
        delete _eventDispatcher;
        _eventDispatcher = nullptr;
    }

    if (_hostReactor != nullptr) {
        delete _hostReactor;
        _hostReactor = nullptr;
    }

    if (_serverProxy != nullptr) {
        delete _serverProxy;
        _serverProxy = nullptr;
    }

    if (_securityManager != nullptr) {
        delete _securityManager;
        _securityManager = nullptr;
    }

    if (_serverListManager != nullptr) {
        delete _serverListManager;
        _serverListManager = nullptr;
    }

    if (_httpDelegate != nullptr) {
        delete _httpDelegate;
        _httpDelegate = nullptr;
    }

    if (_httpCli != nullptr) {
        delete _httpCli;
        _httpCli = nullptr;
    }

    if (_appConfigManager != nullptr)
    {
        delete _appConfigManager;
        _appConfigManager = nullptr;
    }

    if (_sequenceProvider != nullptr)
    {
        delete _sequenceProvider;
        _sequenceProvider = nullptr;
    }
}

void ObjectConfigData::destroyMaintainService() {
    if (_serverListManager != nullptr) {
        _serverListManager->stop();
    }

    if (_securityManager != nullptr) {
        _securityManager->stop();
    }

    if (_serverProxy != nullptr) {
        delete _serverProxy;
        _serverProxy = nullptr;
    }
    if (_serverListManager != nullptr) {
        delete _serverListManager;
        _serverListManager = nullptr;
    }
    if (_appConfigManager != nullptr) {
        delete _appConfigManager;
        _appConfigManager = nullptr;
    }
    if (_securityManager != nullptr) {
        delete _securityManager;
        _securityManager = nullptr;
    }
    if (_httpDelegate != nullptr) {
        delete _httpDelegate;
        _httpDelegate = nullptr;
    }
    if (_httpCli != nullptr) {
        delete _httpCli;
        _httpCli = nullptr;
    }
}

void ObjectConfigData::checkAssembledObject() NACOS_THROW(NacosException) {
    switch (factoryType) {
    case NAMING:
        checkNamingService();
        return;
    case CONFIG:
        checkConfigService();
        return;
    case MAINTAIN:
        checkMaintainService();
        break;
    default:
        abort();//never happens
    }
}

ObjectConfigData::~ObjectConfigData() {
    switch (factoryType) {
    case NAMING:
        destroyNamingService();
        return;
    case CONFIG:
        destroyConfigService();
        return;
    case MAINTAIN:
        destroyMaintainService();
        break;
    default:
        abort();//never happens
    }
}

}//namespace nacos
