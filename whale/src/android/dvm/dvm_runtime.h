#ifndef WHALE_ANDROID_DVM_INTERCEPTOR_H_
#define WHALE_ANDROID_DVM_INTERCEPTOR_H_

#include <jni.h>
#include <map>
#include "platform/linux/elf_image.h"
#include "platform/linux/process_map.h"
#include "dbi/instruction_set.h"
#include "android/dvm/dvm_symbol_resolver.h"
#include "android/dvm/dvm_hook_param.h"
#include "android/native_bridge.h"
#include "android/jni_helper.h"
#include "base/macros.h"
#include "base/primitive_types.h"

namespace whale {
namespace dvm {

enum DalvikJniReturnType {
    DALVIK_JNI_RETURN_VOID = 0,     /* must be zero */
    DALVIK_JNI_RETURN_FLOAT = 1,
    DALVIK_JNI_RETURN_DOUBLE = 2,
    DALVIK_JNI_RETURN_S8 = 3,
    DALVIK_JNI_RETURN_S4 = 4,
    DALVIK_JNI_RETURN_S2 = 5,
    DALVIK_JNI_RETURN_U2 = 6,
    DALVIK_JNI_RETURN_S1 = 7
};

#define DALVIK_JNI_NO_ARG_INFO  0x80000000
#define DALVIK_JNI_RETURN_MASK  0x70000000
#define DALVIK_JNI_RETURN_SHIFT 28
#define DALVIK_JNI_COUNT_MASK   0x0f000000
#define DALVIK_JNI_COUNT_SHIFT  24



class DvmThread;

struct Method {
    void*    clazz;
    u4              accessFlags;
    u2             methodIndex;
    u2              registersSize;  /* ins + locals */
    u2              outsSize;
    u2              insSize;
    const char*     name;
    u8       prototype;
    const char*     shorty; 
    const u2*       insns;
    int             jniArgInfo;
    void* nativeFunc;
    bool fastJni;
    bool noRef;
    bool shouldTrace;
    const void* registerMap;
    bool            inProfile;
} __attribute__((packed));


struct DvmMethodOffsets final {
    size_t method_size_;
    offset_t jni_code_offset_;//jni入口函数的偏移地址
    offset_t access_flags_offset_;//访问符号标志位
    offset_t shorty_offset_;//函数简短描述符偏移地址
    offset_t dex_code_item_offset_offset_;//bytecode偏移地址 dvm虚拟机，为什么将jni的入口设置在这里
    offset_t jniArgInfo_offset_;//jniArgInfo的偏移地址
    offset_t noRef_offset_;
    offset_t dex_method_index_offset_;
    offset_t method_index_offset_;
};

struct RuntimeObjects final {
    ptr_t OPTION runtime_;
    ptr_t OPTION heap_;
    ptr_t OPTION thread_list_;
    ptr_t OPTION class_linker_;
    ptr_t OPTION intern_table_;
};

struct ClassLinkerObjects {
    ptr_t quick_generic_jni_trampoline_;
};

class DvmRuntime final {
 public:
    friend class DvmMethod;

	bool isArt = false;
	
    static DvmRuntime *Get();

    DvmRuntime() {}

    bool OnLoad(JavaVM *vm, JNIEnv *env, jclass java_class);

    jlong HookMethod(JNIEnv *env, jclass decl_class, jobject hooked_java_method,
                     jobject addition_info);

    JNIEnv *GetJniEnv() {
        JNIEnv *env = nullptr;
        jint ret = vm_->AttachCurrentThread(&env, nullptr);
        DCHECK_EQ(JNI_OK, ret);
        return env;
    }

    DvmMethodOffsets *GetDvmMethodOffsets() {
        return &method_offset_;
    }


    RuntimeObjects *GetRuntimeObjects() {
        return &runtime_objects_;
    }

    DvmResolvedSymbols *GetSymbols() {
        return dvm_symbol_resolver_.GetSymbols();
    }

    DvmThread *GetCurrentDvmThread();

    void EnsureClassInitialized(JNIEnv *env, jclass cl);


    jobject
    InvokeHookedMethodBridge(JNIEnv *env, DvmHookParam *param, jobject receiver,
                             jobjectArray array);

    jobject
    InvokeOriginalMethod(jlong slot, jobject this_object, jobjectArray args);

    jlong GetMethodSlot(JNIEnv *env, jclass cl, jobject method_obj);

    ALWAYS_INLINE void VisitInterceptParams(std::function<void(DvmHookParam *)> visitor) {
        for (auto &entry : hooked_method_map_) {
            visitor(entry.second);
        }
    }

    void SetObjectClass(JNIEnv *env, jobject obj, jclass cl);

    void SetObjectClassUnsafe(JNIEnv *env, jobject obj, jclass cl);

    jobject CloneToSubclass(JNIEnv *env, jobject obj, jclass sub_class);

    void RemoveFinalFlag(JNIEnv *env, jclass java_class);

    bool EnforceDisableHiddenAPIPolicy();

    ptr_t CloneDvmObject(ptr_t art_object);

    void FixBugN();

 private:
 	void *jniBridge_;
    JavaVM *vm_;
    jclass java_class_;
    jmethodID bridge_method_;
    s4 api_level_;
    void *dvm_elf_image_;
    NativeBridgeCallbacks OPTION *android_bridge_callbacks_;
    DvmSymbolResolver dvm_symbol_resolver_;
    RuntimeObjects runtime_objects_;
    DvmMethodOffsets method_offset_;
    std::map<jmethodID, DvmHookParam *> hooked_method_map_;
    pthread_mutex_t mutex;

    bool EnforceDisableHiddenAPIPolicyImpl();

    DISALLOW_COPY_AND_ASSIGN(DvmRuntime);
};


}  // namespace dvm
}  // namespace whale

#endif  // WHALE_ANDROID_DVM_INTERCEPTOR_H_
