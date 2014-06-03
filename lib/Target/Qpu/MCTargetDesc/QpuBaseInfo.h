//===-- QpuBaseInfo.h - Top level definitions for QPU MC ------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains small standalone helper functions and enum definitions for
// the Qpu target useful for the compiler back-end and the MC libraries.
//
//===----------------------------------------------------------------------===//
#ifndef QPUBASEINFO_H
#define QPUBASEINFO_H

#include "QpuFixupKinds.h"
#include "QpuMCTargetDesc.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/Support/DataTypes.h"
#include "llvm/Support/ErrorHandling.h"

namespace llvm {

/// QpuII - This namespace holds all of the target specific flags that
/// instruction info tracks.
///
namespace QpuII {
  /// Target Operand Flag enum.
  enum TOF {
    //===------------------------------------------------------------------===//
    // Qpu Specific MachineOperand flags.

    MO_NO_FLAG,

    /// MO_GOT16 - Represents the offset into the global offset table at which
    /// the address the relocation entry symbol resides during execution.
    MO_GOT16,
    MO_GOT,

    /// MO_GOT_CALL - Represents the offset into the global offset table at
    /// which the address of a call site relocation entry symbol resides
    /// during execution. This is different from the above since this flag
    /// can only be present in call instructions.
    MO_GOT_CALL,

    /// MO_GPREL - Represents the offset from the current gp value to be used
    /// for the relocatable object file being produced.
    MO_GPREL,

    /// MO_ABS_HI/LO - Represents the hi or low part of an absolute symbol
    /// address.
    MO_ABS_HI,
    MO_ABS_LO,

    /// MO_TLSGD - Represents the offset into the global offset table at which
    // the module ID and TSL block offset reside during execution (General
    // Dynamic TLS).
    MO_TLSGD,

    /// MO_TLSLDM - Represents the offset into the global offset table at which
    // the module ID and TSL block offset reside during execution (Local
    // Dynamic TLS).
    MO_TLSLDM,
    MO_DTPREL_HI,
    MO_DTPREL_LO,

    /// MO_GOTTPREL - Represents the offset from the thread pointer (Initial
    // Exec TLS).
    MO_GOTTPREL,

    /// MO_TPREL_HI/LO - Represents the hi and low part of the offset from
    // the thread pointer (Local Exec TLS).
    MO_TPREL_HI,
    MO_TPREL_LO,

    // N32/64 Flags.
    MO_GPOFF_HI,
    MO_GPOFF_LO,

    /// MO_GOT_HI16/LO16 - Relocations used for large GOTs.
    MO_GOT_HI16,
    MO_GOT_LO16,
    MO_GOT_DISP,
    MO_GOT_PAGE,
    MO_GOT_OFST
  }; // enum TOF {

  enum {
    //===------------------------------------------------------------------===//
    // Instruction encodings.  These are the standard/most common forms for
    // Qpu instructions.
    //

    // Pseudo - This represents an instruction that is a pseudo instruction
    // or one that has not been implemented yet.  It is illegal to code generate
    // it, but tolerated for intermediate implementation stages.
    Pseudo   = 0,

    /// FrmR - This form is for instructions of the format R.
    FrmR  = 1,
    /// FrmI - This form is for instructions of the format I.
    FrmI  = 2,
    /// FrmJ - This form is for instructions of the format J.
    FrmJ  = 3,
    /// FrmOther - This form is for instructions that have no specific format.
    FrmOther = 4,

    FormMask = 15
  };
}

/// getQpuRegisterNumbering - Given the enum value for some register,
/// return the number that it corresponds to.
inline static unsigned getQpuRegisterNumbering(unsigned RegEnum)
{
  switch (RegEnum) {
  case Qpu::ZERO_IN:
  case Qpu::ZERO_OUT:
    return 0;
  case Qpu::AT:
    return 1;
  case Qpu::RA0:
    return 2;
  case Qpu::RA1:
    return 3;
  case Qpu::RA2:
    return 4;
  case Qpu::RA3:
    return 5;
  case Qpu::T9:
    return 6;
  case Qpu::T0:
    return 7;
  case Qpu::SW:
    return 10;
  case Qpu::GP:
    return 11;
  case Qpu::FP:
    return 12;
  case Qpu::SP:
    return 13;
  case Qpu::LR:
    return 14;
  case Qpu::PC:
    return 15;
  case Qpu::VPM_LD_ADDR:
	  return 16;
  case Qpu::VPM_ST_ADDR:
	  return 17;
  case Qpu::VPM_LD_WAIT:
  	  return 18;
  case Qpu::VPM_ST_WAIT:
  	  return 19;
  case Qpu::VPM_DAT_RDA:
	  return 20;
  case Qpu::VPM_DAT_WRA:
	  return 21;
  case Qpu::VPM_LD_SETUP:
	  return 22;
  case Qpu::VPM_ST_SETUP:
	  return 23;
  case Qpu::ACC0:
    return 24;
  case Qpu::ACC1:
    return 25;
  case Qpu::ACC2:
    return 26;
  case Qpu::ACC3:
    return 27;
  case Qpu::ACC5:
    return 28;
  /*case Qpu::HI:
    return 18;
  case Qpu::LO:
    return 19;*/
  default: llvm_unreachable("Unknown register number!");
  }
} // lbd document - mark - getQpuRegisterNumbering

inline static std::pair<const MCSymbolRefExpr*, int64_t>
QpuGetSymAndOffset(const MCFixup &Fixup) {
  MCFixupKind FixupKind = Fixup.getKind();

  if ((FixupKind < FirstTargetFixupKind) ||
      (FixupKind >= MCFixupKind(Qpu::LastTargetFixupKind)))
    return std::make_pair((const MCSymbolRefExpr*)0, (int64_t)0);

  const MCExpr *Expr = Fixup.getValue();
  MCExpr::ExprKind Kind = Expr->getKind();

  if (Kind == MCExpr::Binary) {
    const MCBinaryExpr *BE = static_cast<const MCBinaryExpr*>(Expr);
    const MCExpr *LHS = BE->getLHS();
    const MCConstantExpr *CE = dyn_cast<MCConstantExpr>(BE->getRHS());

    if ((LHS->getKind() != MCExpr::SymbolRef) || !CE)
      return std::make_pair((const MCSymbolRefExpr*)0, (int64_t)0);

    return std::make_pair(cast<MCSymbolRefExpr>(LHS), CE->getValue());
  }

  if (Kind != MCExpr::SymbolRef)
    return std::make_pair((const MCSymbolRefExpr*)0, (int64_t)0);

  return std::make_pair(cast<MCSymbolRefExpr>(Expr), 0);
} // QpuGetSymAndOffset
} // lbd document - mark - namespace llvm - end

#endif
