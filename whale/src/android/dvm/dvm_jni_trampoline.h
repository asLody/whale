#ifndef WHALE_ANDROID_DVM_JNI_TRAMPOLINE_H_
#define WHALE_ANDROID_DVM_JNI_TRAMPOLINE_H_

#include "android/dvm/dvm_hook_param.h"

namespace whale {
namespace dvm {


void BuildJniClosure(DvmHookParam *param);


}  // namespace dvm
}  // namespace whale

#endif  // WHALE_ANDROID_DVM_JNI_TRAMPOLINE_H_
