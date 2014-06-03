//===-- QpuTargetMachine.h - Define TargetMachine for Qpu -----*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file declares the Qpu specific subclass of TargetMachine.
//
//===----------------------------------------------------------------------===//

#ifndef QPUTARGETMACHINE_H
#define QPUTARGETMACHINE_H

#include "QpuFrameLowering.h"
#include "QpuInstrInfo.h"
#include "QpuISelLowering.h"
#include "QpuSelectionDAGInfo.h"
#include "QpuSubtarget.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/Target/TargetFrameLowering.h"

namespace llvm {
  class formatted_raw_ostream;

  class QpuTargetMachine : public LLVMTargetMachine {
    QpuSubtarget       Subtarget;
    const DataLayout    DL; // Calculates type size & alignment
    QpuInstrInfo       InstrInfo;	//- Instructions
    QpuFrameLowering   FrameLowering;	//- Stack(Frame) and Stack direction
    QpuTargetLowering  TLInfo;	//- Stack(Frame) and Stack direction
    QpuSelectionDAGInfo TSInfo;	//- Map .bc DAG to backend DAG

  public:
    QpuTargetMachine(const Target &T, StringRef TT,
                      StringRef CPU, StringRef FS, const TargetOptions &Options,
                      Reloc::Model RM, CodeModel::Model CM,
                      CodeGenOpt::Level OL,
                      bool isLittle);

    virtual const QpuInstrInfo   *getInstrInfo()     const
    { return &InstrInfo; }
    virtual const TargetFrameLowering *getFrameLowering()     const
    { return &FrameLowering; }
    virtual const QpuSubtarget   *getSubtargetImpl() const
    { return &Subtarget; }
    virtual const DataLayout *getDataLayout()    const
    { return &DL;}

    virtual const QpuRegisterInfo *getRegisterInfo()  const {
      return &InstrInfo.getRegisterInfo();
    }

    virtual const QpuTargetLowering *getTargetLowering() const {
      return &TLInfo;
    }

    virtual const QpuSelectionDAGInfo* getSelectionDAGInfo() const {
      return &TSInfo;
    }

    // Pass Pipeline Configuration
    virtual TargetPassConfig *createPassConfig(PassManagerBase &PM);
  };

/// QpuebTargetMachine - Qpu32 big endian target machine.
///
class QpuebTargetMachine : public QpuTargetMachine {
  virtual void anchor();
public:
  QpuebTargetMachine(const Target &T, StringRef TT,
                      StringRef CPU, StringRef FS, const TargetOptions &Options,
                      Reloc::Model RM, CodeModel::Model CM,
                      CodeGenOpt::Level OL);
};

/// QpuelTargetMachine - Qpu32 little endian target machine.
///
class QpuelTargetMachine : public QpuTargetMachine {
  virtual void anchor();
public:
  QpuelTargetMachine(const Target &T, StringRef TT,
                      StringRef CPU, StringRef FS, const TargetOptions &Options,
                      Reloc::Model RM, CodeModel::Model CM,
                      CodeGenOpt::Level OL);
};
} // End llvm namespace

#endif
