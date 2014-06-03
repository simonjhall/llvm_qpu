//===-- QpuTargetMachine.cpp - Define TargetMachine for Qpu -------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// Implements the info about Qpu target spec.
//
//===----------------------------------------------------------------------===//

#include "QpuTargetMachine.h"
#include "Qpu.h"
#include "llvm/PassManager.h"
#include "llvm/CodeGen/Passes.h"
#include "llvm/Support/TargetRegistry.h"
using namespace llvm;

extern "C" void LLVMInitializeQpuTarget() {
  // Register the target.
  //- Big endian Target Machine
  RegisterTargetMachine<QpuebTargetMachine> X(TheQpuTarget);
  //- Little endian Target Machine
  RegisterTargetMachine<QpuelTargetMachine> Y(TheQpuelTarget);
}

// DataLayout --> Big-endian, 32-bit pointer/ABI/alignment
// The stack is always 8 byte aligned
// On function prologue, the stack is created by decrementing
// its pointer. Once decremented, all references are done with positive
// offset from the stack/frame pointer, using StackGrowsUp enables
// an easier handling.
// Using CodeModel::Large enables different CALL behavior.
QpuTargetMachine::
QpuTargetMachine(const Target &T, StringRef TT,
                  StringRef CPU, StringRef FS, const TargetOptions &Options,
                  Reloc::Model RM, CodeModel::Model CM,
                  CodeGenOpt::Level OL,
                  bool isLittle)
  //- Default is big endian
  : LLVMTargetMachine(T, TT, CPU, FS, Options, RM, CM, OL),
    Subtarget(TT, CPU, FS, isLittle, RM),
    DL(isLittle ?
               ("e-p:32:32:32-i8:8:32-i16:16:32-i64:64:64-n32") :
               ("E-p:32:32:32-i8:8:32-i16:16:32-i64:64:64-n32")),
    InstrInfo(*this),
    FrameLowering(Subtarget), 
    TLInfo(*this), TSInfo(*this) {
  initAsmInfo();
}

void QpuebTargetMachine::anchor() { }

QpuebTargetMachine::
QpuebTargetMachine(const Target &T, StringRef TT,
                    StringRef CPU, StringRef FS, const TargetOptions &Options,
                    Reloc::Model RM, CodeModel::Model CM,
                    CodeGenOpt::Level OL)
  : QpuTargetMachine(T, TT, CPU, FS, Options, RM, CM, OL, false) {}

void QpuelTargetMachine::anchor() { }

QpuelTargetMachine::
QpuelTargetMachine(const Target &T, StringRef TT,
                    StringRef CPU, StringRef FS, const TargetOptions &Options,
                    Reloc::Model RM, CodeModel::Model CM,
                    CodeGenOpt::Level OL)
  : QpuTargetMachine(T, TT, CPU, FS, Options, RM, CM, OL, true) {}
namespace {
/// Qpu Code Generator Pass Configuration Options.
class QpuPassConfig : public TargetPassConfig {
public:
  QpuPassConfig(QpuTargetMachine *TM, PassManagerBase &PM)
    : TargetPassConfig(TM, PM) {}

  QpuTargetMachine &getQpuTargetMachine() const {
    return getTM<QpuTargetMachine>();
  }

  const QpuSubtarget &getQpuSubtarget() const {
    return *getQpuTargetMachine().getSubtargetImpl();
  } // lbd document - mark - getQpuSubtarget()
  virtual bool addInstSelector();
  virtual bool addPreRegAlloc();
  virtual bool addPreEmitPass();
};
} // namespace

TargetPassConfig *QpuTargetMachine::createPassConfig(PassManagerBase &PM) {
  return new QpuPassConfig(this, PM);
} // lbd document - mark - createPassConfig

// Install an instruction selector pass using
// the ISelDag to gen Qpu code.
bool QpuPassConfig::addInstSelector() {
  addPass(createQpuISelDag(getQpuTargetMachine()));
  return false;
} // lbd document - mark - addInstSelector()

bool QpuPassConfig::addPreRegAlloc() {
  // $gp is a caller-saved register.

  addPass(createQpuEmitGPRestorePass(getQpuTargetMachine()));
  return true;
}

// Implemented by targets that want to run passes immediately before
// machine code is emitted. return true if -print-machineinstrs should
// print out the code after the passes.
bool QpuPassConfig::addPreEmitPass() {
	QpuTargetMachine &TM = getQpuTargetMachine();
//	addPass(createQpuNopperPass(TM));
//	addPass(createQpuDelJmpPass(TM));
	return true;
}
