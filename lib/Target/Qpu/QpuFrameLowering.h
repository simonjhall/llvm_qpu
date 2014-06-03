//===-- QpuFrameLowering.h - Define frame lowering for Qpu ----*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
//
//
//===----------------------------------------------------------------------===//
#ifndef QPU_FRAMEINFO_H
#define QPU_FRAMEINFO_H

#include "Qpu.h"
#include "QpuSubtarget.h"
#include "llvm/Target/TargetFrameLowering.h"

namespace llvm {
  class QpuSubtarget;

class QpuFrameLowering : public TargetFrameLowering {
protected:
  const QpuSubtarget &STI;

public:
  explicit QpuFrameLowering(const QpuSubtarget &sti)
    : TargetFrameLowering(StackGrowsDown, 8, 0),
      STI(sti) {
  } // lbd document - mark - explicit QpuFrameLowering

  /// emitProlog/emitEpilog - These methods insert prolog and epilog code into
  /// the function.
  void emitPrologue(MachineFunction &MF) const;
  void emitEpilogue(MachineFunction &MF, MachineBasicBlock &MBB) const;
  bool hasFP(const MachineFunction &MF) const;
  void eliminateCallFramePseudoInstr(MachineFunction &MF,
                                     MachineBasicBlock &MBB,
                                     MachineBasicBlock::iterator I) const;
  bool spillCalleeSavedRegisters(MachineBasicBlock &MBB,
                                 MachineBasicBlock::iterator MI,
                                 const std::vector<CalleeSavedInfo> &CSI,
                                 const TargetRegisterInfo *TRI) const;
  void processFunctionBeforeCalleeSavedScan(MachineFunction &MF,
                                            RegScavenger *RS) const;
};

} // End llvm namespace

#endif

