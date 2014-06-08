//===-- QpuMCInstLower.cpp - Convert Qpu MachineInstr to MCInst ---------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains code to lower Qpu MachineInstrs to their corresponding
// MCInst records.
//
//===----------------------------------------------------------------------===//

#include "QpuMCInstLower.h"
#include "QpuAsmPrinter.h"
#include "QpuInstrInfo.h"
#include "MCTargetDesc/QpuBaseInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/CodeGen/MachineOperand.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCInst.h"
#include "llvm/Target/Mangler.h"

using namespace llvm;

QpuMCInstLower::QpuMCInstLower(QpuAsmPrinter &asmprinter)
  : AsmPrinter(asmprinter) {}

void QpuMCInstLower::Initialize(MCContext* C) {
  Ctx = C;
} // lbd document - mark - Initialize

MCOperand QpuMCInstLower::LowerSymbolOperand(const MachineOperand &MO,
                                              MachineOperandType MOTy,
                                              unsigned Offset) const {
  MCSymbolRefExpr::VariantKind Kind;
  const MCSymbol *Symbol;

  switch(MO.getTargetFlags()) {
  default:                   llvm_unreachable("Invalid target flag!");
  case QpuII::MO_NO_FLAG:   Kind = MCSymbolRefExpr::VK_None; break;

// Qpu_GPREL is for llc -march=qpu -relocation-model=static -qpu-islinux-
//  format=false (global var in .sdata).
  case QpuII::MO_GPREL:     Kind = MCSymbolRefExpr::VK_Qpu_GPREL; break;

  case QpuII::MO_GOT_CALL:  Kind = MCSymbolRefExpr::VK_Qpu_GOT_CALL; break;
  case QpuII::MO_GOT16:     Kind = MCSymbolRefExpr::VK_Qpu_GOT16; break;
  case QpuII::MO_GOT:       Kind = MCSymbolRefExpr::VK_Qpu_GOT; break;
// ABS_HI and ABS_LO is for llc -march=qpu -relocation-model=static (global 
//  var in .data).
//  case QpuII::MO_ABS_HI:    Kind = MCSymbolRefExpr::VK_Qpu_ABS_HI; break;
//  case QpuII::MO_ABS_LO:    Kind = MCSymbolRefExpr::VK_Qpu_ABS_LO; break;
  case QpuII::MO_ABS_HILO:    Kind = MCSymbolRefExpr::VK_Qpu_ABS_HILO; break;
  case QpuII::MO_GOT_HI16:  Kind = MCSymbolRefExpr::VK_Qpu_GOT_HI16; break;
  case QpuII::MO_GOT_LO16:  Kind = MCSymbolRefExpr::VK_Qpu_GOT_LO16; break;
  }

  switch (MOTy) {
  case MachineOperand::MO_GlobalAddress:
    Symbol = AsmPrinter.getSymbol(MO.getGlobal());
    break;

  case MachineOperand::MO_MachineBasicBlock:
    Symbol = MO.getMBB()->getSymbol();
    break;

  case MachineOperand::MO_BlockAddress:
    Symbol = AsmPrinter.GetBlockAddressSymbol(MO.getBlockAddress());
    Offset += MO.getOffset();
    break;

  case MachineOperand::MO_ExternalSymbol:
    Symbol = AsmPrinter.GetExternalSymbolSymbol(MO.getSymbolName());
    Offset += MO.getOffset();
    break; // lbd document - mark - case MachineOperand::MO_ExternalSymbol:

  default:
    llvm_unreachable("<unknown operand type>");
  }

  const MCSymbolRefExpr *MCSym = MCSymbolRefExpr::Create(Symbol, Kind, *Ctx);

  if (!Offset)
    return MCOperand::CreateExpr(MCSym);

  // Assume offset is never negative.
  assert(Offset > 0);

  const MCConstantExpr *OffsetExpr =  MCConstantExpr::Create(Offset, *Ctx);
  const MCBinaryExpr *AddExpr = MCBinaryExpr::CreateAdd(MCSym, OffsetExpr, *Ctx);
  return MCOperand::CreateExpr(AddExpr);
} // lbd document - mark - LowerSymbolOperand

static void CreateMCInst(MCInst& Inst, unsigned Opc, const MCOperand& Opnd0,
                         const MCOperand& Opnd1,
                         const MCOperand& Opnd2 = MCOperand()) {
  Inst.setOpcode(Opc);
  Inst.addOperand(Opnd0);
  Inst.addOperand(Opnd1);
  if (Opnd2.isValid())
    Inst.addOperand(Opnd2);
}

// Lower ".cpload $reg" to
//  "lui   $gp, %hi(_gp_disp)"
//  "addiu $gp, $gp, %lo(_gp_disp)"
//  "addu  $gp, $gp, $t9"
void QpuMCInstLower::LowerCPLOAD(SmallVector<MCInst, 4>& MCInsts) {
  MCOperand GPReg = MCOperand::CreateReg(Qpu::GP);
  MCOperand T9Reg = MCOperand::CreateReg(Qpu::T9);
  StringRef SymName("_gp_disp");
  const MCSymbol *Sym = Ctx->GetOrCreateSymbol(SymName);
  const MCSymbolRefExpr *MCSym;

  /*MCSym = MCSymbolRefExpr::Create(Sym, MCSymbolRefExpr::VK_Qpu_ABS_HI, *Ctx);
  MCOperand SymHi = MCOperand::CreateExpr(MCSym);
  MCSym = MCSymbolRefExpr::Create(Sym, MCSymbolRefExpr::VK_Qpu_ABS_LO, *Ctx);
  MCOperand SymLo = MCOperand::CreateExpr(MCSym);

  MCInsts.resize(3);

  CreateMCInst(MCInsts[0], Qpu::LUi, GPReg, SymHi);
  CreateMCInst(MCInsts[1], Qpu::ADDiu, GPReg, GPReg, SymLo);
  CreateMCInst(MCInsts[2], Qpu::ADDu, GPReg, GPReg, T9Reg);*/

  MCSym = MCSymbolRefExpr::Create(Sym, MCSymbolRefExpr::VK_Qpu_ABS_HILO, *Ctx);
  MCOperand SymHiLo = MCOperand::CreateExpr(MCSym);

  MCInsts.resize(2);

  CreateMCInst(MCInsts[0], Qpu::LUi, GPReg, SymHiLo);
  CreateMCInst(MCInsts[2], Qpu::ADDu, GPReg, GPReg, T9Reg);
} // lbd document - mark - LowerCPLOAD

// Lower ".cprestore offset" to "st $gp, offset($sp)".
void QpuMCInstLower::LowerCPRESTORE(int64_t Offset,
                                     SmallVector<MCInst, 4>& MCInsts) {
  assert(isInt<32>(Offset) && (Offset >= 0) &&
         "Imm operand of .cprestore must be a non-negative 32-bit value.");

  MCOperand SPReg = MCOperand::CreateReg(Qpu::SP), BaseReg = SPReg;
  MCOperand GPReg = MCOperand::CreateReg(Qpu::GP);
  MCOperand ZEROReg = MCOperand::CreateReg(Qpu::ZERO_IN);

  if (!(Offset >= -16 && Offset <= 15)) {
    unsigned Hi = ((Offset + 0x8000) >> 16) & 0xffff;
    Offset &= 0xffff;
    MCOperand ATReg = MCOperand::CreateReg(Qpu::AT);
    BaseReg = ATReg;

    // lui   at,hi
    // add   at,at,sp
    MCInsts.resize(2);
    CreateMCInst(MCInsts[0], Qpu::LUi, ATReg, ZEROReg, MCOperand::CreateImm(Hi));
    CreateMCInst(MCInsts[1], Qpu::ADDu, ATReg, ATReg, SPReg);
  }

  MCInst St;
  CreateMCInst(St, Qpu::ST, GPReg, BaseReg, MCOperand::CreateImm(Offset));
  MCInsts.push_back(St);
} // lbd document - mark - LowerCPRESTORE

MCOperand QpuMCInstLower::LowerOperand(const MachineOperand& MO,
                                        unsigned offset) const {
  MachineOperandType MOTy = MO.getType();

  switch (MOTy) {
  default: llvm_unreachable("unknown operand type");
  case MachineOperand::MO_Register:
    // Ignore all implicit register operands.
    if (MO.isImplicit()) break;
    return MCOperand::CreateReg(MO.getReg());
  case MachineOperand::MO_Immediate:
    return MCOperand::CreateImm(MO.getImm() + offset);
  case MachineOperand::MO_MachineBasicBlock:
  case MachineOperand::MO_GlobalAddress:
  case MachineOperand::MO_ExternalSymbol:
  case MachineOperand::MO_BlockAddress:
    return LowerSymbolOperand(MO, MOTy, offset);
  case MachineOperand::MO_RegisterMask:
    break;
 }

  return MCOperand();
}

void QpuMCInstLower::Lower(const MachineInstr *MI, MCInst &OutMI) const {
  OutMI.setOpcode(MI->getOpcode());

  for (unsigned i = 0, e = MI->getNumOperands(); i != e; ++i) {
    const MachineOperand &MO = MI->getOperand(i);
    MCOperand MCOp = LowerOperand(MO);

    if (MCOp.isValid())
      OutMI.addOperand(MCOp);
  }
}

