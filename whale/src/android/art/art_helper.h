//
// Created by FKD on 19-6-21.
//

#ifndef WORLD_ART_HELPER_H
#define WORLD_ART_HELPER_H

#include "art_runtime.h"
#include <jni.h>
namespace whale::art{
    bool set_classloader_parent(jobject target,jobject parent);
}
#endif //WORLD_ART_HELPER_H
