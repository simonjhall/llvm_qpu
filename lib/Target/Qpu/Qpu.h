//===-- Qpu.h - Top-level interface for Qpu representation ----*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the entry points for global functions defined in
// the LLVM Qpu back-end.
//
//===----------------------------------------------------------------------===//

#ifndef TARGET_QPU_H
#define TARGET_QPU_H

#include "MCTargetDesc/QpuMCTargetDesc.h"
#include "llvm/Target/TargetMachine.h"

namespace llvm {
  class QpuTargetMachine;
  class FunctionPass;

  FunctionPass *createQpuISelDag(QpuTargetMachine &TM);
  FunctionPass *createQpuEmitGPRestorePass(QpuTargetMachine &TM);
  FunctionPass *createQpuDelJmpPass(QpuTargetMachine &TM);
  FunctionPass *createQpuNopperPass(QpuTargetMachine &TM);

} // end namespace llvm;

#endif
