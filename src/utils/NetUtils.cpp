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
#include <ifaddrs.h>
#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#endif

#define HOST_AND_LEN 250

namespace nacos{

NacosString NetUtils::getHostIp() NACOS_THROW(NacosException){
#if defined(_MSC_VER) || defined(__WIN32__) || defined(WIN32)
    ULONG family = AF_UNSPEC;
    ULONG flags = GAA_FLAG_INCLUDE_PREFIX | GAA_FLAG_SKIP_ANYCAST | GAA_FLAG_SKIP_MULTICAST | GAA_FLAG_SKIP_DNS_SERVER;
    ULONG size = 0;
    PIP_ADAPTER_ADDRESSES addresses = NULL;
    PIP_ADAPTER_ADDRESSES adapter = NULL;
    DWORD result = 0;

    result = GetAdaptersAddresses(family, flags, NULL, addresses, &size);
    if (result == ERROR_BUFFER_OVERFLOW) {
        addresses = (PIP_ADAPTER_ADDRESSES)malloc(size);
        if (addresses == NULL) {
            std::cerr << "Error allocating memory for adapter addresses" << std::endl;
            throw NacosException(NacosException::UNABLE_TO_GET_HOST_IP, "Error allocating memory for adapter addresses");
        }
        result = GetAdaptersAddresses(family, flags, NULL, addresses, &size);
    }
    if (result != 0L) {
        std::cerr << "Error getting adapter addresses: " << result << std::endl;
        throw NacosException(NacosException::UNABLE_TO_GET_HOST_IP, "Error getting adapter addresses");
    }

    // Find the adapter with the specified name
    const char* adapterName = "Ethernet"; // Replace with the name of the adapter you want to query
    int length1 = strlen(adapterName) + 1;
    int size1 = MultiByteToWideChar(CP_UTF8, 0, adapterName, length1, NULL, 0);
    if (size1 == 0) {
        std::cerr << "Error getting size of wide character string" << std::endl;
        throw NacosException(NacosException::UNABLE_TO_GET_HOST_IP, "Error getting size of wide character string");
    }
    PWCHAR wstr = new WCHAR[size1];
    if (MultiByteToWideChar(CP_UTF8, 0, adapterName, length1, wstr, size1) == 0) {
        std::cerr << "Error converting string to wide character string" << std::endl;
        delete[] wstr;
        throw NacosException(NacosException::UNABLE_TO_GET_HOST_IP, "Error converting string to wide character string");
    }
    
    for (adapter = addresses; adapter != NULL; adapter = adapter->Next) {
        if (wcscmp(adapter->FriendlyName, wstr) == 0) {
            break;
        }
    }
    if (adapter == NULL) {
        std::cerr << "Error finding adapter: " << adapterName << std::endl;
        throw NacosException(NacosException::UNABLE_TO_GET_HOST_IP, "Error finding adapter");
    }

    // Display the IP addresses associated with the adapter
    for (PIP_ADAPTER_UNICAST_ADDRESS address = adapter->FirstUnicastAddress; address != NULL; address = address->Next) {
        sockaddr* sa = address->Address.lpSockaddr;
        char host[NI_MAXHOST];
        DWORD size = NI_MAXHOST;
        int result = getnameinfo(sa, sa->sa_family == AF_INET ? sizeof(sockaddr_in) : sizeof(sockaddr_in6),
            host, size, NULL, 0, NI_NUMERICHOST);
        if (result != 0) {
            std::cerr << "Error getting address: " << result << std::endl;
            continue;
        }
        std::cout << "IP address: " << host << std::endl;
        free(addresses);
        return host;
    }
    return NULL;    
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
        if (ifa->ifa_addr == NULL || !(ifa->ifa_addr->sa_family==AF_INET)) {
            continue;
        }

        if((strcmp(ifa->ifa_name,"lo")==0)) {
            continue;
        }

        s = getnameinfo(ifa->ifa_addr, sizeof(struct sockaddr_in),host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
        if (s != 0) {
            freeifaddrs(ifaddr);
            throw NacosException(NacosException::UNABLE_TO_GET_HOST_IP, "Failed to get IF address");
        }

        log_debug("selected iface=%s ip=%s\n", ifa->ifa_name, host);
        freeifaddrs(ifaddr);
        return host;
    }
    //Usually the program will not run to here
    throw NacosException(NacosException::UNABLE_TO_GET_HOST_IP, "Failed to get IF address");
#endif
}

NacosString NetUtils::getHostName() NACOS_THROW(NacosException)
{
    char hostname[HOST_AND_LEN];
    
    int res = gethostname(hostname, HOST_AND_LEN);
    if (res == 0) {
        return NacosString(hostname);
    }

    throw NacosException(NacosException::UNABLE_TO_GET_HOST_NAME, "Failed to get hostname, errno = " + NacosStringOps::valueOf(errno));
}

}//namespace nacos
