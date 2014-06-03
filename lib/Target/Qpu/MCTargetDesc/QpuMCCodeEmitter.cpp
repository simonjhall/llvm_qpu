//===-- QpuMCCodeEmitter.cpp - Convert Qpu Code to Machine Code ---------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements the QpuMCCodeEmitter class.
//
//===----------------------------------------------------------------------===//
//
#define DEBUG_TYPE "mccodeemitter"
#include "MCTargetDesc/QpuBaseInfo.h"
#include "MCTargetDesc/QpuFixupKinds.h"
#include "MCTargetDesc/QpuMCTargetDesc.h"
#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/MC/MCCodeEmitter.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

namespace {
class QpuMCCodeEmitter : public MCCodeEmitter {
  // #define LLVM_DELETED_FUNCTION
  //  LLVM_DELETED_FUNCTION - Expands to = delete if the compiler supports it. 
  //  Use to mark functions as uncallable. Member functions with this should be 
  //  declared private so that some behavior is kept in C++03 mode.
  //  class DontCopy { private: DontCopy(const DontCopy&) LLVM_DELETED_FUNCTION;
  //  DontCopy &operator =(const DontCopy&) LLVM_DELETED_FUNCTION; public: ... };
  //  Definition at line 79 of file Compiler.h.

  QpuMCCodeEmitter(const QpuMCCodeEmitter &) LLVM_DELETED_FUNCTION;
  void operator=(const QpuMCCodeEmitter &) LLVM_DELETED_FUNCTION;

  const MCInstrInfo &MCII;
  const MCSubtargetInfo &STI;
  MCContext &Ctx;
  bool IsLittleEndian;

public:
  QpuMCCodeEmitter(const MCInstrInfo &mcii, const MCSubtargetInfo &sti,
                    MCContext &ctx, bool IsLittle) :
            MCII(mcii), STI(sti) , Ctx(ctx), IsLittleEndian(IsLittle) {}

  ~QpuMCCodeEmitter() {}

  void EmitByte(unsigned char C, raw_ostream &OS) const {
    OS << (char)C;
  }

  void EmitInstruction(uint64_t Val, unsigned Size, raw_ostream &OS) const {
    // Output the instruction encoding in little endian byte order.
    for (unsigned i = 0; i < Size; ++i) {
      unsigned Shift = IsLittleEndian ? i * 8 : (Size - 1 - i) * 8;
      EmitByte((Val >> Shift) & 0xff, OS);
    }
  }

  void EncodeInstruction(const MCInst &MI, raw_ostream &OS,
                         SmallVectorImpl<MCFixup> &Fixups) const;

  // getBinaryCodeForInstr - TableGen'erated function for getting the
  // binary encoding for an instruction.
  uint64_t getBinaryCodeForInstr(const MCInst &MI,
                                 SmallVectorImpl<MCFixup> &Fixups) const;

  // getBranch16TargetOpValue - Return binary encoding of the branch
  // target operand, such as BEQ, BNE. If the machine operand
  // requires relocation, record the relocation and return zero.
  unsigned getBranch16TargetOpValue(const MCInst &MI, unsigned OpNo,
                                  SmallVectorImpl<MCFixup> &Fixups) const;
  // lbd document - mark - declare getBranch16TargetOpValue

  // getBranch24TargetOpValue - Return binary encoding of the branch
  // target operand, such as JMP #BB01, JEQ, JSUB. If the machine operand
  // requires relocation, record the relocation and return zero.
  unsigned getBranch24TargetOpValue(const MCInst &MI, unsigned OpNo,
                                  SmallVectorImpl<MCFixup> &Fixups) const;
                                  
  // getJumpTargetOpValue - Return binary encoding of the jump
  // target operand, such as SWI #interrupt_addr and JSUB #function_addr. 
  // If the machine operand requires relocation,
  // record the relocation and return zero.
   unsigned getJumpTargetOpValue(const MCInst &MI, unsigned OpNo,
                                 SmallVectorImpl<MCFixup> &Fixups) const;
  // lbd document - mark - unsigned getJumpTargetOpValue

  // getMachineOpValue - Return binary encoding of operand. If the machin
  // operand requires relocation, record the relocation and return zero.
  unsigned getMachineOpValue(const MCInst &MI,const MCOperand &MO,
                             SmallVectorImpl<MCFixup> &Fixups) const;

  unsigned getMemEncoding(const MCInst &MI, unsigned OpNo,
                          SmallVectorImpl<MCFixup> &Fixups) const;
}; // class QpuMCCodeEmitter
}  // namespace

MCCodeEmitter *llvm::createQpuMCCodeEmitterEB(const MCInstrInfo &MCII,
                                               const MCRegisterInfo &MRI,
                                               const MCSubtargetInfo &STI,
                                               MCContext &Ctx)
{
  return new QpuMCCodeEmitter(MCII, STI, Ctx, false);
}

MCCodeEmitter *llvm::createQpuMCCodeEmitterEL(const MCInstrInfo &MCII,
                                               const MCRegisterInfo &MRI,
                                               const MCSubtargetInfo &STI,
                                               MCContext &Ctx)
{
  return new QpuMCCodeEmitter(MCII, STI, Ctx, true);
}

/// EncodeInstruction - Emit the instruction.
/// Size the instruction (currently only 4 bytes
void QpuMCCodeEmitter::
EncodeInstruction(const MCInst &MI, raw_ostream &OS,
                  SmallVectorImpl<MCFixup> &Fixups) const
{
  uint32_t Binary = getBinaryCodeForInstr(MI, Fixups);

  // Check for unimplemented opcodes.
  // Unfortunately in QPU both NOT and SLL will come in with Binary == 0
  // so we have to special check for them.
  unsigned Opcode = MI.getOpcode();
  if ((Opcode != Qpu::NOP)/* && (Opcode != Qpu::SHL)*/ && !Binary)
    llvm_unreachable("unimplemented opcode in EncodeInstruction()");

  const MCInstrDesc &Desc = MCII.get(MI.getOpcode());
  uint64_t TSFlags = Desc.TSFlags;

  // Pseudo instructions don't get encoded and shouldn't be here
  // in the first place!
  if ((TSFlags & QpuII::FormMask) == QpuII::Pseudo)
    llvm_unreachable("Pseudo opcode found in EncodeInstruction()");

  // For now all instructions are 4 bytes
  int Size = 4; // FIXME: Have Desc.getSize() return the correct value!

  EmitInstruction(Binary, Size, OS);
}

/// getBranch16TargetOpValue - Return binary encoding of the branch
/// target operand. If the machine operand requires relocation,
/// record the relocation and return zero.
unsigned QpuMCCodeEmitter::
getBranch16TargetOpValue(const MCInst &MI, unsigned OpNo,
                       SmallVectorImpl<MCFixup> &Fixups) const {

  const MCOperand &MO = MI.getOperand(OpNo);

  // If the destination is an immediate, we have nothing to do.
  if (MO.isImm()) return MO.getImm();
  assert(MO.isExpr() && "getBranch16TargetOpValue expects only expressions");

  const MCExpr *Expr = MO.getExpr();
  Fixups.push_back(MCFixup::Create(0, Expr,
                                   MCFixupKind(Qpu::fixup_Qpu_PC16)));
  return 0;
} // lbd document - mark - getBranch16TargetOpValue

/// getBranch24TargetOpValue - Return binary encoding of the branch
/// target operand. If the machine operand requires relocation,
/// record the relocation and return zero.
unsigned QpuMCCodeEmitter::
getBranch24TargetOpValue(const MCInst &MI, unsigned OpNo,
                       SmallVectorImpl<MCFixup> &Fixups) const {

  const MCOperand &MO = MI.getOperand(OpNo);

  // If the destination is an immediate, we have nothing to do.
  if (MO.isImm()) return MO.getImm();
  assert(MO.isExpr() && "getBranch24TargetOpValue expects only expressions");

  const MCExpr *Expr = MO.getExpr();
  Fixups.push_back(MCFixup::Create(0, Expr,
                                   MCFixupKind(Qpu::fixup_Qpu_PC24)));
  return 0;
}

/// getJumpTargetOpValue - Return binary encoding of the jump
/// target operand. Such as SWI and JSUB. 
/// If the machine operand requires relocation,
/// record the relocation and return zero.
unsigned QpuMCCodeEmitter::
getJumpTargetOpValue(const MCInst &MI, unsigned OpNo,
                     SmallVectorImpl<MCFixup> &Fixups) const {

  unsigned Opcode = MI.getOpcode();
  const MCOperand &MO = MI.getOperand(OpNo);
  // If the destination is an immediate, we have nothing to do.
  if (MO.isImm()) return MO.getImm();
  assert(MO.isExpr() && "getJumpTargetOpValue expects only expressions");

  const MCExpr *Expr = MO.getExpr();
  if (Opcode == Qpu::JSUB || Opcode == Qpu::JMP)
    Fixups.push_back(MCFixup::Create(0, Expr,
                                     MCFixupKind(Qpu::fixup_Qpu_PC24)));
  else if (Opcode == Qpu::SWI)
    Fixups.push_back(MCFixup::Create(0, Expr,
                                     MCFixupKind(Qpu::fixup_Qpu_24)));
  else
    llvm_unreachable("unexpect opcode in getJumpAbsoluteTargetOpValue()");
  return 0;
} // lbd document - mark - getJumpTargetOpValue

/// getMachineOpValue - Return binary encoding of operand. If the machine
/// operand requires relocation, record the relocation and return zero.
unsigned QpuMCCodeEmitter::
getMachineOpValue(const MCInst &MI, const MCOperand &MO,
                  SmallVectorImpl<MCFixup> &Fixups) const {
  if (MO.isReg()) {
    unsigned Reg = MO.getReg();
    unsigned RegNo = getQpuRegisterNumbering(Reg);
    return RegNo;
  } else if (MO.isImm()) {
    return static_cast<unsigned>(MO.getImm());
  } else if (MO.isFPImm()) {
    return static_cast<unsigned>(APFloat(MO.getFPImm())
        .bitcastToAPInt().getHiBits(32).getLimitedValue());
  } 

  // MO must be an Expr.
  assert(MO.isExpr());

  const MCExpr *Expr = MO.getExpr();
  MCExpr::ExprKind Kind = Expr->getKind();

  if (Kind == MCExpr::Binary) {
    Expr = static_cast<const MCBinaryExpr*>(Expr)->getLHS();
    Kind = Expr->getKind();
  }

  assert (Kind == MCExpr::SymbolRef);

  Qpu::Fixups FixupKind = Qpu::Fixups(0);

  switch(cast<MCSymbolRefExpr>(Expr)->getKind()) {
  case MCSymbolRefExpr::VK_Qpu_GPREL:
    FixupKind = Qpu::fixup_Qpu_GPREL16;
    break;
  case MCSymbolRefExpr::VK_Qpu_GOT_CALL:
    FixupKind = Qpu::fixup_Qpu_CALL16;
    break;
  case MCSymbolRefExpr::VK_Qpu_GOT16:
    FixupKind = Qpu::fixup_Qpu_GOT_Global;
    break;
  case MCSymbolRefExpr::VK_Qpu_GOT:
    FixupKind = Qpu::fixup_Qpu_GOT_Local;
    break;
  case MCSymbolRefExpr::VK_Qpu_ABS_HI:
    FixupKind = Qpu::fixup_Qpu_HI16;
    break;
  case MCSymbolRefExpr::VK_Qpu_ABS_LO:
    FixupKind = Qpu::fixup_Qpu_LO16;
    break;
  case MCSymbolRefExpr::VK_Qpu_GOT_HI16:
    FixupKind = Qpu::fixup_Qpu_GOT_HI16;
    break;
  case MCSymbolRefExpr::VK_Qpu_GOT_LO16:
    FixupKind = Qpu::fixup_Qpu_GOT_LO16;
    break;
  default:
    break;
  } // switch

  Fixups.push_back(MCFixup::Create(0, MO.getExpr(), MCFixupKind(FixupKind)));

  // All of the information is in the fixup.
  return 0;
}

/// getMemEncoding - Return binary encoding of memory related operand.
/// If the offset operand requires relocation, record the relocation.
unsigned
QpuMCCodeEmitter::getMemEncoding(const MCInst &MI, unsigned OpNo,
                                  SmallVectorImpl<MCFixup> &Fixups) const {
  // Base register is encoded in bits 20-16, offset is encoded in bits 15-0.
  assert(MI.getOperand(OpNo).isReg());
  unsigned RegBits = getMachineOpValue(MI, MI.getOperand(OpNo),Fixups) << 16;
  unsigned OffBits = getMachineOpValue(MI, MI.getOperand(OpNo+1), Fixups);

  return (OffBits & 0xFFFF) | RegBits;
}

#include "QpuGenMCCodeEmitter.inc"

