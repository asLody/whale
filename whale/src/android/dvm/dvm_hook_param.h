#ifndef WHALE_ANDROID_DVM_INTERCEPT_PARAM_H_
#define WHALE_ANDROID_DVM_INTERCEPT_PARAM_H_

#include <jni.h>
#include "base/primitive_types.h"
#include "ffi_cxx.h"

namespace whale {
namespace dvm {

struct DvmHookParam final {
    bool is_static_;					//标志位，标志函数是否为静态函数
    const char *shorty_;				//函数描述符缩写
    jobject addition_info_;
    u4 origin_access_flags;
    jobject origin_method_;
    jobject hooked_method_;
    volatile ptr_t decl_class_;
    jobject class_Loader_;				//
    jmethodID hooked_native_method_;
    jmethodID origin_native_method_;
    FFIClosure *jni_closure_;
};

}  // namespace dvm
}  // namespace whale

#endif  // WHALE_ANDROID_DVM_INTERCEPT_PARAM_H_
