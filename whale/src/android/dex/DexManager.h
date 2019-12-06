//
// Created by FKD on 19-6-21.
//

#ifndef WORLD_DEXMANAGER_H
#define WORLD_DEXMANAGER_H

#include <jni.h>
namespace whale::dex {
    jobject combine_array(JNIEnv *env,jobject l_array,jobject r_array);
    void patch_class_loader(JNIEnv *env,jobject target,jobject dex_class_loader);
    jobject get_sys_classloader(JNIEnv *env);
    jobject get_loaded_dex(const char* dex_path);
    jobject new_dex_class_loader(JNIEnv *env,const char* dex_path, const char* opt_path,const char* lib_path);
    jstring get_dir(JNIEnv *env,const char* name);
    jobject get_app_context(JNIEnv *env);
    jobject get_app_classloader(JNIEnv *env);
    jstring get_pkg_name(JNIEnv *env);
    jclass load_by_app_classloader(JNIEnv *env,const char* class_name_wp);
    jclass load_by_specific_classloader(JNIEnv *env,jobject class_loader,const char* class_name_wp);

    class DexManager {
    };
}

#endif //WORLD_DEXMANAGER_H
