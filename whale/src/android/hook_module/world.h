//
// Created by FKD on 19-6-22.
//
#pragma once
#ifndef WORLD_WORLD_H
#define WORLD_WORLD_H

#include <jni.h>

JavaVM* GetJavaVM();
void hook_create_jvm();
void hook_findclass();
void hook_call_app_oncreate();
#endif //WORLD_WORLD_H
