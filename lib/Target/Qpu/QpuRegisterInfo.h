//===-- QpuRegisterInfo.h - Qpu Register Information Impl -----*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the Qpu implementation of the TargetRegisterInfo class.
//
//===----------------------------------------------------------------------===//

#ifndef QPUREGISTERINFO_H
#define QPUREGISTERINFO_H

#include "Qpu.h"
#include "llvm/Target/TargetRegisterInfo.h"

#define GET_REGINFO_HEADER
#include "QpuGenRegisterInfo.inc"

namespace llvm {
class QpuSubtarget;
class TargetInstrInfo;
class Type;

struct QpuRegisterInfo : public QpuGenRegisterInfo {
  const QpuSubtarget &Subtarget;
  const TargetInstrInfo &TII;

  QpuRegisterInfo(const QpuSubtarget &Subtarget, const TargetInstrInfo &tii);

  /// getRegisterNumbering - Given the enum value for some register, e.g.
  /// Qpu::RA, return the number that it corresponds to (e.g. 31).
//  static unsigned getRegisterNumbering(unsigned RegEnum);

  /// Code Generation virtual methods...
  const uint16_t *getCalleeSavedRegs(const MachineFunction* MF = 0) const;
  const uint32_t *getCallPreservedMask(CallingConv::ID) const;

// pure virtual method
  BitVector getReservedRegs(const MachineFunction &MF) const;

// pure virtual method
  /// Stack Frame Processing Methods
  void eliminateFrameIndex(MachineBasicBlock::iterator II,
                           int SPAdj, unsigned FIOperandNum,
                           RegScavenger *RS = NULL) const;

// pure virtual method
  /// Debug information queries.
  unsigned getFrameRegister(const MachineFunction &MF) const;
};

} // end namespace llvm

#endif
