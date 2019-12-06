//
// Created by FKD on 19-6-28.
//

#include <zconf.h>
#include "process_util.h"
#include "../base/logging.h"
using namespace std;
#define BUF_SIZE 1024
#define FAIL_PROCESS_NAME ""

string get_name_by_pid(pid_t pid) {
    char buf[BUF_SIZE];
    char cmd[100] = {'\0'};
    sprintf(cmd, "cat /proc/%d/cmdline", pid);
    FILE *fp=popen(cmd, "r");
    if (fp == nullptr)
        return FAIL_PROCESS_NAME;
    if (fgets(buf, BUF_SIZE, fp) == nullptr)
        return FAIL_PROCESS_NAME;
    LOG(INFO)<<"process:"<<pid<<"'s name is:"<<buf;
    pclose(fp);
    return string(buf);
}

void get_name_by_pid(pid_t pid,string &process_name) {
    char buf[BUF_SIZE];
    char cmd[100] = {'\0'};
    sprintf(cmd, "cat /proc/%d/cmdline", pid);
    FILE *fp=popen(cmd, "r");
    if (fp == nullptr)
        return;
    if (fgets(buf, BUF_SIZE, fp) == nullptr)
        return;
    LOG(INFO)<<"process:"<<pid<<"'s name is:"<<buf;
    pclose(fp);
    process_name=string(buf);
}

bool check_dir_rw(const char* path){
    return access(path,W_OK)>=0;
}
