//
// Created by FKD on 19-6-29.
//

#include <cstring>
#include "str_util.h"

bool Contains(const char *u, const char *sub_str) {
    return (strstr(u,sub_str)!= nullptr);
}
