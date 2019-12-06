//
// Created by FKD on 19-6-21.
//

#ifndef WORLD_JNIHOOKMANAGER_H
#define WORLD_JNIHOOKMANAGER_H


#include <jni.h>
#include <base/primitive_types.h>
#include <map>


#define J_HOOK_MODULE_ANNO_NAME "com.lody.whale.enity.HookModule"
#define J_HOOK_MODULE_PKG_PREFIX "com.pvdnc"
namespace whale::art {
    void load_hook_module(JNIEnv *env,const char* module_dex_path/*Must can be accessed by all apps.*/);
    class JNIHookManager {
    public:
        bool AddHook(const char *name, jmethodID target, ptr_t replace);
    private:
        std::map<const char *, jlong> hooked_method_map_;
    };
}

#endif //WORLD_JNIHOOKMANAGER_H
