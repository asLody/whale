#include "android/jni_helper.h"
#include "android/android_build.h"
#include "android/scoped_thread_state_change.h"
#include "android/art/art_runtime.h"
#include "android/dvm/dvm_runtime.h"


namespace whale {

static volatile long kNoGCDaemonsGuard = 0;

jclass ScopedNoGCDaemons::java_lang_Daemons;
jmethodID ScopedNoGCDaemons::java_lang_Daemons_start;
jmethodID ScopedNoGCDaemons::java_lang_Daemons_stop;

void ScopedNoGCDaemons::Load(JNIEnv *env) {
    java_lang_Daemons = reinterpret_cast<jclass>(env->NewGlobalRef(
            env->FindClass("java/lang/Daemons")));
    if (java_lang_Daemons == nullptr) {
        JNIExceptionClear(env);
        LOG(WARNING) << "java/lang/Daemons API is unavailable.";
        return;
    }
    java_lang_Daemons_start = env->GetStaticMethodID(java_lang_Daemons, "start", "()V");
    if (java_lang_Daemons_start == nullptr) {
        JNIExceptionClear(env);
        java_lang_Daemons_start = env->GetStaticMethodID(java_lang_Daemons, "startPostZygoteFork",
                                                         "()V");
    }
    if (java_lang_Daemons_start == nullptr) {
        LOG(WARNING)
                << "java/lang/Daemons API is available but no start/startPostZygoteFork method.";
        JNIExceptionClear(env);
    }
    java_lang_Daemons_stop = env->GetStaticMethodID(java_lang_Daemons, "stop", "()V");
    JNIExceptionClear(env);
    LOG(ERROR) << "[ScopedNoGCDaemons::Load] ScopedNoGCDaemons::Load over";
}

ScopedNoGCDaemons::ScopedNoGCDaemons(JNIEnv *env) : env_(env) {
    if (java_lang_Daemons_start != nullptr) {
        if (__sync_sub_and_fetch(&kNoGCDaemonsGuard, 1) <= 0) {
            env_->CallStaticVoidMethod(java_lang_Daemons, java_lang_Daemons_stop);
            JNIExceptionClear(env_);
        }
    }
}


ScopedNoGCDaemons::~ScopedNoGCDaemons() {
    if (java_lang_Daemons_stop != nullptr) {
        if (__sync_add_and_fetch(&kNoGCDaemonsGuard, 1) == 1) {
            env_->CallStaticVoidMethod(java_lang_Daemons, java_lang_Daemons_start);
            JNIExceptionClear(env_);
        }
    }
}

ScopedSuspendAll::ScopedSuspendAll() {
    if(g_isArt){
        LOG(ERROR) << "[ScopedSuspendAll::ScopedSuspendAll] isArt";
        whale::art::ArtResolvedSymbols *symbols = whale::art::ArtRuntime::Get()->GetSymbols();
		if (symbols->Dbg_SuspendVM && symbols->Dbg_ResumeVM) {
            symbols->Dbg_SuspendVM();
        } else {
        	LOG(WARNING) << "Suspend art VM API is unavailable.";
    	}
    }else{
        LOG(ERROR) << "[ScopedSuspendAll::ScopedSuspendAll] isDvm";
        whale::dvm::DvmResolvedSymbols *symbols = whale::dvm::DvmRuntime::Get()->GetSymbols();
		if (symbols->Dbg_SuspendVM && symbols->Dbg_ResumeVM) {
            symbols->Dbg_SuspendVM(false);
	    } else {
	        LOG(WARNING) << "Suspend dvm VM API is unavailable.";
	    }
    }
    
}

ScopedSuspendAll::~ScopedSuspendAll() {
    if(g_isArt){
    LOG(ERROR) << "[ScopedSuspendAll::~ScopedSuspendAll] m_isArt";
        whale::art::ArtResolvedSymbols *art_symbols = whale::art::ArtRuntime::Get()->GetSymbols();
        if (art_symbols->Dbg_SuspendVM && art_symbols->Dbg_ResumeVM) {
                art_symbols->Dbg_ResumeVM();
            }
    }else{
        LOG(ERROR) << "[ScopedSuspendAll::~ScopedSuspendAll] m_isDvm";
        whale::dvm::DvmResolvedSymbols *dvm_symbols = whale::dvm::DvmRuntime::Get()->GetSymbols();
        if (dvm_symbols->Dbg_SuspendVM && dvm_symbols->Dbg_ResumeVM) {
                dvm_symbols->Dbg_ResumeVM();
            }
    }
}
}  // namespace whale

