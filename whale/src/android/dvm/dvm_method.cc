#include "android/dvm/dvm_method.h"
#include "android/well_known_classes.h"

namespace whale {
namespace dvm {

jmethodID DvmMethod::Clone(JNIEnv *env, u4 access_flags) {
    jmethodID jni_clone_method = reinterpret_cast<jmethodID>(malloc(sizeof(Method)));
	memcpy(jni_clone_method, jni_method_, sizeof(Method));
	hex_dump(jni_clone_method, sizeof(Method));
    return jni_clone_method;
}
}  // namespace dvm
}  // namespace whale
