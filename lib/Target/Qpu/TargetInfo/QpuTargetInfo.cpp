//===-- QpuTargetInfo.cpp - Qpu Target Implementation -------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "Qpu.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/TargetRegistry.h"
using namespace llvm;

Target llvm::TheQpuTarget, llvm::TheQpuelTarget;

extern "C" void LLVMInitializeQpuTargetInfo() {
  RegisterTarget<Triple::qpu,
        /*HasJIT=*/true> X(TheQpuTarget, "qpu", "Qpu");

  RegisterTarget<Triple::qpuel,
        /*HasJIT=*/true> Y(TheQpuelTarget, "qpuel", "Qpuel");
}
