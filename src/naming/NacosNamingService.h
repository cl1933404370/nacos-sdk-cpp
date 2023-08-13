#ifndef __NACOS_NAM_SVC_H_
#define __NACOS_NAM_SVC_H_

#include "Compatibility.h"
#include "NacosString.h"
#include "Properties.h"
#include "naming/Instance.h"
#include "naming/NamingService.h"
#include "src/factory/ObjectConfigData.h"
#include "src/http/IHttpCli.h"
#include "src/naming/NamingProxy.h"
#include "src/naming/beat/BeatReactor.h"
#include "src/naming/subscribe/EventDispatcher.h"

namespace nacos{
class NacosNamingService : public NamingService {
    ObjectConfigData *_objectConfigData;
    NacosString cacheDir;
    NacosString logName;
public:
    NacosNamingService() = delete;
    NacosNamingService(ObjectConfigData *objectConfigData);

    ~NacosNamingService() override;

    void registerInstance(const NacosString &serviceName, const NacosString &ip, int port) override NACOS_THROW(NacosException);

    void registerInstance(const NacosString &serviceName, const NacosString &groupName, const NacosString &ip,
                          int port) override NACOS_THROW(NacosException);

    void registerInstance(const NacosString &serviceName, const NacosString &ip, int port,
                          const NacosString &clusterName) override NACOS_THROW(NacosException);

    void registerInstance(const NacosString &serviceName, const NacosString &groupName, const NacosString &ip, int port,
                          const NacosString &clusterName) override NACOS_THROW(NacosException);

    void registerInstance(const NacosString &serviceName, Instance &instance) override NACOS_THROW(NacosException);

    void registerInstance(const NacosString &serviceName, const NacosString &groupName,
                          Instance &instance) override NACOS_THROW(NacosException);

    void deregisterInstance(const NacosString &serviceName, const NacosString &ip, int port) override NACOS_THROW(NacosException);

    void deregisterInstance(const NacosString &serviceName, const NacosString &groupName, const NacosString &ip,
                            int port) override NACOS_THROW(NacosException);

    void deregisterInstance(const NacosString &serviceName, const NacosString &ip, int port,
                            const NacosString &clusterName) override NACOS_THROW(NacosException);

    void
    deregisterInstance(const NacosString &serviceName, const NacosString &groupName, const NacosString &ip, int port,
                       const NacosString &clusterName) override NACOS_THROW(NacosException);

    void deregisterInstance(const NacosString &serviceName, const NacosString &groupName,
                            Instance &instance) override NACOS_THROW(NacosException);

    std::list <Instance> getAllInstances(const NacosString &serviceName) override NACOS_THROW(NacosException);

    std::list <Instance>
    getAllInstances(const NacosString &serviceName, const NacosString &groupName) override NACOS_THROW(NacosException);

    std::list <Instance>
    getAllInstances(const NacosString &serviceName, const std::list <NacosString> &clusters) override NACOS_THROW(NacosException);

    std::list <Instance> getAllInstances(const NacosString &serviceName, const NacosString &groupName,
                                         const std::list <NacosString> &clusters) override NACOS_THROW(NacosException);

    void subscribe(const NacosString &serviceName, EventListener *listener) override NACOS_THROW (NacosException);

    void subscribe(const NacosString &serviceName, const NacosString &groupName, const std::list<NacosString> &clusters, EventListener *listener) override NACOS_THROW (NacosException);

    void subscribe(const NacosString &serviceName, const NacosString &groupName, EventListener *listener) override NACOS_THROW (NacosException);

    void subscribe(const NacosString &serviceName, const std::list<NacosString> &clusters, EventListener *listener) override NACOS_THROW (NacosException);

    void unsubscribe(const NacosString &serviceName, EventListener *listener) override NACOS_THROW (NacosException);

    void unsubscribe(const NacosString &serviceName, const NacosString &groupName, EventListener *listener) override NACOS_THROW (NacosException);

    void unsubscribe(const NacosString &serviceName, const std::list<NacosString> &clusters, EventListener *listener) override NACOS_THROW (NacosException);

    void unsubscribe(const NacosString &serviceName, const NacosString &groupName, const std::list<NacosString> &clusters, EventListener *listener) override NACOS_THROW (NacosException);

    ListView<NacosString> getServiceList(int pageNo, int pageSize) override NACOS_THROW (NacosException);

    ListView<NacosString> getServiceList(int pageNo, int pageSize, const NacosString &groupName) override NACOS_THROW (NacosException);

    std::list<Instance> getInstanceWithPredicate(const NacosString &serviceName, const NacosString &groupName,
                                                 const std::list <NacosString> &clusters,
                                                 nacos::naming::selectors::Selector<Instance> *predicate) override NACOS_THROW(NacosException);

    std::list<Instance> getInstanceWithPredicate(const NacosString &serviceName,
                                                 const std::list <NacosString> &clusters,
                                                 nacos::naming::selectors::Selector<Instance> *predicate) override NACOS_THROW(NacosException);

    std::list<Instance> getInstanceWithPredicate(const NacosString &serviceName, const NacosString &groupName,
                                                 nacos::naming::selectors::Selector<Instance> *predicate) override NACOS_THROW(NacosException);

    std::list<Instance> getInstanceWithPredicate(const NacosString &serviceName,
                                                 nacos::naming::selectors::Selector<Instance> *predicate) override NACOS_THROW(NacosException);

    [[nodiscard]] IHttpCli *getHttpCli() const { return _objectConfigData->_httpCli; }

    [[nodiscard]] NamingProxy *getServerProxy() const { return _objectConfigData->_serverProxy; }

    [[nodiscard]] BeatReactor *getBeatReactor() const { return _objectConfigData->_beatReactor; }

    void setHttpCli(IHttpCli *httpCli) const { this->_objectConfigData->_httpCli = httpCli; }

    void setServerProxy(NamingProxy *_namingProxy) const { this->_objectConfigData->_serverProxy = _namingProxy; }

    void setBeatReactor(BeatReactor *_beatReactor) const { this->_objectConfigData->_beatReactor = _beatReactor; }
};
}//namespace nacos

#endif
