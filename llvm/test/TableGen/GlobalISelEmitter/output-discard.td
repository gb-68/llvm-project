// RUN: llvm-tblgen -gen-global-isel -warn-on-skipped-patterns -I %p/../../../include -I %p/../Common %s | FileCheck -check-prefix=GISEL %s

include "llvm/Target/Target.td"
include "GlobalISelEmitterCommon.td"

// Test that extra explicit results are treated as dead defs.
def ADD_CO : I<(outs GPR32:$dst, GPR8:$flag),
               (ins GPR32Op:$src0, GPR32Op:$src1), []>;

// GISEL: GIM_CheckOpcode, /*MI*/0, GIMT_Encode2(TargetOpcode::G_ADD),
// GISEL-NEXT: GIM_RootCheckType, /*Op*/0, /*Type*/GILLT_s32,
// GISEL-NEXT: GIM_RootCheckType, /*Op*/1, /*Type*/GILLT_s32,
// GISEL-NEXT: GIM_RootCheckType, /*Op*/2, /*Type*/GILLT_s32,
// GISEL-NEXT: GIM_RootCheckRegBankForClass, /*Op*/0, /*RC*/GIMT_Encode2(MyTarget::GPR32RegClassID),
// GISEL-NEXT: // (add:{ *:[i32] } i32:{ *:[i32] }:$src0, i32:{ *:[i32] }:$src1)  =>  (ADD_CO:{ *:[i32] }:{ *:[i8] } GPR32:{ *:[i32] }:$src0, GPR32:{ *:[i32] }:$src1)
// GISEL-NEXT: GIR_MakeTempReg, /*TempRegID*/0, /*TypeID*/GILLT_s8,
// GISEL-NEXT: GIR_BuildRootMI, /*Opcode*/GIMT_Encode2(MyTarget::ADD_CO),
// GISEL-NEXT: GIR_RootToRootCopy, /*OpIdx*/0, // DstI[dst]
// GISEL-NEXT: GIR_AddTempRegister, /*InsnID*/0, /*TempRegID*/0, /*TempRegFlags*/GIMT_Encode2(RegState::Define|RegState::Dead),
// GISEL-NEXT: GIR_RootToRootCopy, /*OpIdx*/1, // src0
// GISEL-NEXT: GIR_RootToRootCopy, /*OpIdx*/2, // src1
// GISEL-NEXT: GIR_RootConstrainSelectedInstOperands,
// GISEL-NEXT: // GIR_Coverage
// GISEL-NEXT: GIR_EraseRootFromParent_Done,
def : Pat <
  (add i32:$src0, i32:$src1),
  (ADD_CO GPR32:$src0, GPR32:$src1)
>;
