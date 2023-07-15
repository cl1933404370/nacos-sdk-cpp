#include <stdlib.h>

#if defined(_MSC_VER) || defined(__WIN32__) || defined(WIN32)
#include <thread>
#include <chrono>
#else
#include <unistd.h>
#endif /* _UNISTD_H */

#include "ServerListManager.h"

#include <algorithm>

#include "constant/PropertyKeyConst.h"
#include "src/utils/ParamUtils.h"
#include "src/log/Logger.h"
#include "src/json/JSON.h"

using namespace std;

namespace nacos
{
    void ServerListManager::addToSrvList(NacosString& address)
    {
        address = ParamUtils::trim(address);
        NacosString address_lc = ParamUtils::toLower(address);
        if (address_lc.find("http://") == 0 ||
            address_lc.find("https://") == 0)
        {
            size_t startPos = address.find(':'); // 4=http,5=https
            // use http://someaddress[:port] as server address
            NacosString ip = address;
            int port = PropertyKeyConst::NACOS_DEFAULT_PORT;
            size_t pos = address.find_last_of(':');
            if (pos != 4 && pos != 5)
            {
                NacosString portStr = address.substr(pos + 1);
                port = atoi(portStr.c_str());
                ip = address.substr(0, pos);
            }
            NacosServerInfo curServer;
            curServer.setKey(address);
            curServer.setAlive(true);
            curServer.setIp(ip);
            curServer.setPort(port);
            curServer.setWeight(1.0);
            curServer.setAdWeight(1.0);
            curServer.setMode(startPos == 4 ? NacosServerInfo::mode_http : NacosServerInfo::mode_http_safe);
            serverList.push_back(curServer);
        }
        else if (address.find(':') == std::string::npos)
        {
            // If the address doesn't contain port, add 8848 as the default port for it
            NacosServerInfo curServer;
            curServer.setKey("http://" + address + ":" + NacosStringOps::valueOf(PropertyKeyConst::NACOS_DEFAULT_PORT));
            curServer.setAlive(true);
            curServer.setIp("http://" + address);
            curServer.setPort(PropertyKeyConst::NACOS_DEFAULT_PORT);
            curServer.setWeight(1.0);
            curServer.setAdWeight(1.0);
            curServer.setMode(NacosServerInfo::mode_http);
            serverList.push_back(curServer);
        }
        else
        {
            // user specified address & port
            vector<NacosString> explodedAddress;
            ParamUtils::Explode(explodedAddress, address, ':');
            NacosServerInfo curServer;
            curServer.setKey("http://" + address);
            curServer.setAlive(true);
            curServer.setIp("http://" + explodedAddress[0]);
            curServer.setPort(atoi(explodedAddress[1].c_str()));
            curServer.setWeight(1.0);
            curServer.setAdWeight(1.0);
            curServer.setMode(NacosServerInfo::mode_http);
            serverList.push_back(curServer);
        }
    }

    ServerListManager::ServerListManager(list<NacosString>& fixed)
    {
        started = false;
        isFixed = true;
        _pullThread = NULL;
        refreshInterval = 30000;
        for (auto it = fixed.begin(); it != fixed.end(); it++)
        {
            addToSrvList(*it);
        }
    }

    NacosString ServerListManager::getCurrentServerAddr()
    {
        // By default, this function returns a server in the serverList randomly
        // later we should sort it according to the java client and use cache
        ReadGuard _readGuard(rwLock);
        size_t max_serv_slot = serverList.size();
        srand(static_cast<unsigned>(time(nullptr)));
        const int to_skip = rand() % max_serv_slot;
        auto it = serverList.begin();
        for (int skipper = 0; skipper < to_skip; skipper++)
        {
            ++it;
        }

        return it->getCompleteAddress();
    }

    void ServerListManager::initAll() NACOS_THROW(NacosException)
    {
        serverList.clear();
        Properties props = _objectConfigData->_appConfigManager->getAllConfig();
        if (props.contains(PropertyKeyConst::SERVER_ADDR))
        {
            // Server address is configured
            isFixed = true;
            NacosString server_addr = props[PropertyKeyConst::SERVER_ADDR];

            vector<NacosString> explodedServers;
            ParamUtils::Explode(explodedServers, server_addr, ',');
            for (auto& explodedServer : explodedServers)
            {
                addToSrvList(explodedServer);
            }

            serverList.sort();
        }
        else
        {
            // use endpoint mode to pull nacos server info from server
            if (!props.contains(PropertyKeyConst::ENDPOINT))
            {
                throw NacosException(NacosException::CLIENT_INVALID_PARAM,
                                     "no server address specified and the endpoint is blank");
            }

            isFixed = false;
            NacosString endpoint = getEndpoint();
            NacosString endpoint_lc = ParamUtils::toLower(endpoint);
            // endpoint doesn't start with http or https prefix, consider it as http
            if (!(endpoint_lc.find("http://") == 0) && !(endpoint_lc.find("https://") == 0))
            {
                endpoint = "http://" + endpoint;
            }
            if (NacosStringOps::isNullStr(getNamespace()))
            {
                addressServerUrl = endpoint + ":" + NacosStringOps::valueOf(getEndpointPort()) + "/" +
                    getContextPath() + "/" + getClusterName();
            }
            else
            {
                addressServerUrl = endpoint + ":" + NacosStringOps::valueOf(getEndpointPort()) + "/" +
                    getContextPath() + "/" + getClusterName() + "?namespace=" + getNamespace();
            }

            log_debug("Assembled addressServerUrl:%s\n", addressServerUrl.c_str());

            serverList = pullServerList();
            start();
        }
    }

    ServerListManager::ServerListManager(ObjectConfigData* objectConfigData) NACOS_THROW(NacosException)
    {
        started = false;
        _pullThread = nullptr;
        _objectConfigData = objectConfigData;
        try
        {
           
            refreshInterval = std::strtol(
                _objectConfigData->_appConfigManager->get(PropertyKeyConst::SRVLISTMGR_REFRESH_INTERVAL).c_str(), nullptr,
                10);
        }
        catch (...)
        {
            refreshInterval = 30000;
        }
        initAll();
    }

    list<NacosServerInfo> ServerListManager::tryPullServerListFromNacosServer() NACOS_THROW(NacosException)
    {
        std::list<NacosString> headers;
        std::list<NacosString> paramValues;
        size_t maxSvrSlot = serverList.size();
        if (maxSvrSlot == 0)
        {
            throw NacosException(0, "failed to get nacos servers, raison: no server(s) available");
        }
        log_debug("nr_servers:%d\n", maxSvrSlot);
        srand(static_cast<unsigned int>(time(nullptr)));

        const long _read_timeout = _objectConfigData->_appConfigManager->getServeReqTimeout();
        NacosString errmsg;
        for (size_t i = 0; i < serverList.size(); i++)
        {
            size_t selectedServer = rand() % maxSvrSlot;
            const NacosServerInfo& server = ParamUtils::getNthElem(serverList, selectedServer);
            log_debug("selected_server:%d\n", selectedServer);
            log_debug("Trying to access server:%s\n", server.getCompleteAddress().c_str());
            try
            {
                HttpResult serverRes = _objectConfigData->_httpDelegate->httpGet(
                    server.getCompleteAddress() + "/" + _objectConfigData->_appConfigManager->getContextPath() + "/" +
                    ConfigConstant::PROTOCOL_VERSION + "/" + ConfigConstant::GET_SERVERS_PATH,
                    headers, paramValues, NULLSTR, _read_timeout);
                return JSON::Json2NacosServerInfo(serverRes.content);
            }
            catch (NacosException& e)
            {
                errmsg = e.what();
                log_error("request %s failed.\n", server.getCompleteAddress().c_str());
            }
            catch (exception& e)
            {
                errmsg = e.what();
                log_error("request %s failed.\n", server.getCompleteAddress().c_str());
            }

            selectedServer = (selectedServer + 1) % serverList.size();
        }

        throw NacosException(0,
                             "failed to get nacos servers after all servers(" + toString() + ") tried: " + errmsg);
    }

    list<NacosServerInfo> ServerListManager::pullServerList() NACOS_THROW(NacosException)
    {
        std::list<NacosString> headers;
        std::list<NacosString> paramValues;

        long _read_timeout = _objectConfigData->_appConfigManager->getServeReqTimeout();
        if (!NacosStringOps::isNullStr(addressServerUrl))
        {
            HttpResult serverRes = _objectConfigData->_httpDelegate->httpGet(
                addressServerUrl, headers, paramValues, NULLSTR,
                _read_timeout);
            list<NacosString> explodedServerList;
            ParamUtils::Explode(explodedServerList, serverRes.content, '\n');
            list<NacosServerInfo> serversPulled;

            for (list<NacosString>::const_iterator it = explodedServerList.begin();
                 it != explodedServerList.end(); it++)
            {
                NacosServerInfo curServer;
                size_t pos = it->find(":");
                if (pos == std::string::npos)
                {
                    curServer.setIp(*it);
                    curServer.setPort(8848);
                }
                else
                {
                    NacosString ip = it->substr(0, pos);
                    NacosString port = it->substr(pos + 1);

                    curServer.setIp(ip);
                    curServer.setPort(atoi(port.c_str()));
                }
                curServer.setAlive(true);
                serversPulled.push_back(curServer);
            }

            serversPulled.sort();

            log_debug("pullServerList: servers list: %s\n", serverListToString(serversPulled).c_str());
            return serversPulled;
        }
        // usually this should not be happening
        throw NacosException(0, "addressServerUrl is not set, please config it on properties");
    }

    std::list<NacosServerInfo> ServerListManager::__debug()
    {
        return tryPullServerListFromNacosServer();
    }

    NacosString ServerListManager::serverListToString(const std::list<NacosServerInfo>& serverList)
    {
        NacosString res;
        bool first = true;
        for (auto it = serverList.begin(); it != serverList.end(); it++)
        {
            if (first)
            {
                first = false;
            }
            else
            {
                res += ",";
            }
            res += it->toString();
        }

        return res;
    }

    NacosString ServerListManager::toString() const
    {
        return serverListToString(serverList);
    }

    void* ServerListManager::pullWorkerThread(void* param)
    {
        auto thisMgr = (ServerListManager*)param;
        while (thisMgr->started)
        {
            try
            {
                bool changed = false;
                list<NacosServerInfo> serverList = thisMgr->pullServerList();

                {
                    ReadGuard _readGuard(thisMgr->rwLock);
                    if (serverList != thisMgr->serverList)
                    {
                        log_debug("Servers got from nacos differs from local one, updating...\n");
                        changed = true;
                    }
                }

                if (changed)
                {
                    WriteGuard _writeGuard(thisMgr->rwLock);
                    log_debug("updated!\n");
                    thisMgr->serverList = serverList;
                }
            }
            catch ([[maybe_unused]] NacosException& e)
            {
#if defined(_MSC_VER) || defined(__WIN32__) || defined(WIN32)
                std::this_thread::sleep_for(std::chrono::seconds(thisMgr->refreshInterval / 1000));
#else
    // Error occured during the invocation, sleep for a longer time
    sleep(thisMgr->refreshInterval / 1000);
#endif
            }

#if defined(_MSC_VER) || defined(__WIN32__) || defined(WIN32)
            std::this_thread::sleep_for(std::chrono::seconds(thisMgr->refreshInterval / 1000));
#else
   sleep(thisMgr->refreshInterval / 1000);
#endif
        }

        return NULL;
    }

    void ServerListManager::start()
    {
        if (started && !isFixed)
        {
            return;
        }

        started = true;

        if (_pullThread == NULL)
        {
            NacosString threadName = getClusterName() + "," + getEndpoint() + ":" +
                NacosStringOps::valueOf(getEndpointPort()) + "-" + getNamespace();
            _pullThread = new Thread(threadName, pullWorkerThread, (void*)this);
        }
        _pullThread->start();
    }

    void ServerListManager::stop()
    {
        if (!started)
        {
            return;
        }

        started = false;
        if (_pullThread != NULL)
        {
            _pullThread->join();
            _pullThread = NULL;
        }
    }

    const NacosString& ServerListManager::getContextPath() const
    {
        return _objectConfigData->_appConfigManager->getContextPath();
    }

    ServerListManager::~ServerListManager()
    {
        stop();
        if (_pullThread != NULL)
        {
            delete _pullThread;
            _pullThread = NULL;
        }
    }

    size_t ServerListManager::getServerCount()
    {
        size_t count = 0;
        {
            ReadGuard _readGuard(rwLock);
            count = serverList.size();
        }
        return count;
    };

    list<NacosServerInfo> ServerListManager::getServerList()
    {
        // further optimization could be implemented here if the server list cannot be changed during runtime
        std::list<NacosServerInfo> res;
        {
            ReadGuard _readGuard(rwLock);
            res = serverList;
        }
        return res;
    };
} // namespace nacos
