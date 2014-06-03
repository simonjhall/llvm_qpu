//===-- QpuMCTargetDesc.cpp - Qpu Target Descriptions -------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file provides Qpu specific target descriptions.
//
//===----------------------------------------------------------------------===//
// #include
#include "QpuMCAsmInfo.h"
#include "QpuMCTargetDesc.h"
#include "InstPrinter/QpuInstPrinter.h"
#include "llvm/MC/MachineLocation.h"
#include "llvm/MC/MCCodeGenInfo.h"
#include "llvm/MC/MCELF.h"
#include "llvm/MC/MCELFStreamer.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/MC/MCSymbol.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/FormattedStream.h"
#include "llvm/Support/TargetRegistry.h"

#define GET_INSTRINFO_MC_DESC
#include "QpuGenInstrInfo.inc"

#define GET_SUBTARGETINFO_MC_DESC
#include "QpuGenSubtargetInfo.inc"

#define GET_REGINFO_MC_DESC
#include "QpuGenRegisterInfo.inc"

using namespace llvm;

static std::string ParseQpuTriple(StringRef TT, StringRef CPU) {
  std::string QpuArchFeature;
  size_t DashPosition = 0;
  StringRef TheTriple;

  // Let's see if there is a dash, like qpu-unknown-linux.
  DashPosition = TT.find('-');

  if (DashPosition == StringRef::npos) {
    // No dash, we check the string size.
    TheTriple = TT.substr(0);
  } else {
    // We are only interested in substring before dash.
    TheTriple = TT.substr(0,DashPosition);
  }

  if (TheTriple == "qpu") {
      QpuArchFeature = "+qpu32I";
  }
  return QpuArchFeature;
}

static MCInstrInfo *createQpuMCInstrInfo() {
  MCInstrInfo *X = new MCInstrInfo();
  InitQpuMCInstrInfo(X); // defined in QpuGenInstrInfo.inc
  return X;
}

static MCRegisterInfo *createQpuMCRegisterInfo(StringRef TT) {
  MCRegisterInfo *X = new MCRegisterInfo();
  InitQpuMCRegisterInfo(X, Qpu::LR); // defined in QpuGenRegisterInfo.inc
  return X;
}

static MCSubtargetInfo *createQpuMCSubtargetInfo(StringRef TT, StringRef CPU,
                                                  StringRef FS) {
  std::string ArchFS = ParseQpuTriple(TT,CPU);
  if (!FS.empty()) {
    if (!ArchFS.empty())
      ArchFS = ArchFS + "," + FS.str();
    else
      ArchFS = FS;
  }
  MCSubtargetInfo *X = new MCSubtargetInfo();
  InitQpuMCSubtargetInfo(X, TT, CPU, ArchFS); // defined in QpuGenSubtargetInfo.inc
  return X;
}

static MCAsmInfo *createQpuMCAsmInfo(const MCRegisterInfo &MRI, StringRef TT) {
  MCAsmInfo *MAI = new QpuMCAsmInfo(TT);

  unsigned SP = MRI.getDwarfRegNum(Qpu::SP, true);
  MCCFIInstruction Inst = MCCFIInstruction::createDefCfa(0, SP, 0);
  MAI->addInitialFrameState(Inst);

  return MAI;
}

static MCCodeGenInfo *createQpuMCCodeGenInfo(StringRef TT, Reloc::Model RM,
                                              CodeModel::Model CM,
                                              CodeGenOpt::Level OL) {
  MCCodeGenInfo *X = new MCCodeGenInfo();
  if (CM == CodeModel::JITDefault)
    RM = Reloc::Static;
  else if (RM == Reloc::Default)
    RM = Reloc::PIC_;
  X->InitMCCodeGenInfo(RM, CM, OL); // defined in lib/MC/MCCodeGenInfo.cpp
  return X;
}

static MCInstPrinter *createQpuMCInstPrinter(const Target &T,
                                              unsigned SyntaxVariant,
                                              const MCAsmInfo &MAI,
                                              const MCInstrInfo &MII,
                                              const MCRegisterInfo &MRI,
                                              const MCSubtargetInfo &STI) {
  return new QpuInstPrinter(MAI, MII, MRI);
} // lbd document - mark - createQpuMCInstPrinter

static MCStreamer *createMCStreamer(const Target &T, StringRef TT,
                                    MCContext &Context, MCAsmBackend &MAB,
                                    raw_ostream &OS, MCCodeEmitter *Emitter,
                                    bool RelaxAll, bool NoExecStack) {
  MCTargetStreamer *S = new MCTargetStreamer();
  return createELFStreamer(Context, S, MAB, OS, Emitter, RelaxAll, NoExecStack);
}

static MCStreamer *
createMCAsmStreamer(MCContext &Ctx, formatted_raw_ostream &OS,
                    bool isVerboseAsm, bool useLoc, bool useCFI,
                    bool useDwarfDirectory, MCInstPrinter *InstPrint,
                    MCCodeEmitter *CE, MCAsmBackend *TAB, bool ShowInst) {
  MCTargetStreamer *S = new MCTargetStreamer();

  return llvm::createAsmStreamer(Ctx, S, OS, isVerboseAsm, useLoc, useCFI,
                                 useDwarfDirectory, InstPrint, CE, TAB,
                                 ShowInst);
} // lbd document - mark - createMCStreamer

extern "C" void LLVMInitializeQpuTargetMC() {
  // Register the MC asm info.
  RegisterMCAsmInfoFn X(TheQpuTarget, createQpuMCAsmInfo);

  // Register the MC codegen info.
  TargetRegistry::RegisterMCCodeGenInfo(TheQpuTarget,
                                        createQpuMCCodeGenInfo);
  // Register the MC instruction info.
  TargetRegistry::RegisterMCInstrInfo(TheQpuTarget, createQpuMCInstrInfo);

  // Register the MC register info.
  TargetRegistry::RegisterMCRegInfo(TheQpuTarget, createQpuMCRegisterInfo);

  // Register the MC Code Emitter
  TargetRegistry::RegisterMCCodeEmitter(TheQpuTarget,
                                        createQpuMCCodeEmitterEB);

  // Register the object streamer.
  TargetRegistry::RegisterMCObjectStreamer(TheQpuTarget, createMCStreamer);

  // Register the asm streamer.
  TargetRegistry::RegisterAsmStreamer(TheQpuTarget, createMCAsmStreamer);

  // Register the asm backend.
  TargetRegistry::RegisterMCAsmBackend(TheQpuTarget,
                                       createQpuAsmBackendEB32);
  // Register the MC subtarget info.
  TargetRegistry::RegisterMCSubtargetInfo(TheQpuTarget,
                                          createQpuMCSubtargetInfo);
  // Register the MCInstPrinter.
  TargetRegistry::RegisterMCInstPrinter(TheQpuTarget,
                                        createQpuMCInstPrinter);
  // lbd document - mark - RegisterMCInstPrinter
}
