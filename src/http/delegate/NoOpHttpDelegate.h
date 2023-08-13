#ifndef __SVR_HTTP_AGENT_H_
#define __SVR_HTTP_AGENT_H_

#include "NacosExceptions.h"
#include "NacosString.h"
#include "src/http/HttpDelegate.h"
#include "src/factory/ObjectConfigData.h"
#include "Compatibility.h"

/**
 * NoOpHttpDelegate
 *
 * @author Liu, Hanyu
 * Directly send request to HttpCli without any operation
 */
namespace nacos{
class NoOpHttpDelegate : public HttpDelegate {
private:
    ObjectConfigData *_objectConfigData;
    NacosString encoding;
public:
    NoOpHttpDelegate(ObjectConfigData *objectConfigData);

    HttpResult httpGet(const NacosString &path, std::list <NacosString> &headers, std::list <NacosString> &paramValues,
                       const NacosString &encoding, long readTimeoutMs) override NACOS_THROW(NetworkException);

    HttpResult httpPost(const NacosString &path, std::list <NacosString> &headers, std::list <NacosString> &paramValues,
                        const NacosString &encoding, long readTimeoutMs) override NACOS_THROW(NetworkException);

    HttpResult
    httpPut(const NacosString &path, std::list <NacosString> &headers, std::list <NacosString> &paramValues,
            const NacosString &encoding, long readTimeoutMs) override NACOS_THROW(NetworkException);

    HttpResult
    httpDelete(const NacosString &path, std::list <NacosString> &headers, std::list <NacosString> &paramValues,
               const NacosString &encoding, long readTimeoutMs) override NACOS_THROW(NetworkException);

    NacosString getEncode() const override;

    ~NoOpHttpDelegate() override = default;
};
}//namespace nacos

#endif