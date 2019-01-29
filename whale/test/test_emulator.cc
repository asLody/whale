#include "base/logging.h"
#include "dbi/arm64/inline_hook_arm64.h"
#include "simulator/code_simulator_container.h"
#include "simulator/code_simulator.h"
#include "vixl/aarch64/macro-assembler-aarch64.h"
#include "dbi/arm/decoder_arm.h"

using namespace whale;  //NOLINT
using namespace whale::arm;  //NOLINT
using namespace whale::arm64;  //NOLINT
using namespace vixl::aarch64;  //NOLINT

#define __ masm->

void GenerateOriginFunction(MacroAssembler *masm) {
    Label label, label2;
    __ Mov(x1, 0);
    __ Cbz(x1, &label);
    __ Mov(x0, 111);
    __ B(&label2);
    __ Bind(&label);
    __ Mov(x0, 222);
    __ Bind(&label2);
    __ Ret();
    masm->FinalizeCode();
}

void GenerateReplaceFunction(MacroAssembler *masm) {
    __ Mov(x0, 999);
    __ Ret();
    masm->FinalizeCode();
}


int main() {
    CodeSimulatorContainer emulator(InstructionSet::kArm64);
    MacroAssembler masm_origin, masm_replace;
    GenerateOriginFunction(&masm_origin);
    GenerateReplaceFunction(&masm_replace);

    auto target = masm_origin.GetBuffer()->GetStartAddress<intptr_t>();
    auto replace = masm_replace.GetBuffer()->GetStartAddress<intptr_t>();
    emulator.Get()->RunFrom(target);
    LOG(INFO) << "Before Hook, result: "<< emulator.Get()->GetCReturnInt32();
    intptr_t origin;
    Arm64InlineHook hook(target, reinterpret_cast<intptr_t>(replace), &origin);
    hook.StartHook();
    emulator.Get()->RunFrom(replace);
    LOG(INFO) << "After Hook, hooked result:"<< emulator.Get()->GetCReturnInt32();
    emulator.Get()->RunFrom(origin);
    LOG(INFO) << "After Hook: origin result:"<< emulator.Get()->GetCReturnInt32();
}
