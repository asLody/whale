#include <dlfcn.h>
#include "whale.h"
#include "android/android_build.h"
#include "android/dvm/dvm_runtime.h"
#include "android/modifiers.h"
#include "android/native_on_load.h"
#include "android/dvm/dvm_method.h"
#include "android/dvm/dvm_symbol_resolver.h"
#include "android/scoped_thread_state_change.h"
#include "android/dvm/dvm_jni_trampoline.h"
#include "android/well_known_classes.h"
#include "android/java_types.h"
#include "platform/memory.h"
#include "base/logging.h"
#include "base/singleton.h"
#include "base/cxx_helper.h"

namespace whale {
namespace dvm {

    DvmRuntime *DvmRuntime::Get() {
        static DvmRuntime instance;
        return &instance;
    }





void PreLoadRequiredStuff(JNIEnv *env) {
    Types::Load(env);
    WellKnownClasses::Load(env);
    ScopedNoGCDaemons::Load(env);
}


u4 dvmPlatformInvokeHints(const char *shorty)
{
    const char* sig = shorty;
    int padFlags, jniHints;
    char sigByte;
    int stackOffset, padMask;

    stackOffset = padFlags = 0;
    padMask = 0x00000001;

    /* Skip past the return type */
    sig++;

    while (true) {
        sigByte = *(sig++);

        if (sigByte == '\0')
            break;

        if (sigByte == 'D' || sigByte == 'J') {
            if ((stackOffset & 1) != 0) {
                padFlags |= padMask;
                stackOffset++;
                padMask <<= 1;
            }
            stackOffset += 2;
            padMask <<= 2;
        } else {
            stackOffset++;
            padMask <<= 1;
        }
    }

    jniHints = 0;

    if (stackOffset > DALVIK_JNI_COUNT_SHIFT) {
        /* too big for "fast" version */
        jniHints = DALVIK_JNI_NO_ARG_INFO;
    } else {
        assert((padFlags & (0xffffffff << DALVIK_JNI_COUNT_SHIFT)) == 0);
        stackOffset -= 2;           // r2/r3 holds first two items
        if (stackOffset < 0)
            stackOffset = 0;
        jniHints |= ((stackOffset+1) / 2) << DALVIK_JNI_COUNT_SHIFT;
        jniHints |= padFlags;
    }

    return jniHints;
}


static int computeJniArgInfo(const char* shorty)
{
    int returnType, jniArgInfo;
    u4 hints;

    /* The first shorty character is the return type. */
    switch (*(shorty)) {
    case 'V':
        returnType = DALVIK_JNI_RETURN_VOID;
        break;
    case 'F':
        returnType = DALVIK_JNI_RETURN_FLOAT;
        break;
    case 'D':
        returnType = DALVIK_JNI_RETURN_DOUBLE;
        break;
    case 'J':
        returnType = DALVIK_JNI_RETURN_S8;
        break;
    case 'Z':
    case 'B':
        returnType = DALVIK_JNI_RETURN_S1;
        break;
    case 'C':
        returnType = DALVIK_JNI_RETURN_U2;
        break;
    case 'S':
        returnType = DALVIK_JNI_RETURN_S2;
        break;
    default:
        returnType = DALVIK_JNI_RETURN_S4;
        break;
    }

    jniArgInfo = returnType << DALVIK_JNI_RETURN_SHIFT;

    hints = dvmPlatformInvokeHints(shorty);

    if (hints & DALVIK_JNI_NO_ARG_INFO) {
        jniArgInfo |= DALVIK_JNI_NO_ARG_INFO;
        jniArgInfo |= hints;
    }

    return jniArgInfo;
}


bool DvmRuntime::OnLoad(JavaVM *vm, JNIEnv *env, jclass java_class) {
#define CHECK_FIELD(field, value)  \
    if ((field) == (value)) {  \
        LOG(ERROR) << "Failed to find " #field ".";  \
        return false;  \
    }

    if ((kRuntimeISA == InstructionSet::kArm
         || kRuntimeISA == InstructionSet::kArm64)
        && IsFileInMemory("libhoudini.so")) {
        LOG(INFO) << '[' << getpid() << ']' << " Unable to launch on houdini environment.";
        return false;
    }
    vm_ = vm;
    java_class_ = reinterpret_cast<jclass>(env->NewGlobalRef(java_class));
    bridge_method_ = env->GetStaticMethodID(
            java_class,
            "handleHookedMethod",
            "(Ljava/lang/reflect/Member;JLjava/lang/Object;Ljava/lang/Object;[Ljava/lang/Object;)Ljava/lang/Object;"
    );
	
    if (JNIExceptionCheck(env)) {
        return false;
    }
    api_level_ = GetAndroidApiLevel();
    PreLoadRequiredStuff(env);
    const char *dvm_path = kLibDvmPath;

    dvm_elf_image_ = WDynamicLibOpen(dvm_path);
    if (dvm_elf_image_ == nullptr) {
        LOG(ERROR) << "Unable to read data from libdvm.so.";
        return false;
    }
    if (!dvm_symbol_resolver_.Resolve(dvm_elf_image_, api_level_)) {
        // The log will all output from DvmSymbolResolver.
        return false;
    }

    jmethodID reserved0 = env->GetStaticMethodID(java_class, kMethodReserved0, "()V");
	Method* reserved0_ = (Method *)reserved0;
	jniBridge_ = reserved0_->nativeFunc;


    pthread_mutex_init(&mutex, nullptr);
    EnforceDisableHiddenAPIPolicy();
    return true;
#undef CHECK_OFFSET
}


/*
*decl_class 被hook函数所在类
*hooked_java_method 被hook函数
*/
jlong
DvmRuntime::HookMethod(JNIEnv *env, jclass decl_class, jobject hooked_java_method,
                       jobject addition_info) {
    ScopedSuspendAll suspend_all;
    jmethodID hooked_jni_method = env->FromReflectedMethod(hooked_java_method);//从java的反射函数映射成JNI的jmethodID结构
	DvmMethod hooked_method(hooked_jni_method);
	Method* hooked_jni_ = (Method *)hooked_jni_method;

    auto *param = new DvmHookParam();
    param->shorty_ = hooked_jni_->shorty;
    param->is_static_ = ((hooked_jni_->accessFlags & kAccStatic) != 0);
    param->origin_access_flags = hooked_jni_->accessFlags;
    jmethodID origin_java_method = hooked_method.Clone(env, param->origin_access_flags);//复制原始函数的对象

    u4 access_flags = hooked_jni_->accessFlags;
    access_flags |= kAccNative;
    hooked_jni_->accessFlags = access_flags;
	hooked_jni_->jniArgInfo = computeJniArgInfo(param->shorty_);//设置这个属性以后，返回值才是正确的

    param->origin_native_method_ = origin_java_method;//被hook函数的克隆对象生成的jmethodID
    param->hooked_native_method_ = hooked_jni_method;//被hook函数的原始引用生成的jmethodID
    param->addition_info_ = env->NewGlobalRef(addition_info);
    param->hooked_method_ = env->NewGlobalRef(hooked_java_method);//被hook函数原始引用

	hex_dump(hooked_jni_, sizeof(Method));
	printMethodInfo((Method *)hooked_method.getJmethod());

    BuildJniClosure(param);

	hooked_jni_->insns = (u2 *)param->jni_closure_->GetCode();//dvm环境下，需要将nativeFunc地址赋值给dexCodeItem
	hooked_jni_->nativeFunc = jniBridge_;//jni函数的入口是一个固定值，从JNI_Onload获取初始值，稍后再赋值
	hooked_jni_->registersSize = hooked_jni_->insSize;
	hooked_jni_->outsSize = 0x0;

    /*
    * JNI: true if this method has no reference arguments. This lets the JNI
    * bridge avoid scanning the shorty for direct pointers that need to be
    * converted to local references.
    *
    * TODO: replace this with a list of indexes of the reference arguments.
    */
    hooked_jni_->noRef = true;
    const char* cp = hooked_jni_->shorty;
    while (*++cp != '\0') { // Pre-increment to skip return type.
        if (*cp == 'L') {
            hooked_jni_->noRef = false;
            break;
        }
    }

    param->decl_class_ = hooked_method.GetDeclaringClass();
    hooked_method_map_.insert(std::make_pair(hooked_jni_method, param));
	hex_dump(hooked_jni_, sizeof(Method));
	printMethodInfo((Method *)hooked_jni_);
    return reinterpret_cast<jlong>(param);
}

jobject
DvmRuntime::InvokeOriginalMethod(jlong slot, jobject this_object, jobjectArray args) {
	JNIEnv *env = GetJniEnv();
    auto *param = reinterpret_cast<DvmHookParam *>(slot);
    if (slot <= 0) {
        env->ThrowNew(
                WellKnownClasses::java_lang_IllegalArgumentException,
                "Failed to resolve slot."
        );
        return nullptr;
    }

    void * tmp = malloc(sizeof(Method));
    memcpy(tmp, param->hooked_native_method_, sizeof(Method));
    memcpy(param->hooked_native_method_, param->origin_native_method_, sizeof(Method));
    printMethodInfo((Method *)param->hooked_native_method_);
    jobject ret = env->CallNonvirtualObjectMethod(
            param->hooked_method_,
            WellKnownClasses::java_lang_reflect_Method,
            WellKnownClasses::java_lang_reflect_Method_invoke,
            this_object,
            args
    );
    memcpy(param->hooked_native_method_, tmp, sizeof(Method));
    printMethodInfo((Method *)param->hooked_native_method_);
    return ret;
}

#if defined(__aarch64__)
# define __get_tls() ({ void** __val; __asm__("mrs %0, tpidr_el0" : "=r"(__val)); __val; })
#elif defined(__arm__)
# define __get_tls() ({ void** __val; __asm__("mrc p15, 0, %0, c13, c0, 3" : "=r"(__val)); __val; })
#elif defined(__i386__)
# define __get_tls() ({ void** __val; __asm__("movl %%gs:0, %0" : "=r"(__val)); __val; })
#elif defined(__x86_64__)
# define __get_tls() ({ void** __val; __asm__("mov %%fs:0, %0" : "=r"(__val)); __val; })
#else
#error unsupported architecture
#endif

DvmThread *DvmRuntime::GetCurrentDvmThread() {
    if (WellKnownClasses::java_lang_Thread_nativePeer) {
        JNIEnv *env = GetJniEnv();
        jobject current = env->CallStaticObjectMethod(
                WellKnownClasses::java_lang_Thread,
                WellKnownClasses::java_lang_Thread_currentThread
        );
        return reinterpret_cast<DvmThread *>(
                env->GetLongField(current, WellKnownClasses::java_lang_Thread_nativePeer)
        );
    }
    return reinterpret_cast<DvmThread *>(__get_tls()[7/*TLS_SLOT_ART_THREAD_SELF*/]);
}

jobject
DvmRuntime::InvokeHookedMethodBridge(JNIEnv *env, DvmHookParam *param, jobject receiver,
                                     jobjectArray array) {
	return env->CallStaticObjectMethod(java_class_, bridge_method_,
                                       param->hooked_method_, reinterpret_cast<jlong>(param),
                                       param->addition_info_, receiver, array);
}

jlong DvmRuntime::GetMethodSlot(JNIEnv *env, jclass cl, jobject method_obj) {
    if (method_obj == nullptr) {
        env->ThrowNew(
                WellKnownClasses::java_lang_IllegalArgumentException,
                "Method param == null"
        );
        return 0;
    }
    jmethodID jni_method = env->FromReflectedMethod(method_obj);
    auto entry = hooked_method_map_.find(jni_method);
    if (entry == hooked_method_map_.end()) {
        env->ThrowNew(
                WellKnownClasses::java_lang_IllegalArgumentException,
                "Failed to find slot."
        );
        return 0;
    }
    return reinterpret_cast<jlong>(entry->second);
}

void DvmRuntime::EnsureClassInitialized(JNIEnv *env, jclass cl) {
    // This invocation will ensure the target class has been initialized also.
    ScopedLocalRef<jobject> unused(env, env->AllocObject(cl));
    JNIExceptionClear(env);
}

void DvmRuntime::SetObjectClass(JNIEnv *env, jobject obj, jclass cl) {
    SetObjectClassUnsafe(env, obj, cl);
}

void DvmRuntime::SetObjectClassUnsafe(JNIEnv *env, jobject obj, jclass cl) {
    jfieldID java_lang_Class_shadow$_klass_ = env->GetFieldID(
            WellKnownClasses::java_lang_Object,
            "shadow$_klass_",
            "Ljava/lang/Class;"
    );
    env->SetObjectField(obj, java_lang_Class_shadow$_klass_, cl);
}

jobject DvmRuntime::CloneToSubclass(JNIEnv *env, jobject obj, jclass sub_class) {
    DvmResolvedSymbols *symbols = GetSymbols();
    DvmThread *thread = GetCurrentDvmThread();
    return nullptr;
}

void DvmRuntime::RemoveFinalFlag(JNIEnv *env, jclass java_class) {
    jfieldID java_lang_Class_accessFlags = env->GetFieldID(
            WellKnownClasses::java_lang_Class,
            "accessFlags",
            "I"
    );
    jint access_flags = env->GetIntField(java_class, java_lang_Class_accessFlags);
    env->SetIntField(java_class, java_lang_Class_accessFlags, access_flags & ~kAccFinal);
}

bool DvmRuntime::EnforceDisableHiddenAPIPolicy() {
    if (GetAndroidApiLevel() < ANDROID_O_MR1) {
        return true;
    }
    static Singleton<bool> enforced([&](bool *result) {
        *result = EnforceDisableHiddenAPIPolicyImpl();
    });
    return enforced.Get();
}

bool OnInvokeHiddenAPI() {
    return false;
}

/**
 * NOTICE:
 * After Android Q(10.0), GetMemberActionImpl has been renamed to ShouldDenyAccessToMemberImpl,
 * But we don't know the symbols until it's published.
 */
ALWAYS_INLINE bool DvmRuntime::EnforceDisableHiddenAPIPolicyImpl() {
    JNIEnv *env = GetJniEnv();
    jfieldID java_lang_Class_shadow$_klass_ = env->GetFieldID(
            WellKnownClasses::java_lang_Object,
            "shadow$_klass_",
            "Ljava/lang/Class;"
    );
    JNIExceptionClear(env);
    if (java_lang_Class_shadow$_klass_ != nullptr) {
        return true;
    }
    void *symbol = nullptr;

    // Android P : Preview 1 ~ 4 version
    symbol = WDynamicLibSymbol(
            dvm_elf_image_,
            "_ZN3dvm9hiddenapi25ShouldBlockAccessToMemberINS_8DvmFieldEEEbPT_PNS_6ThreadENSt3__18functionIFbS6_EEENS0_12AccessMethodE"
    );
    if (symbol) {
        WInlineHookFunction(symbol, reinterpret_cast<void *>(OnInvokeHiddenAPI), nullptr);
    }
    symbol = WDynamicLibSymbol(
            dvm_elf_image_,
            "_ZN3dvm9hiddenapi25ShouldBlockAccessToMemberINS_9DvmMethodEEEbPT_PNS_6ThreadENSt3__18functionIFbS6_EEENS0_12AccessMethodE"
    );

    if (symbol) {
        WInlineHookFunction(symbol, reinterpret_cast<void *>(OnInvokeHiddenAPI), nullptr);
        return true;
    }
    // Android P : Release version
    symbol = WDynamicLibSymbol(
            dvm_elf_image_,
            "_ZN3dvm9hiddenapi6detail19GetMemberActionImplINS_8DvmFieldEEENS0_6ActionEPT_NS_20HiddenApiAccessFlags7ApiListES4_NS0_12AccessMethodE"
    );
    if (symbol) {
        WInlineHookFunction(symbol, reinterpret_cast<void *>(OnInvokeHiddenAPI), nullptr);
    }
    symbol = WDynamicLibSymbol(
            dvm_elf_image_,
            "_ZN3dvm9hiddenapi6detail19GetMemberActionImplINS_9DvmMethodEEENS0_6ActionEPT_NS_20HiddenApiAccessFlags7ApiListES4_NS0_12AccessMethodE"
    );
    if (symbol) {
        WInlineHookFunction(symbol, reinterpret_cast<void *>(OnInvokeHiddenAPI), nullptr);
    }
    return symbol != nullptr;
}


int (*old_ToDexPc)(void *thiz, void *a2, unsigned int a3, int a4);
int new_ToDexPc(void *thiz, void *a2, unsigned int a3, int a4) {
    return old_ToDexPc(thiz, a2, a3, 0);
}

bool is_hooked = false;
void DvmRuntime::FixBugN() {
    if (is_hooked)
        return;
    void *symbol = nullptr;
    symbol = WDynamicLibSymbol(
            dvm_elf_image_,
            "_ZNK3dvm20OatQuickMethodHeader7ToDexPcEPNS_9DvmMethodEjb"
    );
    if (symbol) {
        WInlineHookFunction(symbol, reinterpret_cast<void *>(new_ToDexPc), reinterpret_cast<void **>(&old_ToDexPc));
    }
    is_hooked = true;
}

}  // namespace dvm
}  // namespace whale
