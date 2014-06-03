//===-- QpuMachineFunctionInfo.cpp - Private data used for Qpu ----------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "QpuMachineFunction.h"
#include "QpuInstrInfo.h"
#include "QpuSubtarget.h"
#include "MCTargetDesc/QpuBaseInfo.h"
#include "llvm/IR/Function.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"

using namespace llvm;

bool FixGlobalBaseReg = true;

bool QpuFunctionInfo::globalBaseRegFixed() const {
  return FixGlobalBaseReg;
}

bool QpuFunctionInfo::globalBaseRegSet() const {
  return GlobalBaseReg;
}

unsigned QpuFunctionInfo::getGlobalBaseReg() {
  // Return if it has already been initialized.
  if (GlobalBaseReg)
    return GlobalBaseReg;

  if (FixGlobalBaseReg) // $gp is the global base register.
    return GlobalBaseReg = Qpu::GP;

  const TargetRegisterClass *RC;
  RC = (const TargetRegisterClass*)&Qpu::CPURegsRegClass;

  return GlobalBaseReg = MF.getRegInfo().createVirtualRegister(RC);
}

void QpuFunctionInfo::anchor() { }
