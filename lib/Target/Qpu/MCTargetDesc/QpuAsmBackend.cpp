//===-- QpuASMBackend.cpp - Qpu Asm Backend  ----------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements the QpuAsmBackend and QpuELFObjectWriter classes.
//
//===----------------------------------------------------------------------===//
//

#include "QpuFixupKinds.h"
#include "MCTargetDesc/QpuMCTargetDesc.h"
#include "llvm/MC/MCAsmBackend.h"
#include "llvm/MC/MCAssembler.h"
#include "llvm/MC/MCDirectives.h"
#include "llvm/MC/MCELFObjectWriter.h"
#include "llvm/MC/MCFixupKindInfo.h"
#include "llvm/MC/MCObjectWriter.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

// Prepare value for the target space for it
static unsigned adjustFixupValue(unsigned Kind, uint64_t Value) {

  // Add/subtract and shift
  switch (Kind) {
  default:
    return 0;
  case Qpu::fixup_Qpu_HILO:
	  return Value;
  case FK_GPRel_4:
  case FK_Data_4:
  case Qpu::fixup_Qpu_CALL16:
//  case Qpu::fixup_Qpu_LO16:
  case Qpu::fixup_Qpu_GOT_LO16:
    break;
  case Qpu::fixup_Qpu_PC16:
  case Qpu::fixup_Qpu_PC24:
    // So far we are only using this type for branches and jump.
    // For branches we start 1 instruction after the branch
    // so the displacement will be one instruction size less.
    Value -= 4;
    break;
  case Qpu::fixup_Qpu_24:
    // So far we are only using this type for instruction SWI.
    break;
//  case Qpu::fixup_Qpu_HI16:
  case Qpu::fixup_Qpu_GOT_Local:
  case Qpu::fixup_Qpu_GOT_HI16:
    // Get the higher 16-bits. Also add 1 if bit 15 is 1.
    Value = ((Value + 0x8000) >> 16) & 0xffff;
    break;
  }

  return Value;
}

namespace {
class QpuAsmBackend : public MCAsmBackend {
  Triple::OSType OSType;
  bool IsLittle; // Big or little endian

public:
  QpuAsmBackend(const Target &T,  Triple::OSType _OSType,
                 bool _isLittle)
    :MCAsmBackend(), OSType(_OSType), IsLittle(_isLittle) {}

  MCObjectWriter *createObjectWriter(raw_ostream &OS) const {
  // Change Reason:
  // Reduce the exposure of Triple::OSType in the ELF object writer. This will
  //  avoid including ADT/Triple.h in many places when the target specific bits 
  //  are moved.
    return createQpuELFObjectWriter(OS,
      MCELFObjectTargetWriter::getOSABI(OSType), IsLittle);
  }

  /// ApplyFixup - Apply the \arg Value for given \arg Fixup into the provided
  /// data fragment, at the offset specified by the fixup and following the
  /// fixup kind as appropriate.
  void applyFixup(const MCFixup &Fixup, char *Data, unsigned DataSize,
                  uint64_t Value) const {
    MCFixupKind Kind = Fixup.getKind();
    Value = adjustFixupValue((unsigned)Kind, Value);

    if (!Value)
      return; // Doesn't change encoding.

    // Where do we start in the object
    unsigned Offset = Fixup.getOffset();
    // Number of bytes we need to fixup
    unsigned NumBytes = (getFixupKindInfo(Kind).TargetSize + 7) / 8;
    // Used to point to big endian bytes
    unsigned FullSize;

    switch ((unsigned)Kind) {
    case Qpu::fixup_Qpu_24:
      FullSize = 3;
      break;
    default:
      FullSize = 4;
      break;
    }

    // Grab current value, if any, from bits.
    uint64_t CurVal = 0;

    for (unsigned i = 0; i != NumBytes; ++i) {
      unsigned Idx = IsLittle ? i : (FullSize - 1 - i);
      CurVal |= (uint64_t)((uint8_t)Data[Offset + Idx]) << (i*8);
    }

    uint64_t Mask = ((uint64_t)(-1) >> (64 - getFixupKindInfo(Kind).TargetSize));
    CurVal |= Value & Mask;

    // Write out the fixed up bytes back to the code/data bits.
    for (unsigned i = 0; i != NumBytes; ++i) {
      unsigned Idx = IsLittle ? i : (FullSize - 1 - i);
      Data[Offset + Idx] = (uint8_t)((CurVal >> (i*8)) & 0xff);
    }
  }

  unsigned getNumFixupKinds() const { return Qpu::NumTargetFixupKinds; }

  const MCFixupKindInfo &getFixupKindInfo(MCFixupKind Kind) const {
    const static MCFixupKindInfo Infos[Qpu::NumTargetFixupKinds] = {
      // This table *must* be in same the order of fixup_* kinds in
      // QpuFixupKinds.h.
      //
      // name                        offset  bits  flags
      { "fixup_Qpu_24",             0,     24,   0 },
      { "fixup_Qpu_32",             0,     32,   0 },
//      { "fixup_Qpu_HI16",           0,     16,   0 },
//      { "fixup_Qpu_LO16",           0,     16,   0 },
      { "fixup_Qpu_HILO",           0,     32,   0 },
      { "fixup_Qpu_GPREL16",        0,     16,   0 },
      { "fixup_Qpu_GOT_Global",     0,     16,   0 },
      { "fixup_Qpu_GOT_Local",      0,     16,   0 },
      { "fixup_Qpu_PC16",           0,     16,  MCFixupKindInfo::FKF_IsPCRel },
      { "fixup_Qpu_PC24",           0,     24,  MCFixupKindInfo::FKF_IsPCRel },
      { "fixup_Qpu_CALL16",         0,     16,   0 },
      { "fixup_Qpu_GOT_HI16",       0,     16,   0 },
      { "fixup_Qpu_GOT_LO16",       0,     16,   0 }
    };

    if (Kind < FirstTargetFixupKind)
      return MCAsmBackend::getFixupKindInfo(Kind);

    assert(unsigned(Kind - FirstTargetFixupKind) < getNumFixupKinds() &&
           "Invalid kind!");
    return Infos[Kind - FirstTargetFixupKind];
  }

  /// @name Target Relaxation Interfaces
  /// @{

  /// MayNeedRelaxation - Check whether the given instruction may need
  /// relaxation.
  ///
  /// \param Inst - The instruction to test.
  bool mayNeedRelaxation(const MCInst &Inst) const {
    return false;
  }

  /// fixupNeedsRelaxation - Target specific predicate for whether a given
  /// fixup requires the associated instruction to be relaxed.
  bool fixupNeedsRelaxation(const MCFixup &Fixup,
                            uint64_t Value,
                            const MCRelaxableFragment *DF,
                            const MCAsmLayout &Layout) const {
    // FIXME.
    assert(0 && "RelaxInstruction() unimplemented");
    return false;
  }

  /// RelaxInstruction - Relax the instruction in the given fragment
  /// to the next wider instruction.
  ///
  /// \param Inst - The instruction to relax, which may be the same
  /// as the output.
  /// \parm Res [output] - On return, the relaxed instruction.
  void relaxInstruction(const MCInst &Inst, MCInst &Res) const {
  }

  /// @}

  /// WriteNopData - Write an (optimal) nop sequence of Count bytes
  /// to the given output. If the target cannot generate such a sequence,
  /// it should return an error.
  ///
  /// \return - True on success.
  bool writeNopData(uint64_t Count, MCObjectWriter *OW) const {
    return true;
  }
}; // class QpuAsmBackend

} // namespace

// MCAsmBackend
MCAsmBackend *llvm::createQpuAsmBackendEL32(const Target &T,
                                             const MCRegisterInfo &MRI,
                                             StringRef TT,
                                             StringRef CPU) {
  return new QpuAsmBackend(T, Triple(TT).getOS(),
                            /*IsLittle*/true);
}

MCAsmBackend *llvm::createQpuAsmBackendEB32(const Target &T,
                                             const MCRegisterInfo &MRI,
                                             StringRef TT,
                                             StringRef CPU) {
  return new QpuAsmBackend(T, Triple(TT).getOS(),
                            /*IsLittle*/false);
}

