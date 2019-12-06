//
// Created by FKD on 19-6-21.
//

#include <android/art/art_runtime.h>
#include <android/dex/DexManager.h>
#include <base/process_util.h>
#include <base/str_util.h>
#include "JNIHookManager.h"

namespace whale::art {
    jobjectArray get_dexfiles(JNIEnv *env,jobject class_loader){//checked functional
        jclass bdcl_class=env->FindClass("dalvik/system/BaseDexClassLoader");
        jfieldID path_list_field=env->GetFieldID(bdcl_class,"pathList",
                "Ldalvik/system/DexPathList;");
        jobject path_list=env->GetObjectField(class_loader,path_list_field);
        //Get dexElements
        jclass path_list_class=env->FindClass("dalvik/system/DexPathList");
        jfieldID dex_elements_field=env->GetFieldID(path_list_class,"dexElements",
                "[Ldalvik/system/DexPathList$Element;");
        jobjectArray dex_elements=(jobjectArray)env->GetObjectField(path_list,dex_elements_field);
        int elements_size=env->GetArrayLength(dex_elements);
        //Get every dexFile
        jclass element_class=env->FindClass("dalvik/system/DexPathList$Element");
        jfieldID dex_file_field=env->GetFieldID(element_class,"dexFile",
                "Ldalvik/system/DexFile;");
        jclass dex_file_class=env->FindClass("dalvik/system/DexFile");
        jobjectArray result=(jobjectArray)env->NewGlobalRef(
                env->NewObjectArray(elements_size,dex_file_class, nullptr)
                );//Construct a global array to cache these dexFiles.
        for (int i = 0; i < elements_size; ++i) {
            jobject element_item=env->GetObjectArrayElement(dex_elements,i);
            jobject dex_file=env->GetObjectField(element_item,dex_file_field);
            if(dex_file){//Dictionary element's dexFile is null.
                env->SetObjectArrayElement(result,i,dex_file);
            }
            //All temp objects must be recycle at the end of each loop
            // to prevent JNI reference table overflow(256 for default).
            env->DeleteLocalRef(element_item);
        }
        return result;
    }

    using namespace std;
    std::list<std::string> get_all_class_name(JNIEnv *env,jobject dex_file,const char* prefix){
        list<string> result_list;
        if(!dex_file)
            return result_list;
        if(!prefix)
            prefix="";//Let it always true.
        //Get all necessary method that will be used in the loop.
        jclass dex_file_class=env->FindClass("dalvik/system/DexFile");
        jmethodID get_entries=env->GetMethodID(dex_file_class,"entries",
                                               "()Ljava/util/Enumeration;");
        jclass enum_class=env->FindClass("java/util/Enumeration");
        jmethodID has_more_elements=env->GetMethodID(enum_class,"hasMoreElements","()Z");
        jmethodID next_element=env->GetMethodID(enum_class,"nextElement","()Ljava/lang/Object;");
        jobject entries=env->CallObjectMethod(dex_file,get_entries);
        //Enum all classes and cache them.
        while (env->CallBooleanMethod(entries,has_more_elements)){
            jstring class_name=(jstring)env->CallObjectMethod(entries,next_element);
            const char* c_class_name=env->GetStringUTFChars(class_name, nullptr);
            if(!Contains(c_class_name,"android")&&Contains(c_class_name,prefix)) {
                result_list.emplace_back(c_class_name);//Cache into result_list.
            }
            env->ReleaseStringUTFChars(class_name,c_class_name);
            env->DeleteLocalRef(class_name);
        }
        return result_list;
    }

    jmethodID get_method_by_name(JNIEnv *env,jclass decl_class,const char* name){
        jclass c_class=env->FindClass("java/lang/Class");
        //public Method[] getDeclaredMethods()
        jmethodID get_all_methods=env->GetMethodID(c_class,"getDeclaredMethods",
                                                   "()[Ljava/lang/reflect/Method;");
        jobjectArray methods=(jobjectArray)env->CallObjectMethod(decl_class,get_all_methods);
        jclass method_class=env->FindClass("java/lang/reflect/Method");
        //public String getName()
        jmethodID get_method_name=env->GetMethodID(method_class,"getName","()Ljava/lang/String;");
        int method_count=env->GetArrayLength(methods);
        for (int i = 0; i < method_count; ++i) {
            jobject method_item=env->GetObjectArrayElement(methods,i);
            jstring j_method_name=(jstring)env->CallObjectMethod(method_item,get_method_name);
            const char* c_method_name=env->GetStringUTFChars(j_method_name, nullptr);
            if(Contains(c_method_name,name)){//Found the method is searching for.
                //LOGD("found method %s:%p\n",c_method_name,method_item);
                return env->FromReflectedMethod(method_item);
            }
            //Release local references in loop.
            env->ReleaseStringUTFChars(j_method_name,c_method_name);
            env->DeleteLocalRef(j_method_name);
            env->DeleteLocalRef(method_item);
        }
        LOG(INFO)<<"method "<<name<<" not found";
        return nullptr;
    }

    bool is_module_class(JNIEnv *env,jclass clazz){
        if(!clazz)
            return false;
        jclass c_class=env->FindClass("java/lang/Class");
        //TODO:Finish the remaining parts that load hook modules in every app.
        //Get @HookModule annotation
        jclass hm_anno_class=whale::dex::load_by_app_classloader(env,
                J_HOOK_MODULE_ANNO_NAME);
        if(TryCatch(env)||!hm_anno_class){
            LOG(INFO)<<"hm_anno_class is not found";
            return false;
        }
        //public boolean isAnnotationPresent(Class<? extends Annotation> annotationClass)
        jmethodID is_anno_present=env->GetMethodID(c_class,"isAnnotationPresent",
                "(Ljava/lang/Class;)Z");
        bool hm_present=env->CallBooleanMethod(clazz,is_anno_present,hm_anno_class);
        if(!hm_present)
            return false;

        //Get instance of @HookModule.
        //public <A extends Annotation> A getAnnotation(Class<A> annotationClass)
        jmethodID get_anno=get_method_by_name(env,c_class,"getAnnotation");
        if(!get_anno)
            return false;//Log will be output in get_method_by_name.
        LOG(INFO)<<"before getAnnotation";
        jobject anno=env->CallObjectMethod(clazz,get_anno,
                hm_anno_class);
        LOG(INFO)<<"anno instance:"<<anno;

        //Get targetPkgName to decide whether module should be executed in current process.
        jstring target_pkg=nullptr;
        if(TryCatch(env)) {//AssertionError,try to use old ways to get targetPkgName.
            jmethodID get_target_pkg=env->GetStaticMethodID(clazz,"targetPkgName","()Ljava/lang/String;");
            if(TryCatch(env)) {//NoSuchMethod
                LOG(INFO)<<"used both way to get targetPkgName but failed,please watch sample module for help";
                return false;
            }
            target_pkg=(jstring)env->CallStaticObjectMethod(clazz,get_target_pkg);
        }else{//Annotation can be used.
            jmethodID get_target_pkg=get_method_by_name(env,hm_anno_class,"targetPkgName");
            if(!get_target_pkg)
                return false;
            target_pkg=(jstring)env->CallObjectMethod(anno,get_target_pkg);
        }
        if(!target_pkg)
            return false;
        const char* c_target_pkg=env->GetStringUTFChars(target_pkg, nullptr);
        LOG(INFO)<<"target_pkg_name:"<<c_target_pkg;
        string proc_name;
        get_name_by_pid(getpid(),proc_name);
        const char* this_process_name=proc_name.c_str();
        LOG(INFO)<<"this_process_name:"<<this_process_name;
        if(Contains(this_process_name,c_target_pkg)){
            LOG(INFO)<<this_process_name<<"-"<<c_target_pkg<<" matched";
            return true;
        }
        return false;
    }

    void execute_hook(JNIEnv *env,
            jclass module_class/*Attention:module_class is just a possibility of its mean*/){
        if(!module_class)
            return;
        if(!is_module_class(env,module_class))
            return;
        LOG(INFO)<<"pass is_module_class";
        jmethodID hook_method=env->GetStaticMethodID(module_class,"hook","()V");
        if(!TryCatch(env)){//NoSuchMethod
            if(!hook_method){
                LOG(INFO)<<"call hook_method in "<<module_class<<" has no error but is a nullptr";
                return;
            }
            env->CallStaticVoidMethod(module_class,hook_method);
            TryCatch(env);//Whether the hook method returns successfully,the procedure need to be continue.
            LOG(INFO)<<"hook_method invoked in pid:"<<getpid();
        }
    }

    void load_hook_module(JNIEnv *env,const char* module_dex_path/*Must can be accessed by all apps.*/){
        jstring opt_path=whale::dex::get_dir(env,"optDex");
        const char* c_opt_path=env->GetStringUTFChars(opt_path, nullptr);
        if(!check_dir_rw(c_opt_path)) {
            LOG(INFO)<<c_opt_path<<"is not rw for process:"<<getpid()<<",skip loading modules";
            return;
        }
        jobject dex_class_loader=whale::dex::new_dex_class_loader(env,module_dex_path,
                c_opt_path,module_dex_path);
        //Get that first because app_classloader will be merged(a lot more time consuming).
        jobjectArray dex_files=get_dexfiles(env,dex_class_loader);
        int dex_files_len=env->GetArrayLength(dex_files);
        //Patch app_classloader to dex_classloader(let modules access app's "special" classes).
        whale::dex::patch_class_loader(env,
                                       dex_class_loader,
                                       whale::dex::get_app_classloader(env));
        //Get all dexFiles in dex_class_loader.
        for (int i_df = 0; i_df < dex_files_len; ++i_df) {
            jobject dex_file_item=env->GetObjectArrayElement(dex_files,i_df);
            //Load each classes possible and execute_hook().
            list<string> class_name_list=get_all_class_name(env,dex_file_item,
                    J_HOOK_MODULE_PKG_PREFIX);
            auto itor=class_name_list.begin();
            for (itor=class_name_list.begin();itor!=class_name_list.end();itor++) {
                const char* c_class_name=itor->c_str();
                jclass possible_class= whale::dex::load_by_specific_classloader(env,dex_class_loader,
                        c_class_name);
                LOG(INFO)<<"possible_module_class:"<<c_class_name;
                execute_hook(env,possible_class);
                env->DeleteLocalRef(possible_class);//Prevent crash when l-r-table is overflowed.
            }
            //Release local references in loop.
            env->DeleteLocalRef(dex_file_item);
        }
    }

    bool JNIHookManager::AddHook(const char *name, jmethodID target, ptr_t replace) {
        auto runtime=ArtRuntime::Get();

        return false;
    }
}