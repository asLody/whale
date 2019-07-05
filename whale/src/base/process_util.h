//
// Created by FKD on 19-6-28.
//
#pragma once
#ifndef WORLD_PROCESS_UTIL_H
#define WORLD_PROCESS_UTIL_H

#include <string>
std::string get_name_by_pid(pid_t pid);
void get_name_by_pid(pid_t pid,std::string &process_name);
bool check_dir_rw(const char* path);
#endif //WORLD_PROCESS_UTIL_H
