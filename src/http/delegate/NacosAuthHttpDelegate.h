//
// Created by liuhanyu on 2020/12/5.
//

#ifndef NACOS_SDK_CPP_NACOSAUTHHTTPDELEGATE_H
#define NACOS_SDK_CPP_NACOSAUTHHTTPDELEGATE_H

#include "NacosExceptions.h"
#include "NacosString.h"
#include "src/http/HttpDelegate.h"
#include "src/factory/ObjectConfigData.h"
#include "Compatibility.h"

/**
 * NoOpHttpDelegate
 *
 * @author Liu, Hanyu
 * Send a request to the server with authentication header
 */
namespace nacos{
    class NacosAuthHttpDelegate : public HttpDelegate {
        ObjectConfigData *_objectConfigData;
        NacosString encoding;
    public:
        NacosAuthHttpDelegate(ObjectConfigData *objectConfigData);

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

        ~NacosAuthHttpDelegate() override = default;
    };
}//namespace nacos

#endif //NACOS_SDK_CPP_NACOSAUTHHTTPDELEGATE_H
