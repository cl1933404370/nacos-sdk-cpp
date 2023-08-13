//
// Created by liuhanyu on 2020/9/6.
//

#ifndef NACOS_SDK_CPP_RESOURCEGUARD_H
#define NACOS_SDK_CPP_RESOURCEGUARD_H

namespace nacos{
template<typename T>
class ResourceGuard {
private:
    T *_obj;//The object being watched
    ResourceGuard();

public:
    ResourceGuard(T *obj) { _obj = obj; };

    ~ResourceGuard() {
        delete _obj;
        _obj = NULL;
    };
};
}//namespace nacos


#endif //NACOS_SDK_CPP_RESOURCEGUARD_H
