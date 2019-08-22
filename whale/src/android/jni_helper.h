#ifndef WHALE_ANDROID_ART_JNI_HELPER_H_
#define WHALE_ANDROID_ART_JNI_HELPER_H_

#include <jni.h>
#include <base/macros.h>

namespace whale {
#if defined(__LP64__)
static constexpr const char *kAndroidLibDir = "/system/lib64/";
static constexpr const char *kLibNativeBridgePath = "/system/lib64/libnativebridge.so";
static constexpr const char *kLibArtPath = "/system/lib64/libart.so";
static constexpr const char *kLibDvmPath = "/system/lib/libdvm.so";
static constexpr const char *kLibAocPath = "/system/lib64/libaoc.so";
static constexpr const char *kLibHoudiniArtPath = "/system/lib64/arm64/libart.so";
static constexpr const char *kLibHoudiniDvmPath = "/system/lib/arm/libdvm.so";
#else
static constexpr const char *kAndroidLibDir = "/system/lib/";
static constexpr const char *kLibArtPath = "/system/lib/libart.so";
static constexpr const char *kLibDvmPath = "/system/lib/libdvm.so";
static constexpr const char *kLibAocPath = "/system/lib/libaoc.so";
static constexpr const char *kLibHoudiniArtPath = "/system/lib/arm/libart.so";
static constexpr const char *kLibHoudiniDvmPath = "/system/lib/arm/libdvm.so";
#endif


#define NATIVE_METHOD(className, functionName, signature) \
    { #functionName, signature, reinterpret_cast<void*>(className ## _ ## functionName) }

#define NELEM(x) (sizeof(x)/sizeof((x)[0]))

#define JNI_START JNIEnv *env, jclass cl

static inline void JNIExceptionClear(JNIEnv *env) {
    if (env->ExceptionCheck()) {
        env->ExceptionClear();
    }
}

static inline bool JNIExceptionCheck(JNIEnv *env) {
    if (env->ExceptionCheck()) {
        jthrowable e = env->ExceptionOccurred();
        env->Throw(e);
        env->DeleteLocalRef(e);
        return true;
    }
    return false;
}

static inline void JNIExceptionClearAndDescribe(JNIEnv *env) {
    if (env->ExceptionCheck()) {
        env->ExceptionDescribe();
        env->ExceptionClear();
    }
}

template<typename T>
class ScopedLocalRef {
 public:
    ScopedLocalRef(JNIEnv *env, T localRef) : mEnv(env), mLocalRef(localRef) {
    }

    ~ScopedLocalRef() {
        reset();
    }

    void reset(T ptr = nullptr) {
        if (ptr != mLocalRef) {
            if (mLocalRef != nullptr) {
                mEnv->DeleteLocalRef(mLocalRef);
            }
            mLocalRef = ptr;
        }
    }

    T release() {
        T localRef = mLocalRef;
        mLocalRef = nullptr;
        return localRef;
    }

    T get() const {
        return mLocalRef;
    }

 private:
    JNIEnv *const mEnv;
    T mLocalRef;

    DISALLOW_COPY_AND_ASSIGN(ScopedLocalRef);
};
}
#endif  // WHALE_ANDROID_ART_JNI_HELPER_H_
