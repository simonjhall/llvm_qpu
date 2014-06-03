//===-- QpuSelectionDAGInfo.cpp - Qpu SelectionDAG Info -----------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements the QpuSelectionDAGInfo class.
//
//===----------------------------------------------------------------------===//

#define DEBUG_TYPE "qpu-selectiondag-info"
#include "QpuTargetMachine.h"
using namespace llvm;

QpuSelectionDAGInfo::QpuSelectionDAGInfo(const QpuTargetMachine &TM)
  : TargetSelectionDAGInfo(TM) {
}

QpuSelectionDAGInfo::~QpuSelectionDAGInfo() {
}
