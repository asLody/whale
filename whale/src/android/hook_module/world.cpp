//
// Created by FKD on 19-6-22.
//

#include "world.h"
#include "JNIHookManager.h"
#include <jni.h>
#include <base/logging.h>
#include <android/art/art_runtime.h>
#include <whale.h>
#include <MSHook/Hooker.h>
#include <android/dex/DexManager.h>
#include <android/art/native_on_load.h>

static JavaVM *current_vm= nullptr;
static jint (*get_created_jvms)(JavaVM**, jsize, jsize*)= nullptr;
JavaVM* GetJavaVM() {
    if (current_vm)
        return current_vm;

    ptr_t libArt = WDynamicLibOpen(kLibArtPath);

    if (!get_created_jvms) {
        get_created_jvms = reinterpret_cast<jint (*)(JavaVM **, jsize, jsize *)>
        (WDynamicLibSymbol(libArt, "JNI_GetCreatedJavaVMs"));
        LOG(INFO) << "get_created_jvms:" << get_created_jvms;
        if (!get_created_jvms)
            return nullptr;
    }

    JavaVM *vms[10] = {nullptr};
    jint vm_count = 0;
    get_created_jvms(vms, 10, &vm_count);
    LOG(INFO) << "created VMs count:" << vm_count;
    if (vm_count <= 0)//There is no JVM created yet.
        return nullptr;
    current_vm = vms[0];//As for android,there will be only one JavaVM in all zygote's child process.
    return current_vm;
}

static jint (*old_create_jvm)(JavaVM**, JNIEnv**, void*)= nullptr;
extern "C" jint my_create_jvm(JavaVM** p_vm,JNIEnv** p_env,void* vm_args){
    if(p_vm== nullptr&&p_env== nullptr&&vm_args== nullptr){
        LOG(INFO)<<"Are you OK?";
        return JNI_OK;
    }
    if(old_create_jvm== nullptr){
        LOG(INFO)<<"Fail to get old_create_jvm.";
        return JNI_OK;
    }
    jint ret=old_create_jvm(p_vm,p_env,vm_args);//After this,we have the java world.
    //Beginning of the java world.
    LOG(INFO)<<"old_create_jvm got return:"<<ret;
    current_vm=*p_vm;
    JNI_OnLoad(current_vm, nullptr);//Manually initialize libwhale.

    //hook_findclass();
    hook_call_app_oncreate();
    //WhaleRuntime access:sys_classloader->app_class_loader
    LOG(INFO)<<"Java World has been owned";

    /*
    JNIEnv *env=*p_env;
    jclass obj_class=env->FindClass("java/lang/Object");
    LOG(INFO)<<"java.lang.Object:"<<obj_class;

    jstring j_opt_path=whale::dex::get_dir(env,"optDex");
    const char* c_opt_path=env->GetStringUTFChars(j_opt_path, nullptr);
    jobject dex_class_loader=whale::dex::new_dex_class_loader(env,DEX_PATH,c_opt_path, nullptr);
    env->ReleaseStringUTFChars(j_opt_path,c_opt_path);
    env->DeleteGlobalRef(j_opt_path);//@see:get_dir's annotation,this is a global reference.
    LOG(INFO)<<"dex_class_loader:"<<dex_class_loader;
     */

    return ret;
}

static bool create_jvm_hooked=false;
void hook_create_jvm() {
    if(create_jvm_hooked||GetJavaVM()) {
        LOG(INFO)<<"create_jvm has been hooked";
        return;
    }
    ptr_t create_jvm_symbol = dlsym(RTLD_DEFAULT, "JNI_CreateJavaVM");
    LOG(INFO)<<"create_jvm_symbol:"<< create_jvm_symbol;
    if (create_jvm_symbol == nullptr)
        return;
    //JNI_CreateJavaVM()
    Cydia::MSHookFunction(create_jvm_symbol,
                          reinterpret_cast<void *>(my_create_jvm),
                          reinterpret_cast<void **>(&old_create_jvm));
    create_jvm_hooked=true;
}

static jlong findclass_slot=0;
jclass my_findclass(JNIEnv *env,jobject thiz,
        jstring class_name,jobject supress_list){
    LOG(INFO)<<"invoke PathList.findClass";
    auto runtime=whale::art::ArtRuntime::Get();
    if(!findclass_slot){
        LOG(INFO)<<"fail to get origin slot of findClass";
        return nullptr;
    }
    if(!class_name)
        return nullptr;
    const char* c_class_name=env->GetStringUTFChars(class_name, nullptr);
    jobject args[]={class_name,supress_list};
    if(strstr(c_class_name,"lody")!= nullptr&&create_jvm_hooked){
        LOG(INFO)<<"loading whale class:"<<c_class_name<<" using sys_classloader";
        //Because when using create_jvm load,WhaleRuntime are in the SystemClassLoader.
        jobject sys_classloader=whale::dex::get_sys_classloader(env);
        jclass result=(jclass)runtime->InvokeOriginalMethod(findclass_slot,sys_classloader,
                runtime->ParseParamArray(env,args,2));
        if(!TryCatch(env)&&result)//Found whale classes in sys_classloader.
            return result;
    }
    jclass origin_class=(jclass)runtime->InvokeOriginalMethod(findclass_slot,thiz,
            runtime->ParseParamArray(env,args,2));
    return origin_class;
}

static bool findclass_hooked=false;
void hook_findclass(){
    if(findclass_hooked)return;
    auto runtime=whale::art::ArtRuntime::Get();
    JNIEnv *env= runtime->GetJniEnv();
    jclass cl_class= env->FindClass("java/lang/ClassLoader");
    jmethodID load_class=env->GetMethodID(cl_class,"loadClass",
            "(Ljava/lang/String;)Ljava/lang/Class;");
    jstring j_crossdex_class_name=env->NewStringUTF("com.lody.whale.CrossDex");
    jclass crossdex_class= (jclass)env->CallObjectMethod(whale::dex::get_sys_classloader(env),load_class,
            j_crossdex_class_name);
    if(TryCatch(env))//ClassNotFound
        return;
    jmethodID hook_method=env->GetStaticMethodID(crossdex_class,"hook","()V");
    env->CallStaticVoidMethod(crossdex_class,hook_method);
    LOG(INFO)<<"JNI confrimed hook findClass finished";
    findclass_hooked=true;
}

static jlong cao_slot=0;
void my_call_app_oncreate(JNIEnv *env,jobject thiz,jobject thisApp){
    const char* pkg_name=env->GetStringUTFChars(
            whale::dex::get_pkg_name(env), nullptr);
    LOG(INFO)<<"invoke hooked callApplicationOnCreate ("<<pkg_name<<")";
    auto runtime=whale::art::ArtRuntime::Get();
    jobject args_array[1]={thisApp};
    jobjectArray params=runtime->ParseParamArray(env,args_array,1);
    //Patch app_classloader to let module use WhaleRuntime freely(avoid ugly reflections).
    WhaleRuntime_handleCallAppOnCreate(env, nullptr/*since we do not use it.*/);
    whale::art::load_hook_module(env,"/data/local/tmp/HMTest.dex");
    LOG(INFO)<<"pass load_hook_module";
    //Allow targetPkgName app to start running its own code.
    runtime->InvokeOriginalMethod(cao_slot,thiz,params);
}

static bool cao_hooked=false;
void hook_call_app_oncreate(){
    if(cao_hooked)return;
    auto runtime=whale::art::ArtRuntime::Get();
    JNIEnv *env= runtime->GetJniEnv();
    jclass in_class=env->FindClass("android/app/Instrumentation");
    //public void callApplicationOnCreate(Application app)
    jmethodID cao_method=env->GetMethodID(in_class,
            "callApplicationOnCreate",
            "(Landroid/app/Application;)V");
    cao_slot=runtime->HookMethodJNI(env, in_class,
            env->ToReflectedMethod(in_class,cao_method,false),
            reinterpret_cast<ptr_t>(my_call_app_oncreate));
    LOG(INFO)<<"cao_slot:"<<cao_slot;
    cao_hooked=true;
}

class static_initializer
{
public:
    static_initializer() {
        hook_create_jvm();
    }
};
static static_initializer s;

