//
// Created by FKD on 19-6-21.
//

#include <jni.h>
#include <zconf.h>
#include <cstdio>
#include <map>
#include <base/logging.h>
#include <android/jni_helper.h>
#include "DexManager.h"

namespace whale::dex{
    /*public static Object combineArray(Object firstArray, Object secondArray) {
        Class<?> localClass = firstArray.getClass().getComponentType();
        int firstArrayLength = Array.getLength(firstArray);
        int allLength = firstArrayLength + Array.getLength(secondArray);
        Object result = Array.newInstance(localClass, allLength);
        for (int k = 0; k < allLength; ++k) {
            if (k < firstArrayLength) {
                Array.set(result, k, Array.get(firstArray, k));
            } else {
                Array.set(result, k, Array.get(secondArray, k - firstArrayLength));
            }
        }
        return result;
    }*/
    jobject combine_array(JNIEnv *env,jobject l_array,jobject r_array){
        jclass array_class=env->FindClass("java/lang/reflect/Array");
        //public static int getLength(Object array)
        jmethodID get_length=env->GetStaticMethodID(array_class,"getLength","(Ljava/lang/Object;)I");
        int first_length=env->CallStaticIntMethod(array_class,get_length,
                l_array);
        int all_length=first_length+ env->CallStaticIntMethod(array_class,get_length,
                r_array);
        jclass obj_class=env->FindClass("java/lang/Object");
        jmethodID get_class=env->GetMethodID(obj_class,"getClass","()Ljava/lang/Class;");
        jclass la_class= (jclass)env->CallObjectMethod(l_array,get_class);
        jmethodID get_component_type=env->GetMethodID(env->GetObjectClass(la_class),
                "getComponentType","()Ljava/lang/Class;");
        jclass decl_class=(jclass)env->CallObjectMethod(la_class,get_component_type);
        //public static Object newInstance(Class<?> componentType, int length)
        jmethodID new_instance=env->GetStaticMethodID(array_class,"newInstance",
                "(Ljava/lang/Class;I)Ljava/lang/Object;");
        /*jobject result=env->CallStaticObjectMethod(decl_class,new_instance,
                decl_class,all_length);*/ //this will cause a unknown JNI type cast error.
        jobject result=env->NewObjectArray(all_length,decl_class, nullptr);
        //public static Object get(Object array, int index)
        jmethodID array_get=env->GetStaticMethodID(array_class,"get",
                "(Ljava/lang/Object;I)Ljava/lang/Object;");
        //public static void set(Object array, int index, Object value)
        jmethodID array_set=env->GetStaticMethodID(array_class,"set",
                                                   "(Ljava/lang/Object;ILjava/lang/Object;)V");
        for (int i = 0; i < all_length; ++i) {
            if (i < first_length) {
                jobject item=env->CallStaticObjectMethod(array_class,array_get,
                        l_array,i);
                env->CallStaticVoidMethod(array_class,array_set,
                        result, i,item);
                env->DeleteLocalRef(item);
            } else {
                //Array.set(result, i, Array.get(secondArray, k - firstArrayLength));
                jobject item=env->CallStaticObjectMethod(array_class,array_get,
                        r_array,i-first_length);
                env->CallStaticVoidMethod(array_class,array_set,
                        result, i,item);
                env->DeleteLocalRef(item);
            }
        }
        return result;
    }

    void patch_class_loader(JNIEnv *env,jobject target,jobject dex_class_loader){
        //Get pathList
        jclass bdcl_class=env->FindClass("dalvik/system/BaseDexClassLoader");
        jfieldID path_list_field=env->GetFieldID(bdcl_class,"pathList",
                "Ldalvik/system/DexPathList;");
        jobject target_path_list=env->GetObjectField(target,path_list_field);
        jobject new_path_list=env->GetObjectField(dex_class_loader,path_list_field);
        //Get dexElements
        jclass path_list_class=env->FindClass("dalvik/system/DexPathList");
        jfieldID dex_elements_field=env->GetFieldID(path_list_class,"dexElements",
                "[Ldalvik/system/DexPathList$Element;");
        jobject target_dex_elements=env->GetObjectField(target_path_list,dex_elements_field);
        jobject new_dex_elements=env->GetObjectField(new_path_list,dex_elements_field);
        //Combine both dexElements(Notice:there is no same object check,
        // so do not call it with the same params twice)
        jobject all_dex_elemnts=combine_array(env,new_dex_elements,target_dex_elements);
        //Patch targetPkgName dexElements with the combined.
        env->SetObjectField(target_path_list,dex_elements_field,all_dex_elemnts);
    }

    static jobject sys_classloader= nullptr;
    jobject get_sys_classloader(JNIEnv *env){
        if(sys_classloader)//Prevent meaningless consuming.
            return sys_classloader;
        jclass cl_class=env->FindClass("java/lang/ClassLoader");
        jmethodID get_sys_classloader= env->GetStaticMethodID(cl_class,
                                                              "getSystemClassLoader",
                                                              "()Ljava/lang/ClassLoader;");
        sys_classloader=env->NewGlobalRef(
                env->CallStaticObjectMethod(cl_class,get_sys_classloader)
                );
        return sys_classloader;
    }

    static std::map<const char*,jobject> loaded_dex_map;
    jobject get_loaded_dex(const char* dex_path){
        auto iter =loaded_dex_map.begin();
        while (iter!=loaded_dex_map.end()){
            const char* item_dex_path=iter->first;
            if(strcmp(item_dex_path,dex_path) == 0){
                return iter->second;//return a DexClassLoader's global reference.
            }
            iter++;
        }
        return nullptr;
    }

    jobject new_dex_class_loader(JNIEnv *env,const char* dex_path, const char* opt_path,const char* lib_path){
        jobject loaded_dex=get_loaded_dex(dex_path);
        if (loaded_dex) {
            LOG(INFO)<<"found a loaded dex:"<<dex_path;
            return loaded_dex;//Probably cause a mistake by incorrect dereference second pointer.
        }
        //Prepare java params
        jstring j_dex_path=env->NewStringUTF(dex_path);
        jstring j_opt_path=env->NewStringUTF(opt_path);
        jstring j_lib_path= nullptr;
        if(lib_path)j_lib_path=env->NewStringUTF(lib_path);
        //Get systemClassLoader
        jobject sys_classloader=get_sys_classloader(env);
        //Get constructor of DexClassLoader
        jclass dex_class_loader_class=env->FindClass("dalvik/system/DexClassLoader");
        jmethodID constructor=env->GetMethodID(dex_class_loader_class,"<init>",
                "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/ClassLoader;)V");
        //Construct a new DexClassLoader with global reference
        jobject result=env->NewGlobalRef(
                env->NewObject(dex_class_loader_class,constructor,
                j_dex_path,j_opt_path,j_lib_path,sys_classloader)
                        );
        loaded_dex_map.insert(std::make_pair(dex_path,result));//Cache loaded dex to prevent repeatedly creation.
        //Recycle local vars
        env->DeleteLocalRef(j_dex_path);
        env->DeleteLocalRef(j_opt_path);
        if(j_lib_path)env->DeleteLocalRef(j_lib_path);
        return result;
    }

    jstring get_dir(JNIEnv *env,const char* name){
        jclass file_class=env->FindClass("java/io/File");
        //public String getAbsolutePath()
        jmethodID get_abs_path=env->GetMethodID(file_class,"getAbsolutePath",
                "()Ljava/lang/String;");
        int uid=getuid();
        bool privilege_uids=(uid==0||uid==1000);
        if(privilege_uids){//Privilege processes
            LOG(INFO)<<"privilege process need to use /data/system";
            jmethodID construct=env->GetMethodID(file_class,"<init>","(Ljava/lang/String;)V");
            //Build full dir path
            char dir_path[1024]={0};
            sprintf(dir_path,"%s/%s\0","/data/system",name);
            jstring j_dir_path=env->NewStringUTF(dir_path);
            jobject dir=env->NewObject(file_class,construct,j_dir_path);
            env->DeleteLocalRef(j_dir_path);
            //Check exists and mkdirs()
            jmethodID exists=env->GetMethodID(file_class,"exists","()Z");
            bool is_exists=env->CallBooleanMethod(dir,exists);
            if(!is_exists){
                jmethodID mkdirs=env->GetMethodID(file_class,"mkdirs","()Z");
                if(!env->CallBooleanMethod(dir,mkdirs)){
                    return nullptr;
                }
            }
            //Notice:(1 TIME) Caller need to use DeleteGlobalRef after path is used.
            return (jstring)env->NewGlobalRef(
                    env->CallObjectMethod(dir,get_abs_path)
                    );
        }
        jobject context=get_app_context(env);
        jclass wrapper_class=env->FindClass("android/content/Context");
        jmethodID get_dir_method=env->GetMethodID(wrapper_class,"getDir",
                "(Ljava/lang/String;I)Ljava/io/File;");
        jobject dir=env->CallObjectMethod(context,get_dir_method,
                env->NewStringUTF(name),0);
        jstring dir_str=(jstring)env->CallObjectMethod(dir,get_abs_path);
        //Notice:(2 TIMES) Caller need to use DeleteGlobalRef after path is used.
        return (jstring)env->NewGlobalRef(dir_str);
    }

    static jobject app_context= nullptr;
    jobject get_app_context(JNIEnv *env){
        if(app_context)
            return app_context;
        //Get application
        jclass at_class=env->FindClass("android/app/ActivityThread");
        jmethodID current_app=env->GetStaticMethodID(at_class,"currentApplication",
                "()Landroid/app/Application;");
        jobject app=env->CallStaticObjectMethod(at_class,current_app);
        if(TryCatch(env)||!app)//process has no application object.
            return nullptr;
        //Get applicationContext
        jclass wrapper_class=env->FindClass("android/content/ContextWrapper");
        jmethodID get_app_context_method=env->GetMethodID(wrapper_class,
                "getApplicationContext",
                "()Landroid/content/Context;");
        app_context=env->NewGlobalRef(
                env->CallObjectMethod(app,get_app_context_method)
        );
        return app_context;
    }

    static jobject app_classloader=nullptr;
    jobject get_app_classloader(JNIEnv *env) {
        if(app_classloader)
            return app_classloader;

        jclass context_class = env->FindClass("android/content/Context");
        jmethodID get_classloader = env->GetMethodID(context_class,
                                                     "getClassLoader",
                                                     "()Ljava/lang/ClassLoader;");
        //env->EnsureLocalCapacity(10);
        jobject context = get_app_context(env);
        if (!context) {
            LOG(INFO) << "use sys_classloader as app_classloader(context is nullptr)";
            //Naturally return a global reference from get_sys_classloader directly.
            return get_sys_classloader(env);
        }else {
            //LOG(INFO)<<"app_context:"<<context;
            app_classloader=env->NewGlobalRef(
                    env->CallObjectMethod(context, get_classloader)
                    );
            return app_classloader;
        }
    }

    jstring get_pkg_name(JNIEnv *env){
        jobject context=get_app_context(env);
        if(!context){
            LOG(INFO)<<"return pkg_name as \"android\" because context is nullptr";
            return env->NewStringUTF("android");
        }
        jclass wrapper_class=env->FindClass("android/content/ContextWrapper");
        jmethodID get_pkg_name=env->GetMethodID(wrapper_class,"getPackageName",
                "()Ljava/lang/String;");
        jstring j_pkg_name=(jstring)env->CallObjectMethod(context,get_pkg_name);
        const char* c_pkg_name=env->GetStringUTFChars(j_pkg_name, nullptr);
        return env->NewStringUTF(c_pkg_name);
    }

    jclass load_by_specific_classloader(JNIEnv *env,jobject class_loader,const char* class_name_wp){
        jclass cl_class=env->FindClass("java/lang/ClassLoader");
        jmethodID load_class=env->GetMethodID(cl_class,
                "loadClass",
                "(Ljava/lang/String;)Ljava/lang/Class;");
        jclass clazz=(jclass)env->CallObjectMethod(class_loader,load_class,
                env->NewStringUTF(class_name_wp)/*must be a java String,or throw a stale reference JNI error.*/);
        if(TryCatch(env))return nullptr;
        return clazz;
    }

    jclass load_by_app_classloader(JNIEnv *env,const char* class_name_wp){
        jclass cl_class=env->FindClass("java/lang/ClassLoader");
        jmethodID load_class=env->GetMethodID(cl_class,
                                              "loadClass",
                                              "(Ljava/lang/String;)Ljava/lang/Class;");
        jobject app_classloader=whale::dex::get_app_classloader(env);
        LOG(INFO)<<"app_classloader:"<<app_classloader;
        jclass clazz=(jclass)env->CallObjectMethod(app_classloader,load_class,
                env->NewStringUTF(class_name_wp)/*must be a java String,or throw a stale reference JNI error.*/);
        if(TryCatch(env))return nullptr;
        return clazz;
    }
}
