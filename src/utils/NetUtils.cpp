#include "NetUtils.h"
#include <errno.h>
#include <src/log/Logger.h>
#include <string.h>

#if defined(_MSC_VER) || defined(__WIN32__) || defined(WIN32)
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <iphlpapi.h>
#include <io.h>
#include <process.h>
#include <winerror.h>
#pragma comment(lib, "IPHLPAPI.lib")
#pragma comment(lib, "Ws2_32.lib")
#else
#include <unistd.h>
#include <ifaddrs.h>
#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#endif

#define HOST_AND_LEN 250

namespace nacos
{

    NacosString NetUtils::getHostIp() NACOS_THROW(NacosException)
    {
#if defined(_MSC_VER) || defined(__WIN32__) || defined(WIN32)
        PIP_ADAPTER_ADDRESSES pAddresses = NULL;
        ULONG family = AF_INET;
        ULONG flags = GAA_FLAG_INCLUDE_PREFIX;
        ULONG bufferSize = 0;
        DWORD result = GetAdaptersAddresses(family, flags, NULL, pAddresses, &bufferSize);
        if (result == ERROR_BUFFER_OVERFLOW)
        {
            pAddresses = (PIP_ADAPTER_ADDRESSES)malloc(bufferSize);
            result = GetAdaptersAddresses(family, flags, NULL, pAddresses, &bufferSize);
        }
        if (result != 0L)
        {
            throw std::runtime_error("Failed to get adapter addresses");
        }
        PIP_ADAPTER_ADDRESSES adapter = pAddresses;
        WSADATA wsaData; // Declare a variable of type WSADATA to store details of the Winsock implementation
        WSAStartup(MAKEWORD(2, 2), &wsaData);    
        sockaddr *sa = nullptr;
        char host[NI_MAXHOST];
        DWORD size = NI_MAXHOST;
        while (adapter)
        {
             if (adapter->IfType != IF_TYPE_ETHERNET_CSMACD && adapter->IfType != IF_TYPE_SOFTWARE_LOOPBACK && adapter->OperStatus == IfOperStatusUp)
             {
                PIP_ADAPTER_UNICAST_ADDRESS address = adapter->FirstUnicastAddress;
                while (address)
                {
                    sa = address->Address.lpSockaddr;
                    int result = getnameinfo(sa, sa->sa_family == AF_INET ? sizeof(sockaddr_in) : sizeof(sockaddr_in6),
                                             host, size, NULL, 0, NI_NUMERICHOST);
                    if (result == 0)
                    {
                        free(pAddresses);
                        WSACleanup();
                        return std::string(host);
                    }
                    else
                    {
                        printf("getnameinfo failed with error # %ld\n", WSAGetLastError());
                    }
                    address = address->Next;
                }
             }
            adapter = adapter->Next;
        }
        free(pAddresses);
        WSACleanup();
        throw std::runtime_error("Failed to get local IP address");
        return "";
#else
        struct ifaddrs *ifaddr, *ifa;
        int s;
        char host[NI_MAXHOST];

        if (getifaddrs(&ifaddr) == -1)
        {
            throw NacosException(NacosException::UNABLE_TO_GET_HOST_IP, "Failed to get IF address");
        }

        for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next)
        {
            log_debug("iterating on iface=%s\n", ifa->ifa_name);
            if (ifa->ifa_addr == NULL || !(ifa->ifa_addr->sa_family == AF_INET))
            {
                continue;
            }

            if ((strcmp(ifa->ifa_name, "lo") == 0))
            {
                continue;
            }

            s = getnameinfo(ifa->ifa_addr, sizeof(struct sockaddr_in), host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
            if (s != 0)
            {
                freeifaddrs(ifaddr);
                throw NacosException(NacosException::UNABLE_TO_GET_HOST_IP, "Failed to get IF address");
            }

            log_debug("selected iface=%s ip=%s\n", ifa->ifa_name, host);
            freeifaddrs(ifaddr);
            return host;
        }
        // Usually the program will not run to here
        throw NacosException(NacosException::UNABLE_TO_GET_HOST_IP, "Failed to get IF address");
#endif
    }

    NacosString NetUtils::getHostName() NACOS_THROW(NacosException)
    {
        char hostname[HOST_AND_LEN];

        int res = gethostname(hostname, HOST_AND_LEN);
        if (res == 0)
        {
            return NacosString(hostname);
        }

        throw NacosException(NacosException::UNABLE_TO_GET_HOST_NAME, "Failed to get hostname, errno = " + NacosStringOps::valueOf(errno));
    }
}
        