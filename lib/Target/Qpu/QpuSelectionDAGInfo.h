//===-- QpuSelectionDAGInfo.h - Qpu SelectionDAG Info ---------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file defines the Qpu subclass for TargetSelectionDAGInfo.
//
//===----------------------------------------------------------------------===//

#ifndef QPUSELECTIONDAGINFO_H
#define QPUSELECTIONDAGINFO_H

#include "llvm/Target/TargetSelectionDAGInfo.h"

namespace llvm {

class QpuTargetMachine;

class QpuSelectionDAGInfo : public TargetSelectionDAGInfo {
public:
  explicit QpuSelectionDAGInfo(const QpuTargetMachine &TM);
  ~QpuSelectionDAGInfo();
};

}

#endif
