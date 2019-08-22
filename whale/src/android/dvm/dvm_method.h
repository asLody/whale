#ifndef WHALE_ANDROID_DVM_DVM_METHOD_H_
#define WHALE_ANDROID_DVM_DVM_METHOD_H_

#include <jni.h>
#include <android/android_build.h>
#include "android/dvm/dvm_runtime.h"
#include "android/modifiers.h"
#include "base/cxx_helper.h"
#include "base/primitive_types.h"

namespace whale {
namespace dvm {

class DvmMethod final {
 public:
    explicit DvmMethod(jmethodID method) : jni_method_(method) {
        DvmRuntime *runtime = DvmRuntime::Get();
        offset_ = runtime->GetDvmMethodOffsets();
        symbols_ = runtime->GetSymbols();
    }

	jmethodID getJmethod(){
		return jni_method_;
	}

    /*
     * Notice: This is a GcRoot reference.
     */
    ptr_t GetDeclaringClass() {
        return MemberOf<ptr_t>(jni_method_, 0);
    }


    jmethodID Clone(JNIEnv *env, u4 access_flags);

 private:
    jmethodID jni_method_;
    DvmMethodOffsets *offset_;
    DvmResolvedSymbols *symbols_;
};


}  // namespace dvm
}  // namespace whale

#endif  // WHALE_ANDROID_DVM_DVM_METHOD_H_
