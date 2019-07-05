#ifndef WHALE_ANDROID_ART_NATIVE_ON_LOAD_H_
#define WHALE_ANDROID_ART_NATIVE_ON_LOAD_H_

#include <jni.h>

constexpr const char *kMethodReserved0 = "reserved0";
constexpr const char *kMethodReserved1 = "reserved1";

#define DEX_PATH "/data/local/tmp/whale.dex"
#define CLASS_NAME "com/lody/whale/WhaleRuntime"
#define J_CLASS_NAME "com.lody.whale.WhaleRuntime"

/**
 * DO NOT rename the following function
 */
extern "C" {

void WhaleRuntime_reserved0(JNIEnv *env, jclass cl);

void WhaleRuntime_reserved1(JNIEnv *env, jclass cl);

}

void WhaleRuntime_handleCallAppOnCreate(JNI_START);

#ifndef WHALE_ANDROID_AUTO_LOAD
JNIEXPORT jint JNICALL Whale_OnLoad(JavaVM *vm, void *reserved);
#endif


#endif  // WHALE_ANDROID_ART_NATIVE_ON_LOAD_H_
