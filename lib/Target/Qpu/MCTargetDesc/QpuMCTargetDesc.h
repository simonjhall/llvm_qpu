//===-- QpuMCTargetDesc.h - Qpu Target Descriptions -----------*- C++ -*-===//
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

#ifndef QPUMCTARGETDESC_H
#define QPUMCTARGETDESC_H

#include "llvm/Support/DataTypes.h"

namespace llvm {
class MCAsmBackend;
class MCCodeEmitter;
class MCContext;
class MCInstrInfo;
class MCObjectWriter;
class MCRegisterInfo;
class MCSubtargetInfo;
class StringRef;
class Target;
class raw_ostream;

extern Target TheQpuTarget;
extern Target TheQpuelTarget;

MCCodeEmitter *createQpuMCCodeEmitterEB(const MCInstrInfo &MCII,
                                         const MCRegisterInfo &MRI,
                                         const MCSubtargetInfo &STI,
                                         MCContext &Ctx);
MCCodeEmitter *createQpuMCCodeEmitterEL(const MCInstrInfo &MCII,
                                         const MCRegisterInfo &MRI,
                                         const MCSubtargetInfo &STI,
                                         MCContext &Ctx);
// lbd document - mark - createQpuMCCodeEmitterEL

MCAsmBackend *createQpuAsmBackendEB32(const Target &T, const MCRegisterInfo &MRI,
                                       StringRef TT, StringRef CPU);
MCAsmBackend *createQpuAsmBackendEL32(const Target &T, const MCRegisterInfo &MRI,
                                       StringRef TT, StringRef CPU);
// lbd document - mark - createQpuAsmBackendEL32

MCObjectWriter *createQpuELFObjectWriter(raw_ostream &OS,
                                          uint8_t OSABI,
                                          bool IsLittleEndian);
} // End llvm namespace

// Defines symbolic names for Qpu registers.  This defines a mapping from
// register name to register number.
#define GET_REGINFO_ENUM
#include "QpuGenRegisterInfo.inc"

// Defines symbolic names for the Qpu instructions.
#define GET_INSTRINFO_ENUM
#include "QpuGenInstrInfo.inc"

#define GET_SUBTARGETINFO_ENUM
#include "QpuGenSubtargetInfo.inc"
#endif


