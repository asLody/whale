#include <whale.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <iostream>

char *(*Origin_getenv)(const char *);

char *Hooked_getenv(const char *name) {
    if (!strcmp(name, "lody")) {
        return strdup("are you ok?");
    }
    char *(*O)(const char *) = Origin_getenv;
    return O(name);
}

int main() {
#if defined(__APPLE__)
    void *handle = dlopen("libc.dylib", RTLD_NOW);
#else
    void *handle = dlopen("libc.so", RTLD_NOW);
#endif
    assert(handle != nullptr);
    void *symbol = dlsym(handle, "getenv");
    assert(symbol != nullptr);
    WInlineHookFunction(
            symbol,
            reinterpret_cast<void *>(Hooked_getenv),
            reinterpret_cast<void **>(&Origin_getenv)
    );
//    WImportHookFunction(
//            "_getenv",
//            reinterpret_cast<void *>(Hooked_getenv),
//            reinterpret_cast<void **>(&Origin_getenv)
//    );

    const char *val = getenv("lody");
    std::cout << val;
    return 0;
}
