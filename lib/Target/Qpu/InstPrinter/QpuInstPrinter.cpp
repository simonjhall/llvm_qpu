//===-- QpuInstPrinter.cpp - Convert Qpu MCInst to assembly syntax ------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This class prints an Qpu MCInst to a .s file.
//
//===----------------------------------------------------------------------===//

#define DEBUG_TYPE "asm-printer"
#include "QpuInstPrinter.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCSymbol.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/raw_ostream.h"
using namespace llvm;

#include "QpuGenAsmWriter.inc"

void QpuInstPrinter::printRegName(raw_ostream &OS, unsigned RegNo) const {
//- getRegisterName(RegNo) defined in QpuGenAsmWriter.inc which came from 
//   Qpu.td indicate.
  OS << StringRef(getRegisterName(RegNo)).lower();
}

void QpuInstPrinter::printInst(const MCInst *MI, raw_ostream &O,
                                StringRef Annot) {
//- printInstruction(MI, O) defined in QpuGenAsmWriter.inc which came from 
//   Qpu.td indicate.
  printInstruction(MI, O);
  printAnnotation(O, Annot);
}

static void printExpr(const MCExpr *Expr, raw_ostream &OS) {
  int Offset = 0;
  const MCSymbolRefExpr *SRE;

  if (const MCBinaryExpr *BE = dyn_cast<MCBinaryExpr>(Expr)) {
    SRE = dyn_cast<MCSymbolRefExpr>(BE->getLHS());
    const MCConstantExpr *CE = dyn_cast<MCConstantExpr>(BE->getRHS());
    assert(SRE && CE && "Binary expression must be sym+const.");
    Offset = CE->getValue();
  }
  else if (!(SRE = dyn_cast<MCSymbolRefExpr>(Expr)))
    assert(false && "Unexpected MCExpr type.");

  MCSymbolRefExpr::VariantKind Kind = SRE->getKind();

  switch (Kind) {
  default:                                 llvm_unreachable("Invalid kind!");
  case MCSymbolRefExpr::VK_None:           break;
// Qpu_GPREL is for llc -march=qpu -relocation-model=static
  case MCSymbolRefExpr::VK_Qpu_GPREL:     OS << "%gp_rel("; break;
  case MCSymbolRefExpr::VK_Qpu_GOT_CALL:  OS << "%call16("; break;
  case MCSymbolRefExpr::VK_Qpu_GOT16:     OS << "%got(";    break;
  case MCSymbolRefExpr::VK_Qpu_GOT:       OS << "%got(";    break;
//  case MCSymbolRefExpr::VK_Qpu_ABS_HI:    OS << "%hi(";     break;
//  case MCSymbolRefExpr::VK_Qpu_ABS_LO:    OS << "%lo(";     break;
  case MCSymbolRefExpr::VK_Qpu_ABS_HILO:    OS << "";     break;
  case MCSymbolRefExpr::VK_Qpu_GOT_HI16:  OS << "%got_hi("; break;
  case MCSymbolRefExpr::VK_Qpu_GOT_LO16:  OS << "%got_lo("; break;
  }

  OS << "#";
  OS << SRE->getSymbol();
  OS << "#";

  if (Offset) {
    if (Offset > 0)
      OS << '+';
    OS << Offset;
  }

  /*if ((Kind == MCSymbolRefExpr::VK_Qpu_GPOFF_HI) ||
      (Kind == MCSymbolRefExpr::VK_Qpu_GPOFF_LO))
    OS << ")))";
  else if (Kind != MCSymbolRefExpr::VK_None)
    OS << ')';*/
}

void QpuInstPrinter::printOperand(const MCInst *MI, unsigned OpNo,
                                   raw_ostream &O) {
  const MCOperand &Op = MI->getOperand(OpNo);
  if (Op.isReg()) {
    printRegName(O, Op.getReg());
    return;
  }

  if (Op.isImm()) {
	  uint16_t imm = Op.getImm();
    O << imm;

    if (Op.getImm() & 0x100000)
    	O << " vpm";

    return;
  }

  assert(Op.isExpr() && "unknown operand kind in printOperand");
  printExpr(Op.getExpr(), O);
}

void QpuInstPrinter::printUnsignedImm(const MCInst *MI, int opNum,
                                       raw_ostream &O) {
  const MCOperand &MO = MI->getOperand(opNum);
  if (MO.isImm())
    O << (unsigned short int)MO.getImm();
  else
    printOperand(MI, opNum, O);
}

void QpuInstPrinter::
printMemOperand(const MCInst *MI, int opNum, raw_ostream &O) {
  // Load/Store memory operands -- imm($reg)
  // If PIC target the target is loaded as the
  // pattern ld $t9,%call16($gp)
  printOperand(MI, opNum, O);
  O << ", ";
  printOperand(MI, opNum+1, O);
}

void QpuInstPrinter:: // lbd document - mark - printMemOperandEA
printMemOperandEA(const MCInst *MI, int opNum, raw_ostream &O) {
  // when using stack locations for not load/store instructions
  // print the same way as all normal 3 operand instructions.
  printOperand(MI, opNum, O);
  O << ", ";
  printOperand(MI, opNum+1, O);
  return;
}
