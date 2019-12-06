#ifndef WHALE_ANDROID_DVM_SYMBOL_RESOLVER_H_
#define WHALE_ANDROID_DVM_SYMBOL_RESOLVER_H_

#include <jni.h>
#include "platform/linux/elf_image.h"
#include "base/primitive_types.h"

namespace whale {
namespace dvm {

struct DvmResolvedSymbols {

    void (*Dbg_SuspendVM)(bool isEvent);

    void (*Dbg_ResumeVM)();

};

class DvmSymbolResolver {
 public:
    DvmSymbolResolver() = default;

    bool Resolve(void *elf_image, s4 api_level);

    DvmResolvedSymbols *GetSymbols() {
        return &symbols_;
    };

 private:
    DvmResolvedSymbols symbols_;
};

}  // namespace dvm
}  // namespace whale

#endif  // WHALE_ANDROID_DVM_SYMBOL_RESOLVER_H_
