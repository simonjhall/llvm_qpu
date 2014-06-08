//===-- QpuFixupKinds.h - Qpu Specific Fixup Entries ----------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_QPU_QPUFIXUPKINDS_H
#define LLVM_QPU_QPUFIXUPKINDS_H

#include "llvm/MC/MCFixup.h"

namespace llvm {
namespace Qpu {
  // Although most of the current fixup types reflect a unique relocation
  // one can have multiple fixup types for a given relocation and thus need
  // to be uniquely named.
  //
  // This table *must* be in the save order of
  // MCFixupKindInfo Infos[Qpu::NumTargetFixupKinds]
  // in QpuAsmBackend.cpp.
  //
  enum Fixups {
    // Jump 24 bit fixup resulting in - R_QPU_24.
    fixup_Qpu_24 = FirstTargetFixupKind,

    // Pure upper 32 bit fixup resulting in - R_QPU_32.
    fixup_Qpu_32,

    // Pure upper 16 bit fixup resulting in - R_QPU_HI16.
//    fixup_Qpu_HI16,

    // Pure lower 16 bit fixup resulting in - R_QPU_LO16.
//    fixup_Qpu_LO16,
    fixup_Qpu_HILO,

    // Pure lower 16 bit fixup resulting in - R_QPU_GPREL16.
    fixup_Qpu_GPREL16,

    // Global symbol fixup resulting in - R_QPU_GOT16.
    fixup_Qpu_GOT_Global,

    // Local symbol fixup resulting in - R_QPU_GOT16.
    fixup_Qpu_GOT_Local,
    
    // PC relative branch fixup resulting in - R_QPU_PC16.
    // qpu PC16, e.g. beq
    fixup_Qpu_PC16,
    
    // PC relative branch fixup resulting in - R_QPU_PC24.
    // qpu PC24, e.g. jeq, jmp
    fixup_Qpu_PC24,
    
    // resulting in - R_QPU_CALL16.
    fixup_Qpu_CALL16,

    // resulting in - R_QPU_GOT_HI16
    fixup_Qpu_GOT_HI16,

    // resulting in - R_QPU_GOT_LO16
    fixup_Qpu_GOT_LO16,

    // Marker
    LastTargetFixupKind,
    NumTargetFixupKinds = LastTargetFixupKind - FirstTargetFixupKind
  };
} // namespace Qpu
} // namespace llvm


#endif // LLVM_QPU_QPUFIXUPKINDS_H
