#include "whale.h"
#include "android/dvm/dvm_symbol_resolver.h"
#include "android/android_build.h"
#include "android/dvm/dvm_runtime.h"

#define SYMBOL static constexpr const char *

namespace whale {
namespace dvm {
// dvmDbgSuspendVM(bool isEvent)
SYMBOL kDbg_SuspendVM = "_Z15dvmDbgSuspendVMb";
// dvmDbgResumeVM()
SYMBOL kDbg_ResumeVM = "_Z14dvmDbgResumeVMv";
bool DvmSymbolResolver::Resolve(void *elf_image, s4 api_level) {
#define FIND_SYMBOL(symbol, decl, ret)  \
        if ((decl = reinterpret_cast<typeof(decl)>(WDynamicLibSymbol(elf_image, symbol))) == nullptr) {  \
        if (ret) {  \
            LOG(ERROR) << "Failed to resolve symbol : " << #symbol;  \
            return false;  \
         } \
        }

    FIND_SYMBOL(kDbg_SuspendVM, symbols_.Dbg_SuspendVM, false);
    FIND_SYMBOL(kDbg_ResumeVM, symbols_.Dbg_ResumeVM, false);
    return true;
#undef FIND_SYMBOL
}

}  // namespace dvm
}  // namespace whale
