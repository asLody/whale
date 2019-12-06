//
// Created by FKD on 19-6-21.
//

#include "art_helper.h"

namespace whale::art{
    jobject get_system_classloader(JNIEnv *env){
        jclass cl_class=env->FindClass("java/lang/ClassLoader");
    }

    bool set_classloader_parent(jobject target, jobject parent) {

    }
}
