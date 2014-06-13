//===- QpuInstrInfo.td - Target Description for Qpu Target -*- tablegen -*-=//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the Qpu implementation of the TargetInstrInfo class.
//
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
// Instruction format superclass
//===----------------------------------------------------------------------===//

include "QpuInstrFormats.td"

//===----------------------------------------------------------------------===//
// Qpu profiles and nodes
//===----------------------------------------------------------------------===//

//def SDT_QpuRet          : SDTypeProfile<0, 1, [SDTCisInt<0>]>;
def SDT_QpuDivRem       : SDTypeProfile<0, 2,
                                         [SDTCisInt<0>,
                                          SDTCisSameAs<0, 1>]>;

def SDT_QpuJmpLink      : SDTypeProfile<0, 1, [SDTCisVT<0, iPTR>]>;

def SDT_QpuCallSeqStart : SDCallSeqStart<[SDTCisVT<0, i32>]>;
def SDT_QpuCallSeqEnd   : SDCallSeqEnd<[SDTCisVT<0, i32>, SDTCisVT<1, i32>]>;

// Call
def QpuJmpLink : SDNode<"QpuISD::JmpLink",SDT_QpuJmpLink,
                         [SDNPHasChain, SDNPOutGlue, SDNPOptInGlue,
                          SDNPVariadic]>;

// Hi and Lo nodes are used to handle global addresses. Used on
// QpuISelLowering to lower stuff like GlobalAddress, ExternalSymbol
// static model. (nothing to do with Qpu Registers Hi and Lo)
//def QpuHi    : SDNode<"QpuISD::Hi", SDTIntUnaryOp>;
//def QpuLo    : SDNode<"QpuISD::Lo", SDTIntUnaryOp>;
def QpuHiLo    : SDNode<"QpuISD::HiLo", SDTIntUnaryOp>;
def QpuGPRel : SDNode<"QpuISD::GPRel", SDTIntUnaryOp>;

// Return
def QpuRet : SDNode<"QpuISD::Ret", SDTNone,
                     [SDNPHasChain, SDNPOptInGlue, SDNPVariadic]>;

// These are target-independent nodes, but have target-specific formats.
def callseq_start : SDNode<"ISD::CALLSEQ_START", SDT_QpuCallSeqStart,
                           [SDNPHasChain, SDNPOutGlue]>;
def callseq_end   : SDNode<"ISD::CALLSEQ_END", SDT_QpuCallSeqEnd,
                           [SDNPHasChain, SDNPOptInGlue, SDNPOutGlue]>;

// DivRem(u) nodes
def QpuDivRem    : SDNode<"QpuISD::DivRem", SDT_QpuDivRem,
                           [SDNPOutGlue]>;
def QpuDivRemU   : SDNode<"QpuISD::DivRemU", SDT_QpuDivRem,
                           [SDNPOutGlue]>;

//===----------------------------------------------------------------------===//
// Qpu Instruction Predicate Definitions.
//===----------------------------------------------------------------------===//
def HasCmp      :     Predicate<"Subtarget.hasCmp()">,
                      AssemblerPredicate<"FeatureCmp">;
/*def HasSlt      :     Predicate<"Subtarget.hasSlt()">,
                      AssemblerPredicate<"FeatureSlt">;*/
def IsQpu32I   :     Predicate<"Subtarget.hasQpu32I()">,
                      AssemblerPredicate<"FeatureQpu32I">;
def RelocPIC    :     Predicate<"TM.getRelocationModel() == Reloc::PIC_">;

//===----------------------------------------------------------------------===//
// Qpu Operand, Complex Patterns and Transformations Definitions.
//===----------------------------------------------------------------------===//
// Instruction operand types

// BEQ, BNE
def brtarget16    : Operand<OtherVT> {
  let EncoderMethod = "getBranch16TargetOpValue";
  let OperandType = "OPERAND_PCREL";
  let DecoderMethod = "DecodeBranch16Target";
}

// JEQ, JNE, ...
def brtarget24    : Operand<OtherVT> {
  let EncoderMethod = "getBranch24TargetOpValue";
  let OperandType = "OPERAND_PCREL";
  let DecoderMethod = "DecodeBranch24Target";
}

// JMP
def jmptarget    : Operand<OtherVT> {
  let EncoderMethod = "getJumpTargetOpValue";
  let OperandType = "OPERAND_PCREL";
  let DecoderMethod = "DecodeJumpRelativeTarget";
}

def calltarget  : Operand<iPTR> {
  let EncoderMethod = "getJumpTargetOpValue";
} // lbd document - mark - def calltarget


def shamt       : Operand<i32>;


// Address operand
def mem : Operand<i32> {
  let PrintMethod = "printMemOperand";
  let MIOperandInfo = (ops CPURegs, Operand<i32>);
  let EncoderMethod = "getMemEncoding";
}

def mem_ea : Operand<i32> {
  let PrintMethod = "printMemOperandEA";
  let MIOperandInfo = (ops CPURegs, Operand<i32>);
  let EncoderMethod = "getMemEncoding";
}

def immSmallInt : PatLeaf<(imm), [{ int64_t i = N->getSExtValue(); if (i >= -16 && i <= 15) return true; else return false; }]>;

// Immediate can be loaded with LUi (32-bit int with lower 16-bit cleared).
def immLow16Zero : PatLeaf<(imm), [{
  int64_t Val = N->getSExtValue();
  return isInt<32>(Val) && !(Val & 0xffff);
}]>;


// Qpu Address Mode! SDNode frameindex could possibily be a match
// since load and store instructions from stack used it.
def addr : ComplexPattern<iPTR, 2, "SelectAddr", [frameindex], [SDNPWantParent]>;

//===----------------------------------------------------------------------===//
// Pattern fragment for load/store
//===----------------------------------------------------------------------===//

class AlignedLoad<PatFrag Node> :
  PatFrag<(ops node:$ptr), (Node node:$ptr), [{
  LoadSDNode *LD = cast<LoadSDNode>(N);
  return LD->getMemoryVT().getSizeInBits()/8 <= LD->getAlignment();
}]>;

class AlignedStore<PatFrag Node> :
  PatFrag<(ops node:$val, node:$ptr), (Node node:$val, node:$ptr), [{
  StoreSDNode *SD = cast<StoreSDNode>(N);
  return SD->getMemoryVT().getSizeInBits()/8 <= SD->getAlignment();
}]>;

// Load/Store PatFrags.
def sextloadi16_a   : AlignedLoad<sextloadi16>;
def zextloadi16_a   : AlignedLoad<zextloadi16>;
def extloadi16_a    : AlignedLoad<extloadi16>;
def load_a          : AlignedLoad<load>;
def truncstorei16_a : AlignedStore<truncstorei16>;
def store_a         : AlignedStore<store>;

//===----------------------------------------------------------------------===//
// Instructions specific format
//===----------------------------------------------------------------------===//

// Arithmetic and logical instructions with 3 register operands.
class ArithLogicR<bits<8> op, string instr_asm, SDNode OpNode,
                  InstrItinClass itin, RegisterClass RD, RegisterClass RCa, RegisterClass RCb, bit isComm = 0>:
  FA<op, (outs RD:$ra), (ins RCa:$rb, RCb:$rc),
     !strconcat(instr_asm, "\t$ra, $rb, $rc"),
     [(set RD:$ra, (OpNode RCa:$rb, RCb:$rc))], itin> {
  let shamt = 0;
  let isCommutable = isComm;	// e.g. add rb rc =  add rc rb
  let isReMaterializable = 1;
}

class ArithLogicR1<bits<8> op, string instr_asm, SDNode OpNode,
                  InstrItinClass itin, RegisterClass RD, RegisterClass RC>:
  FA<op, (outs RD:$ra), (ins RC:$rb),
     !strconcat(instr_asm, "\t$ra, $rb, $rb"),
     [(set RD:$ra, (OpNode RC:$rb))], itin> {
  let shamt = 0;
  let isCommutable = 0;
  let isReMaterializable = 1;
}

class CmpInstr<bits<8> op, string instr_asm,
               InstrItinClass itin, RegisterClass RCa, RegisterClass RCb, RegisterClass RD,
               bit isComm = 0>:
  FA<op, (outs RD:$rc), (ins RCa:$ra, RCb:$rb),
     !strconcat(instr_asm, "\tacc5, $ra, $rb\n\tv8min acc5, acc5, acc5\n\tors wra_nop, acc5, acc5"), [], itin> {
  let rc = 0;
  let shamt = 0;
  let isCommutable = isComm;
  let Predicates = [HasCmp];
} // lbd document - mark - class CmpInstr

// Arithmetic and logical instructions with 2 register operands.
class ArithLogicI<bits<8> op, string instr_asm, SDNode OpNode,
                  Operand Od, PatLeaf imm_type, RegisterClass RD, RegisterClass RC> :
  FL<op, (outs RD:$ra), (ins RC:$rb, Od:$imm5),
     !strconcat(instr_asm, "\t$ra, $rb, $imm5"),
     [(set RD:$ra, (OpNode RC:$rb, imm_type:$imm5))], IIAlu> {
  let isReMaterializable = 1;
}

// Shifts
class shift_rotate_imm<bits<8> op, bits<4> isRotate, string instr_asm,
                       SDNode OpNode, PatFrag PF, Operand ImmOpnd,
                       RegisterClass RD, RegisterClass RC>:
  FA<op, (outs RD:$ra), (ins RC:$rb, ImmOpnd:$shamt),
     !strconcat(instr_asm, "\t$ra, $rb, $shamt"),
     [(set RD:$ra, (OpNode RC:$rb, PF:$shamt))], IIAlu> {
  let rc = 0;
  let shamt = shamt;
}

// 32-bit shift instructions.
class shift_rotate_imm32<bits<8> op, bits<4> isRotate, string instr_asm,
                         SDNode OpNode>:
  shift_rotate_imm<op, isRotate, instr_asm, OpNode, immSmallInt, shamt, GPRAccRA, GPRAccRB>;

class shift_rotate_reg<bits<8> op, bits<4> isRotate, string instr_asm,
                       SDNode OpNode, RegisterClass RD, RegisterClass RCa, RegisterClass RCb>:
  FA<op, (outs RD:$ra), (ins RCa:$rb, RCb:$rc),
     !strconcat(instr_asm, "\t$ra, $rb, $rc"),
     [(set RD:$ra, (OpNode RCa:$rb, RCb:$rc))], IIAlu> {
  let shamt = 0;
}

// Load Upper Imediate
class LoadUpper<bits<8> op, string instr_asm, RegisterClass RC, Operand Imm>:
  FL<op, (outs RC:$ra), (ins Imm:$imm5),
     !strconcat(instr_asm, "\t$ra, $imm5"), [], IIAlu> {
  let rb = 0;
  let neverHasSideEffects = 1;
  let isReMaterializable = 1;
} // lbd document - mark - class LoadUpper

class LoadUpperCC<bits<8> op, string instr_asm, RegisterClass RC, RegisterClass RSW, Operand Imm>:
  FL<op, (outs RC:$ra), (ins RSW:$sw, Imm:$imm5),
     !strconcat(instr_asm, "\t$ra, $imm5"), [], IIAlu> {
  let rb = 0;
  let neverHasSideEffects = 1;
  let isReMaterializable = 1;
} // lbd document - mark - class LoadUpper

class FMem<bits<8> op, dag outs, dag ins, string asmstr, list<dag> pattern,
          InstrItinClass itin>: FL<op, outs, ins, asmstr, pattern, itin> {
  bits<20> addr;
  let Inst{19-16} = addr{19-16};
  let Inst{15-0}  = addr{15-0};
  let DecoderMethod = "DecodeMem";
}

// Memory Load/Store
let canFoldAsLoad = 1 in
class LoadM<bits<8> op, string instr_asm, PatFrag OpNode, RegisterClass RC,
            Operand MemOpnd, bit Pseudo>:
  FMem<op, (outs RC:$ra), (ins MemOpnd:$addr),
     !strconcat(instr_asm, "\t$ra, $addr"),
     [(set RC:$ra, (OpNode addr:$addr))], IILoad> {
  let isPseudo = Pseudo;
}

class StoreM<bits<8> op, string instr_asm, PatFrag OpNode, RegisterClass RC,
             Operand MemOpnd, bit Pseudo>:
  FMem<op, (outs), (ins RC:$ra, MemOpnd:$addr),
     !strconcat(instr_asm, "\t$ra, $addr, 1"),
     [(OpNode RC:$ra, addr:$addr)], IIStore> {
  let isPseudo = Pseudo;
}

// 32-bit load.
multiclass LoadM32<bits<8> op, string instr_asm, PatFrag OpNode,
                   bit Pseudo = 0> {
	def #NAME# : LoadM<op, instr_asm, OpNode, GPRAccRARB, mem, Pseudo>;
	def #NAME#_F32 : LoadM<op, instr_asm, OpNode, GPRAccRARB_FP, mem, Pseudo>;
//	def #NAME# : LoadM<op, instr_asm, OpNode, LdAddrDest, mem, Pseudo>;
//	def #NAME# : LoadM<op, instr_asm, OpNode, LdDest, mem, Pseudo>;
}

// 32-bit store.
multiclass StoreM32<bits<8> op, string instr_asm, PatFrag OpNode,
                    bit Pseudo = 0> {
	def #NAME# : StoreM<op, instr_asm, OpNode, GPRAccRARB, mem, Pseudo>;
	def #NAME#_F32 : StoreM<op, instr_asm, OpNode, GPRAccRARB_FP, mem, Pseudo>;
}

// Conditional Branch, e.g. JEQ brtarget24
class CBranch24<bits<8> op, string instr_asm, RegisterClass RC,
                   list<Register> UseRegs>:
  FJ<op, (outs), (ins RC:$ra, brtarget24:$addr),
             !strconcat(instr_asm, "\twra_nop, wrb_nop, $addr"),
             [], IIBranch>, Requires<[HasCmp]> {
  let isBranch = 1;
  let isTerminator = 1;
  let hasDelaySlot = 1;
  let neverHasSideEffects = 1;
//  let Predicates = [HasCmp]; // same effect as Requires
}

// Conditional Branch, e.g. BEQ $r1, $r2, brtarget16
/*class CBranch16<bits<8> op, string instr_asm, PatFrag cond_op, RegisterClass RC>:
  FL<op, (outs), (ins RC:$ra, RC:$rb, brtarget16:$imm5),
             !strconcat(instr_asm, "\t$ra, $rb, $imm5"),
             [(brcond (i32 (cond_op RC:$ra, RC:$rb)), bb:$imm5)], IIBranch>, 
             Requires<[HasSlt]> {
  let isBranch = 1;
  let isTerminator = 1;
  let hasDelaySlot = 1;
  let Defs = [AT];
}

// SetCC
class SetCC_R<bits<8> op, string instr_asm, PatFrag cond_op,
              RegisterClass RC>:
  FA<op, (outs GPROut:$ra), (ins RC:$rb, RC:$rc),
     !strconcat(instr_asm, "\t$ra, $rb, $rc"),
     [(set GPROut:$ra, (cond_op RC:$rb, RC:$rc))],
     IIAlu>, Requires<[HasSlt]> {
  let shamt = 0;
}

class SetCC_I<bits<8> op, string instr_asm, PatFrag cond_op, Operand Od,
              PatLeaf imm_type, RegisterClass RC>:
  FL<op, (outs GPROut:$ra), (ins RC:$rb, Od:$imm5),
     !strconcat(instr_asm, "\t$ra, $rb, $imm5"),
     [(set GPROut:$ra, (cond_op RC:$rb, imm_type:$imm5))],
     IIAlu>, Requires<[HasSlt]> {
}*/

// Unconditional branch, such as JMP
class UncondBranch<bits<8> op, string instr_asm>:
  FJ<op, (outs), (ins jmptarget:$addr),
             !strconcat(instr_asm, "\twra_nop, wrb_nop, $addr"), [(br bb:$addr)], IIBranch> {
  let isBranch = 1;
  let isTerminator = 1;
  let isBarrier = 1;
  let hasDelaySlot = 1;
} // lbd document - mark - class UncondBranch

let isBranch=1, isTerminator=1, isBarrier=1, imm5=0, hasDelaySlot = 1,
    isIndirectBranch = 1 in
class JumpFR<bits<8> op, string instr_asm, RegisterClass RC>:
  FL<op, (outs), (ins RC:$ra),
     !strconcat(instr_asm, "\twra_nop, wrb_nop, $ra"), [(brind RC:$ra)], IIBranch> {
  let rb = 0;
  let imm5 = 0;
}

// Return instruction
class RetBase<RegisterClass RC>: JumpFR<0x3c, "bla", RC> {
  let isReturn = 1;
  let isCodeGenOnly = 1;
  let hasCtrlDep = 1;
  let hasExtraSrcRegAllocReq = 1;
}
// Jump and Link (Call)
let isCall=1, hasDelaySlot=1 in {
  class JumpLink<bits<8> op, string instr_asm>:
    FJ<op, (outs), (ins calltarget:$target, variable_ops),
       !strconcat(instr_asm, "\tra31, wrb_nop, $target"), [(QpuJmpLink imm:$target)],
       IIBranch> {
       }

  class JumpLinkReg<bits<8> op, string instr_asm,
                    RegisterClass RC>:
    FA<op, (outs), (ins RC:$rb, variable_ops),
       !strconcat(instr_asm, "\tra31, wrb_nop, $rb"), [(QpuJmpLink RC:$rb)], IIBranch> {
    let rc = 0;
    let ra = 14;
    let shamt = 0;
  }
}

// Mul, Div
class Mult<bits<8> op, string instr_asm, InstrItinClass itin,
           RegisterClass RC, list<Register> DefRegs>:
  FL<op, (outs), (ins RC:$ra, RC:$rb),
     !strconcat(instr_asm, "\t$ra, $rb"), [], itin> {
  let imm5 = 0;
  let isCommutable = 1;
  let Defs = DefRegs;
  let neverHasSideEffects = 1;
}

/*class Mult32<bits<8> op, string instr_asm, InstrItinClass itin>:
  Mult<op, instr_asm, itin, CPURegs, [HI, LO]>;*/

class Div<SDNode opNode, bits<8> op, string instr_asm, InstrItinClass itin,
          RegisterClass RC, list<Register> DefRegs>:
  FL<op, (outs), (ins RC:$ra, RC:$rb),
     !strconcat(instr_asm, "\t$ra, $rb"),
     [(opNode RC:$ra, RC:$rb)], itin> {
  let imm5 = 0;
  let Defs = DefRegs;
}

class MoveFromClassToClass<bits<8> op, string instr_asm, RegisterClass RD, RegisterClass RS>:
  FL<op, (outs RD:$rd), (ins RS:$rs),
     !strconcat(instr_asm, "\t$rd, $rs"), [], IIHiLo> {
  let rb = 0;
  let imm5 = 0;
  let neverHasSideEffects = 1;
}

class MoveFromClassToClassDup<bits<8> op, string instr_asm, RegisterClass RD, RegisterClass RS>:
  FL<op, (outs RD:$rd), (ins RS:$rs),
     !strconcat(instr_asm, "\t$rd, $rs, $rs"), [], IIHiLo> {
  let rb = 0;
  let imm5 = 0;
  let neverHasSideEffects = 1;
}

class MoveFromClassToClassDupNopDest<bits<8> op, string instr_asm, RegisterClass RD, RegisterClass RS>:
  FL<op, (outs RD:$rd), (ins RS:$rs),
     !strconcat(instr_asm, "\twra_nop, $rs, $rs"), [], IIHiLo> {
  let rb = 0;
  let imm5 = 0;
  let neverHasSideEffects = 1;
}

/*class Div32<SDNode opNode, bits<8> op, string instr_asm, InstrItinClass itin>:
  Div<opNode, op, instr_asm, itin, CPURegs, [HI, LO]>;*/

// Move from Lo/Hi
/*class MoveFromLOHI<bits<8> op, string instr_asm, RegisterClass RC,
                   list<Register> UseRegs>:
  FL<op, (outs RC:$ra), (ins),
     !strconcat(instr_asm, "\t$ra"), [], IIHiLo> {
  let rb = 0;
  let imm5 = 0;
  let Uses = UseRegs;
  let neverHasSideEffects = 1;
}

// Move to Lo/Hi
class MoveToLOHI<bits<8> op, string instr_asm, RegisterClass RC,
                 list<Register> DefRegs>:
  FL<op, (outs), (ins RC:$ra),
     !strconcat(instr_asm, "\t$ra"), [], IIHiLo> {
  let rb = 0;
  let imm5 = 0;
  let Defs = DefRegs;
  let neverHasSideEffects = 1;
}*/

// Move from SW
/*class MoveFromSW<bits<8> op, string instr_asm, RegisterClass RC,
                   list<Register> UseRegs>:
  FL<op, (outs RC:$ra), (ins),
     !strconcat(instr_asm, "\t$ra"), [], IIAlu> {
  let rb = 0;
  let imm5 = 0;
  let Uses = UseRegs;
  let neverHasSideEffects = 1;
}

// Move to SW
class MoveToSW<bits<8> op, string instr_asm, RegisterClass RC,
                 list<Register> DefRegs>:
  FL<op, (outs), (ins RC:$ra),
     !strconcat(instr_asm, "\t$ra"), [], IIAlu> {
  let rb = 0;
  let imm5 = 0;
  let Defs = DefRegs;
  let neverHasSideEffects = 1;
}*/

class EffectiveAddress<string instr_asm, RegisterClass RC, Operand Mem> :
  FMem<0x09, (outs RC:$ra), (ins Mem:$addr),
     instr_asm, [(set RC:$ra, addr:$addr)], IIAlu>;

//===----------------------------------------------------------------------===//
// Pseudo instructions
//===----------------------------------------------------------------------===//

// As stack alignment is always done with addiu, we need a 16-bit immediate
let Defs = [SP], Uses = [SP] in {
def ADJCALLSTACKDOWN : QpuPseudo<(outs), (ins Operand<i32>:$amt),
                                  "!ADJCALLSTACKDOWN $amt",
                                  [(callseq_start timm:$amt)]>;
def ADJCALLSTACKUP   : QpuPseudo<(outs), (ins Operand<i32>:$amt1, Operand<i32>:$amt2),
                                  "!ADJCALLSTACKUP $amt1",
                                  [(callseq_end timm:$amt1, timm:$amt2)]>;
}

// When handling PIC code the assembler needs .cpload and .cprestore
// directives. If the real instructions corresponding these directives
// are used, we have the same behavior, but get also a bunch of warnings
// from the assembler.
let neverHasSideEffects = 1 in
def CPRESTORE : QpuPseudo<(outs), (ins i32imm:$loc, CPURegs:$gp),
                           ".cprestore\t$loc", []>;

//===----------------------------------------------------------------------===//
// Instruction definition
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
// Qpu Instructions
//===----------------------------------------------------------------------===//

/// Load and Store Instructions
///  aligned
defm LD     : LoadM32<0x01,  "load_word",  load_a>;
defm ST     : StoreM32<0x02, "store_word",  store_a>;
defm LB     : LoadM32<0x03, "load_byte_signed",  sextloadi8>;
defm LBu    : LoadM32<0x04, "load_byte_unsigned", zextloadi8>;
defm SB     : StoreM32<0x05, "store_byte", truncstorei8>;
defm LH     : LoadM32<0x06, "load_half_signed",  sextloadi16_a>;
defm LHu    : LoadM32<0x07, "load_half_unsigned", zextloadi16_a>;
defm SH     : StoreM32<0x08, "store_half", truncstorei16_a>;

/// Arithmetic Instructions (ALU Immediate)
// IR "add" defined in include/llvm/Target/TargetSelectionDAG.td, line 315 (def add).
def ADDiu   : ArithLogicI<0x09, "add", add, Operand<i32>, immSmallInt, GPRAccRARB, GPRAccRA>;
def SUBiu   : ArithLogicI<0x09, "sub", sub, Operand<i32>, immSmallInt, GPRAccRARB, GPRAccRA>;
def ANDi    : ArithLogicI<0x0c, "and", and, Operand<i32>, immSmallInt, GPRAccRARB, GPRAccRA>;
def ORi     : ArithLogicI<0x0d, "or", or, Operand<i32>, immSmallInt, GPRAccRARB, GPRAccRA>;
def ORi_ns     : ArithLogicI<0x0d, "orns", or, Operand<i32>, immSmallInt, GPRAccRARB, GPRAccRA>;
def ORi_nc     : ArithLogicI<0x0d, "ornc", or, Operand<i32>, immSmallInt, GPRAccRARB, GPRAccRA>;
def ORi_zs     : ArithLogicI<0x0d, "orzs", or, Operand<i32>, immSmallInt, GPRAccRARB, GPRAccRA>;
def ORi_zc     : ArithLogicI<0x0d, "orzc", or, Operand<i32>, immSmallInt, GPRAccRARB, GPRAccRA>;
def XORi    : ArithLogicI<0x0e, "xor", xor, Operand<i32>, immSmallInt, GPRAccRARB, GPRAccRA>;
def SHLi    : ArithLogicI<0x0e, "shl", shl, Operand<i32>, immSmallInt, GPRAccRARB, GPRAccRA>;
def SRAi    : ArithLogicI<0x0e, "asr", sra, Operand<i32>, immSmallInt, GPRAccRARB, GPRAccRA>;
def SRLi    : ArithLogicI<0x0e, "shr", srl, Operand<i32>, immSmallInt, GPRAccRARB, GPRAccRA>;
def RORi    : ArithLogicI<0x0e, "ror", rotr, Operand<i32>, immSmallInt, GPRAccRARB, GPRAccRA>;
def LUi     : LoadUpper<0x0f, "il", GPRAccRARB, Operand<i32>>;

def LUi_al  : LoadUpperCC<0x0f, "il", GPRAccRARB, CPURegs, Operand<i32>>;
def LUi_zs  : LoadUpperCC<0x0f, "ilzs", GPRAccRARB, CPURegs, Operand<i32>>;
def LUi_zc  : LoadUpperCC<0x0f, "ilzc", GPRAccRARB, CPURegs, Operand<i32>>;
def LUi_ns  : LoadUpperCC<0x0f, "ilns", GPRAccRARB, CPURegs, Operand<i32>>;
def LUi_nc  : LoadUpperCC<0x0f, "ilnc", GPRAccRARB, CPURegs, Operand<i32>>;
def LUi_cs  : LoadUpperCC<0x0f, "ilcs", GPRAccRARB, CPURegs, Operand<i32>>;
def LUi_cc  : LoadUpperCC<0x0f, "ilcc", GPRAccRARB, CPURegs, Operand<i32>>;

/// Arithmetic Instructions (3-Operand, R-Type)
def CMP_i32     : CmpInstr<0x10, "sub", IIAlu, GPRAccRA, GPRAccRB, SR, 0>;
def CMP_f32     : CmpInstr<0x10, "fsub", IIAlu, GPRAccRA_FP, GPRAccRB_FP, SR, 0>;
def ADDu     : ArithLogicR<0x13, "add", add, IIAlu, GPRAccRARB, GPRAccRA, GPRAccRB, 1>;
//def ADDe     : ArithLogicR<0x13, "adde", adde, IIAlu, GPRAccRARB, GPRAccRA, GPRAccRB, 1>;
def SUBu     : ArithLogicR<0x14, "sub", sub, IIAlu, GPRAccRARB, GPRAccRA, GPRAccRB>;
def MUL     : ArithLogicR<0x17, "mul24", mul, IIImul, GPRAccRARB, GPRAccRA, GPRAccRB, 1>;
def AND     : ArithLogicR<0x18, "and", and, IIAlu, GPRAccRARB, GPRAccRA, GPRAccRB, 1>;
def OR      : ArithLogicR<0x19, "or", or, IIAlu, GPRAccRARB, GPRAccRA, GPRAccRB, 1>;
def XOR     : ArithLogicR<0x1a, "xor", xor, IIAlu, GPRAccRARB, GPRAccRA, GPRAccRB, 1>;

def SHL     : ArithLogicR<0x19, "shl", shl, IIAlu, GPRAccRARB, GPRAccRA, GPRAccRB, 0>;
def SRA     : ArithLogicR<0x1a, "asr", sra, IIAlu, GPRAccRARB, GPRAccRA, GPRAccRB, 0>;
def SRL     : ArithLogicR<0x1a, "shr", srl, IIAlu, GPRAccRARB, GPRAccRA, GPRAccRB, 0>;
def ROR     : ArithLogicR<0x1a, "ror", rotr, IIAlu, GPRAccRARB, GPRAccRA, GPRAccRB, 0>;

def MOVE    : MoveFromClassToClass<0x46, "mov", GPRAccRARB, GPRAccRARB>;
def BROADCAST    : MoveFromClassToClassDup<0x46, "v8min", GPRAcc5, GPRAccRARB>;
def SET_FLAGS    : MoveFromClassToClassDupNopDest<0x46, "ors", SR, GPRAcc5>;

def FADD     : ArithLogicR<0x13, "fadd", fadd, IIAlu, GPRAccRARB_FP, GPRAccRA_FP, GPRAccRB_FP, 1>;
def FMUL     : ArithLogicR<0x13, "fmul", fmul, IIAlu, GPRAccRARB_FP, GPRAccRA_FP, GPRAccRB_FP, 1>;
def FSUB     : ArithLogicR<0x13, "fsub", fsub, IIAlu, GPRAccRARB_FP, GPRAccRA_FP, GPRAccRB_FP, 0>;
def FTOI     : ArithLogicR1<0x13, "ftoi", fp_to_sint, IIAlu, GPRAccRARB, GPRAccRARB_FP>;
def ITOF     : ArithLogicR1<0x13, "itof", sint_to_fp, IIAlu, GPRAccRARB_FP, GPRAccRARB>;

//def MOVE     : ArithLogicR<0x1a, "move", xor, IIAlu, LdAddrDest, 1>;

//def MFHI    : MoveFromLOHI<0x46, "mfhi", CPURegs, [VPM_LD_ADDR]>;
//def MFLO    : MoveFromLOHI<0x47, "mflo", CPURegs, [VPM_ST_ADDR]>;

/// Shift Instructions

/*def SHL     : shift_rotate_imm32<0x1e, 0x00, "shl", shl>;
def SRA     : shift_rotate_imm32<0x1d, 0x00, "asr", sra>;*/

// sra is IR node for ashr llvm IR instruction of .bc
/*def ROL     : shift_rotate_imm32<0x1b, 0x01, "rol", rotl>;
def ROR     : shift_rotate_imm32<0x1c, 0x01, "ror", rotr>;
def SRA     : shift_rotate_imm32<0x1d, 0x00, "sra", sra>;
def SHL     : shift_rotate_imm32<0x1e, 0x00, "shl", shl>;
// srl is IR node for lshr llvm IR instruction of .bc
def SHR     : shift_rotate_imm32<0x1f, 0x00, "shr", srl>;
def SRAV    : shift_rotate_reg<0x20, 0x00, "srav", sra, CPURegs>;
def SHLV    : shift_rotate_reg<0x21, 0x00, "shlv", shl, CPURegs>;
def SHRV    : shift_rotate_reg<0x22, 0x00, "shrv", srl, CPURegs>;*/

/*def SLTi    : SetCC_I<0x26, "slti", setlt, Operand<i32>, immSmallInt, CPURegs>;
def SLTiu   : SetCC_I<0x27, "sltiu", setult, Operand<i32>, immSmallInt, CPURegs>;
def SLT     : SetCC_R<0x28, "slt", setlt, CPURegs>;
def SLTu    : SetCC_R<0x29, "sltu", setult, CPURegs>;*/

/// Jump and Branch Instructions
/*def JEQ     : CBranch24<0x30, "blaallzs", SR, [SW]>;
def JNE     : CBranch24<0x31, "blaallzc", SR, [SW]>;
def JLT     : CBranch24<0x32, "blaallns", SR, [SW]>;
def JGT     : CBranch24<0x33, "blaallnc", SR, [SW]>;
def JLE     : CBranch24<0x34, "jle_unk", SR, [SW]>;
def JGE     : CBranch24<0x35, "blaallcc", SR, [SW]>;*/

/*def BEQ     : CBranch16<0x37, "beq", seteq, GPROut>;
def BNE     : CBranch16<0x38, "bne", setne, GPROut>;*/

def JMP     : UncondBranch<0x36, "bla">;

/// Jump & link and Return Instructions
def SWI     : JumpLink<0x3a, "swi">;
def JSUB    : JumpLink<0x3b, "bla">;

let isReturn=1, isTerminator=1, hasDelaySlot=1, isBarrier=1, hasCtrlDep=1 in
  def RetLR : QpuPseudo<(outs), (ins), "", [(QpuRet)]>;

def RET     : RetBase<GPRAccRARB>;
//def IRET    : JumpFR<0x3d, "iret", GPROut>;

def JALR    : JumpLinkReg<0x3e, "bla", GPRAccRARB>;

/// Multiply and Divide Instructions.
//def MULT    : Mult32<0x41, "mult", IIImul>;
//def MULTu   : Mult32<0x42, "multu", IIImul>;
//def SDIV    : Div32<QpuDivRem, 0x43, "div", IIIdiv>;
//def UDIV    : Div32<QpuDivRemU, 0x44, "divu", IIIdiv>;

/*def MFHI    : MoveFromLOHI<0x46, "mfhi", CPURegs, [HI]>;
def MFLO    : MoveFromLOHI<0x47, "mflo", CPURegs, [LO]>;
def MTHI    : MoveToLOHI<0x48, "mthi", CPURegs, [HI]>;
def MTLO    : MoveToLOHI<0x49, "mtlo", CPURegs, [LO]>;
def MFSW    : MoveFromSW<0x50, "mfsw", CPURegs, [SW]>;
def MTSW    : MoveToSW<0x51, "mtsw", CPURegs, [SW]>;*/


class CmpInstrSub<bits<8> op, string instr_asm,
               InstrItinClass itin, RegisterClass RCa, RegisterClass RCb, RegisterClass RD>:
  FA<op, (outs RD:$rc), (ins RCa:$ra, RCb:$rb),
     !strconcat(instr_asm, "\twra_nop, $ra, $rb"), [], itin> {
  let rc = 0;
  let shamt = 0;
  let isCommutable = 0;
  let Predicates = [HasCmp];
} // lbd document - mark - class CmpInstr

def CMP_INTERNAL_i32     : CmpInstrSub<0x10, "subs", IIAlu, GPRAccRA, GPRAccRB, SR>;
def CMP_INTERNAL_f32     : CmpInstrSub<0x10, "fsubs", IIAlu, GPRAccRA, GPRAccRB, SR>;

def JZERO     : CBranch24<0x35, "blaallzs", SR, [SW]>;
def JONE     : CBranch24<0x35, "blaallzc", SR, [SW]>;

def JNE     : CBranch24<0x35, "blaallzs", SR, [SW]>;
def JEQ     : CBranch24<0x35, "blaallzc", SR, [SW]>;

/* one == negative
 * branch if one for lt
 * branch if zero for ge
 */
multiclass BranchComparisons<RegisterClass RC, ValueType type, Instruction CMP_INTERNAL> {
	def : Pat<(brcond (type (setge RC:$lhs, RC:$rhs)), bb:$dst),
			  (JZERO (SET_FLAGS
					  (BROADCAST
						  (LUi_ns
							  (LUi_al
									  (CMP_INTERNAL
											  RC:$lhs,
											  RC:$rhs),
									  0),
								1)
						  )
					  ), bb:$dst)>;
	def : Pat<(brcond (type (setuge RC:$lhs, RC:$rhs)), bb:$dst),
			  (JZERO (SET_FLAGS
					  (BROADCAST
						  (LUi_ns
							  (LUi_al
									  (CMP_INTERNAL
											  RC:$lhs,
											  RC:$rhs),
									  0),
								1)
						  )
					  ), bb:$dst)>;

	def : Pat<(brcond (type (setlt RC:$lhs, RC:$rhs)), bb:$dst),
			  (JONE (SET_FLAGS
					  (BROADCAST
						  (LUi_ns
							  (LUi_al
									  (CMP_INTERNAL
											  RC:$lhs,
											  RC:$rhs),
									  0),
								1)
						  )
					  ), bb:$dst)>;
	def : Pat<(brcond (type (setult RC:$lhs, RC:$rhs)), bb:$dst),
			  (JONE (SET_FLAGS
					  (BROADCAST
						  (LUi_ns
							  (LUi_al
									  (CMP_INTERNAL
											  RC:$lhs,
											  RC:$rhs),
									  0),
								1)
						  )
					  ), bb:$dst)>;

	//swapped inputs around
	def : Pat<(brcond (type (setle RC:$rhs, RC:$lhs)), bb:$dst),
			  (JZERO (SET_FLAGS
					  (BROADCAST
						  (LUi_ns
							  (LUi_al
									  (CMP_INTERNAL
											  RC:$lhs,
											  RC:$rhs),
									  0),
								1)
						  )
					  ), bb:$dst)>;
	def : Pat<(brcond (type (setule RC:$rhs, RC:$lhs)), bb:$dst),
			  (JZERO (SET_FLAGS
					  (BROADCAST
						  (LUi_ns
							  (LUi_al
									  (CMP_INTERNAL
											  RC:$lhs,
											  RC:$rhs),
									  0),
								1)
						  )
					  ), bb:$dst)>;

	def : Pat<(brcond (type (setgt RC:$rhs, RC:$lhs)), bb:$dst),
			  (JONE (SET_FLAGS
					  (BROADCAST
						  (LUi_ns
							  (LUi_al
									  (CMP_INTERNAL
											  RC:$lhs,
											  RC:$rhs),
									  0),
								1)
						  )
					  ), bb:$dst)>;
	def : Pat<(brcond (type (setugt RC:$rhs, RC:$lhs)), bb:$dst),
			  (JONE (SET_FLAGS
					  (BROADCAST
						  (LUi_ns
							  (LUi_al
									  (CMP_INTERNAL
											  RC:$lhs,
											  RC:$rhs),
									  0),
								1)
						  )
					  ), bb:$dst)>;

	/*
	 * eq, 1 = eq (zero), 0 = ne (non-zero)
	 */
	def : Pat<(brcond (type (seteq RC:$lhs, RC:$rhs)), bb:$dst),
			  (JONE (SET_FLAGS
					  (BROADCAST
						  (LUi_zs
							  (LUi_al
									  (CMP_INTERNAL
											  RC:$lhs,
											  RC:$rhs),
									  0),
								1)
						  )
					  ), bb:$dst)>;

	def : Pat<(brcond (type (setne RC:$lhs, RC:$rhs)), bb:$dst),
			  (JZERO (SET_FLAGS
					  (BROADCAST
						  (LUi_zs
							  (LUi_al
									  (CMP_INTERNAL
											  RC:$lhs,
											  RC:$rhs),
									  0),
								1)
						  )
					  ), bb:$dst)>;
}

/*def : Pat<(setne GPRAccRARB_FP:$lhs, GPRAccRARB_FP:$rhs),
			  (FSUB
											  GPRAccRARB_FP:$lhs,
											  GPRAccRARB_FP:$rhs)>;
*/
defm : BranchComparisons<GPRAccRARB, i32, CMP_INTERNAL_i32>;
//defm : BranchComparisons<GPRAccRARB_FP, f32, CMP_INTERNAL_f32>;

/*def : Pat<(brcond (i32 (seteq CPURegs:$lhs, CPURegs:$rhs)), bb:$dst),
          (JEQ
								  (CMP_INTERNAL
										  CPURegs:$lhs,
										  CPURegs:$rhs),
								  bb:$dst)>;

def : Pat<(brcond (i32 (setne CPURegs:$lhs, CPURegs:$rhs)), bb:$dst),
			(JNE
								  (CMP_INTERNAL
										  CPURegs:$lhs,
										  CPURegs:$rhs),
								  bb:$dst)>;*/

//(LUi_zc (LUi_zs (CMP RC:$lhs, RC:$rhs), 1), 0)>;

// brcond for cmp instruction
//multiclass BrcondPatsCmp<RegisterClass RC, Instruction JEQOp, Instruction JNEOp,
//Instruction JLTOp, Instruction JGTOp, Instruction JLEOp, Instruction JGEOp,
//Instruction CMPOp, Register ZEROReg> {
//def : Pat<(brcond (i32 (seteq RC:$lhs, RC:$rhs)), bb:$dst),
//          (JEQOp (CMPOp RC:$lhs, RC:$rhs), bb:$dst)>;
//def : Pat<(brcond (i32 (setueq RC:$lhs, RC:$rhs)), bb:$dst),
//          (JEQOp (CMPOp RC:$lhs, RC:$rhs), bb:$dst)>;
//def : Pat<(brcond (i32 (setne RC:$lhs, RC:$rhs)), bb:$dst),
//          (JNEOp (CMPOp RC:$lhs, RC:$rhs), bb:$dst)>;
//def : Pat<(brcond (i32 (setune RC:$lhs, RC:$rhs)), bb:$dst),
//          (JNEOp (CMPOp RC:$lhs, RC:$rhs), bb:$dst)>;
//def : Pat<(brcond (i32 (setlt RC:$lhs, RC:$rhs)), bb:$dst),
//          (JLTOp (CMPOp RC:$lhs, RC:$rhs), bb:$dst)>;
//def : Pat<(brcond (i32 (setult RC:$lhs, RC:$rhs)), bb:$dst),
//          (JLTOp (CMPOp RC:$lhs, RC:$rhs), bb:$dst)>;
//def : Pat<(brcond (i32 (setgt RC:$lhs, RC:$rhs)), bb:$dst),
//          (JGTOp (CMPOp RC:$lhs, RC:$rhs), bb:$dst)>;
//def : Pat<(brcond (i32 (setugt RC:$lhs, RC:$rhs)), bb:$dst),
//          (JGTOp (CMPOp RC:$lhs, RC:$rhs), bb:$dst)>;
//def : Pat<(brcond (i32 (setle RC:$lhs, RC:$rhs)), bb:$dst),
//          (JLEOp (CMPOp RC:$lhs, RC:$rhs), bb:$dst)>;
//def : Pat<(brcond (i32 (setule RC:$lhs, RC:$rhs)), bb:$dst),
//          (JLEOp (CMPOp RC:$lhs, RC:$rhs), bb:$dst)>;
//def : Pat<(brcond (i32 (setge RC:$lhs, RC:$rhs)), bb:$dst),
//          (JGEOp (CMPOp RC:$lhs, RC:$rhs), bb:$dst)>;
//def : Pat<(brcond (i32 (setuge RC:$lhs, RC:$rhs)), bb:$dst),
//          (JGEOp (CMPOp RC:$lhs, RC:$rhs), bb:$dst)>;
//
//def : Pat<(brcond RC:$cond, bb:$dst),
//          (JNEOp (CMPOp RC:$cond, ZEROReg), bb:$dst)>;
//}
//let Predicates = [HasCmp] in {
//defm : BrcondPatsCmp<CPURegs, JEQ, JNE, JLT, JGT, JLE, JGE, CMP, ZERO_IN>;
//}

/// No operation
let addr=0 in
  def NOP   : FJ<0, (outs), (ins), "nop", [], IIAlu>;

// FrameIndexes are legalized when they are operands from load/store
// instructions. The same not happens for stack address copies, so an
// add op with mem ComplexPattern is used and the stack address copy
// can be matched. It's similar to Sparc LEA_ADDRi
def LEA_ADDiu : EffectiveAddress<"lea\t$ra, $addr", CPURegs, mem_ea> {
  let isCodeGenOnly = 1;
} // lbd document - mark - def LEA_ADDiu

//===----------------------------------------------------------------------===//
//  Arbitrary patterns that map to one or more instructions
//===----------------------------------------------------------------------===//

// Small immediates
def : Pat<(i32 immSmallInt:$in),
          (LUi imm:$in)>;
/*def : Pat<(i32 immSmallInt:$in),
          (ORi ZERO_IN, imm:$in)>;*/

// Arbitrary immediates
def : Pat<(i32 imm:$imm),
          (LUi imm:$imm)>;

// Carry patterns
def : Pat<(subc CPURegs:$lhs, CPURegs:$rhs),
          (SUBu CPURegs:$lhs, CPURegs:$rhs)>;
def : Pat<(addc CPURegs:$lhs, CPURegs:$rhs),
          (ADDu CPURegs:$lhs, CPURegs:$rhs)>;
def : Pat<(addc  CPURegs:$src, immSmallInt:$imm),
          (ADDiu CPURegs:$src, imm:$imm)>;

def : Pat<(QpuJmpLink (i32 tglobaladdr:$dst)),
          (JSUB tglobaladdr:$dst)>;
def : Pat<(QpuJmpLink (i32 texternalsym:$dst)),
          (JSUB texternalsym:$dst)>;

// hi/lo relocs
//def : Pat<(QpuHi tglobaladdr:$in), (SHL (LUi tglobaladdr:$in), 16)>;
def : Pat<(QpuHiLo tglobaladdr:$in), (LUi tglobaladdr:$in)>;

//def : Pat<(add CPURegs:$hi, (QpuLo tglobaladdr:$lo)),
//          (ADDiu CPURegs:$hi, tglobaladdr:$lo)>;

// gp_rel relocs
def : Pat<(add CPURegs:$gp, (QpuGPRel tglobaladdr:$in)),
          (ADDiu CPURegs:$gp, tglobaladdr:$in)>;

/*def : Pat<(not CPURegs:$in),
// 1: in == 0; 0: in != 0
          (XORi CPURegs:$in, 1)>;*/

def : Pat<(i32 (extloadi1  addr:$src)), (LBu addr:$src)>;
def : Pat<(i32 (extloadi8  addr:$src)), (LBu addr:$src)>;
def : Pat<(i32 (extloadi16_a addr:$src)), (LHu addr:$src)>;

// brcond patterns

// brcond for cmp instruction
//multiclass BrcondPatsCmp<RegisterClass RC, Instruction JEQOp, Instruction JNEOp,
//Instruction JLTOp, Instruction JGTOp, Instruction JLEOp, Instruction JGEOp,
//Instruction CMPOp, Register ZEROReg> {
//def : Pat<(brcond (i32 (seteq RC:$lhs, RC:$rhs)), bb:$dst),
//          (JEQOp (CMPOp RC:$lhs, RC:$rhs), bb:$dst)>;
//def : Pat<(brcond (i32 (setueq RC:$lhs, RC:$rhs)), bb:$dst),
//          (JEQOp (CMPOp RC:$lhs, RC:$rhs), bb:$dst)>;
//def : Pat<(brcond (i32 (setne RC:$lhs, RC:$rhs)), bb:$dst),
//          (JNEOp (CMPOp RC:$lhs, RC:$rhs), bb:$dst)>;
//def : Pat<(brcond (i32 (setune RC:$lhs, RC:$rhs)), bb:$dst),
//          (JNEOp (CMPOp RC:$lhs, RC:$rhs), bb:$dst)>;
//def : Pat<(brcond (i32 (setlt RC:$lhs, RC:$rhs)), bb:$dst),
//          (JLTOp (CMPOp RC:$lhs, RC:$rhs), bb:$dst)>;
//def : Pat<(brcond (i32 (setult RC:$lhs, RC:$rhs)), bb:$dst),
//          (JLTOp (CMPOp RC:$lhs, RC:$rhs), bb:$dst)>;
//def : Pat<(brcond (i32 (setgt RC:$lhs, RC:$rhs)), bb:$dst),
//          (JGTOp (CMPOp RC:$lhs, RC:$rhs), bb:$dst)>;
//def : Pat<(brcond (i32 (setugt RC:$lhs, RC:$rhs)), bb:$dst),
//          (JGTOp (CMPOp RC:$lhs, RC:$rhs), bb:$dst)>;
//def : Pat<(brcond (i32 (setle RC:$lhs, RC:$rhs)), bb:$dst),
//          (JLEOp (CMPOp RC:$lhs, RC:$rhs), bb:$dst)>;
//def : Pat<(brcond (i32 (setule RC:$lhs, RC:$rhs)), bb:$dst),
//          (JLEOp (CMPOp RC:$lhs, RC:$rhs), bb:$dst)>;
//def : Pat<(brcond (i32 (setge RC:$lhs, RC:$rhs)), bb:$dst),
//          (JGEOp (CMPOp RC:$lhs, RC:$rhs), bb:$dst)>;
//def : Pat<(brcond (i32 (setuge RC:$lhs, RC:$rhs)), bb:$dst),
//          (JGEOp (CMPOp RC:$lhs, RC:$rhs), bb:$dst)>;
//
//def : Pat<(brcond RC:$cond, bb:$dst),
//          (JNEOp (CMPOp RC:$cond, ZEROReg), bb:$dst)>;
//}

// brcond for slt instruction
/*multiclass BrcondPatsSlt<RegisterClass RC, Instruction BEQOp, Instruction BNEOp,
                      Instruction SLTOp, Instruction SLTuOp, Instruction SLTiOp,
                      Instruction SLTiuOp, Register ZEROReg> {
def : Pat<(brcond (i32 (setne RC:$lhs, 0)), bb:$dst),
              (BNEOp RC:$lhs, ZEROReg, bb:$dst)>;
def : Pat<(brcond (i32 (seteq RC:$lhs, 0)), bb:$dst),
              (BEQOp RC:$lhs, ZEROReg, bb:$dst)>;

def : Pat<(brcond (i32 (seteq RC:$lhs, RC:$rhs)), bb:$dst),
              (BEQOp RC:$lhs, RC:$rhs, bb:$dst)>;
def : Pat<(brcond (i32 (setueq RC:$lhs, RC:$rhs)), bb:$dst),
              (BEQOp RC:$lhs, RC:$rhs, bb:$dst)>;
def : Pat<(brcond (i32 (setne RC:$lhs, RC:$rhs)), bb:$dst),
              (BNEOp RC:$lhs, RC:$rhs, bb:$dst)>;
def : Pat<(brcond (i32 (setune RC:$lhs, RC:$rhs)), bb:$dst),
              (BNEOp RC:$lhs, RC:$rhs, bb:$dst)>;
def : Pat<(brcond (i32 (setlt RC:$lhs, RC:$rhs)), bb:$dst),
              (BNE (SLTOp RC:$lhs, RC:$rhs), ZERO, bb:$dst)>;
def : Pat<(brcond (i32 (setult RC:$lhs, RC:$rhs)), bb:$dst),
              (BNE (SLTuOp RC:$lhs, RC:$rhs), ZERO, bb:$dst)>;
def : Pat<(brcond (i32 (setgt RC:$lhs, RC:$rhs)), bb:$dst),
              (BEQ (SLTOp RC:$lhs, RC:$rhs), ZERO, bb:$dst)>;
def : Pat<(brcond (i32 (setugt RC:$lhs, RC:$rhs)), bb:$dst),
              (BEQ (SLTuOp RC:$lhs, RC:$rhs), ZERO, bb:$dst)>;
def : Pat<(brcond (i32 (setle RC:$lhs, RC:$rhs)), bb:$dst),
              (BEQ (SLTOp RC:$rhs, RC:$lhs), ZERO, bb:$dst)>;
def : Pat<(brcond (i32 (setule RC:$lhs, RC:$rhs)), bb:$dst),
              (BEQ (SLTuOp RC:$rhs, RC:$lhs), ZERO, bb:$dst)>;
def : Pat<(brcond (i32 (setge RC:$lhs, RC:$rhs)), bb:$dst),
              (BEQ (SLTOp RC:$lhs, RC:$rhs), ZERO, bb:$dst)>;
def : Pat<(brcond (i32 (setuge RC:$lhs, RC:$rhs)), bb:$dst),
              (BEQ (SLTuOp RC:$lhs, RC:$rhs), ZERO, bb:$dst)>;

def : Pat<(brcond RC:$cond, bb:$dst),
              (BNEOp RC:$cond, ZEROReg, bb:$dst)>;
}*/

/*let Predicates = [HasSlt] in {
defm : BrcondPatsSlt<CPURegs, BEQ, BNE, SLT, SLTu, SLTi, SLTiu, ZERO>;
}
*/
//let Predicates = [HasCmp] in {
//defm : BrcondPatsCmp<CPURegs, JEQ, JNE, JLT, JGT, JLE, JGE, CMP, ZERO_IN>;
//}

// setcc patterns

//http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.dui0552a/BABEHFEF.html

// setcc for cmp instruction

multiclass SeteqPatsCmp<RegisterClass RC, Instruction CMP> {
	//z set
// a == b
  /*def : Pat<(seteq RC:$lhs, RC:$rhs),
            (SHR (ANDi (CMP RC:$lhs, RC:$rhs), 2), 1)>;*/
	def : Pat<(seteq RC:$lhs, RC:$rhs),
            (LUi_zc (LUi_zs (CMP RC:$lhs, RC:$rhs), 1), 0)>;
// a != b
	//z clear
  /*def : Pat<(setne RC:$lhs, RC:$rhs),
            (XORi (SHR (ANDi (CMP RC:$lhs, RC:$rhs), 2), 1), 1)>;*/
	def : Pat<(setne RC:$lhs, RC:$rhs),
            (LUi_zc (LUi_zs (CMP RC:$lhs, RC:$rhs), 0), 1)>;
}

// a < b
multiclass SetltPatsCmp<RegisterClass RC, Instruction CMP> {
  def : Pat<(setlt RC:$lhs, RC:$rhs),
            (ORi_ns (LUi_al (CMP RC:$lhs, RC:$rhs), 0), 1)>;
// if qpu  `define N    `SW[31]  instead of `SW[0] // Negative flag, then need
// 2 more instructions as follows,
//          (XORi (ANDi (SHR (CMP RC:$lhs, RC:$rhs), (LUi 0x8000), 31), 1), 1)>;
  /*def : Pat<(setult RC:$lhs, RC:$rhs),
            (ORi_ns (LUi_al (CMP RC:$lhs, RC:$rhs), 0), 1)>;*/

  def : Pat<(setult RC:$lhs, RC:$rhs),
            (ORi_ns (LUi_al (CMP RC:$lhs, RC:$rhs), 0), 1)>;
}

// a <= b
multiclass SetlePatsCmp<RegisterClass RC, Instruction CMP> {
  def : Pat<(setle RC:$lhs, RC:$rhs),
// a <= b is equal to (XORi (b < a), 1)
            //(XORi (ANDi (CMP RC:$rhs, RC:$lhs), 1), 1)>;
		  (ORi_zs (ORi_ns (LUi_al (CMP RC:$lhs, RC:$rhs), 0), 1) ,1)>;
  /*def : Pat<(setule RC:$lhs, RC:$rhs),
            (XORi (ANDi (CMP RC:$rhs, RC:$lhs), 1), 1)>;*/

  def : Pat<(setule RC:$lhs, RC:$rhs),
            (ORi_zs (ORi_ns (LUi_al (CMP RC:$lhs, RC:$rhs), 0), 1) ,1)>;
}

// a > b
multiclass SetgtPatsCmp<RegisterClass RC, Instruction CMP> {
  def : Pat<(setgt RC:$lhs, RC:$rhs),
// a > b is equal to b < a is equal to setlt(b, a)
            //(ANDi (CMP RC:$rhs, RC:$lhs), 1)>;
		  (ORi_ns (LUi_al (CMP RC:$rhs, RC:$lhs), 0), 1)>;
  /*def : Pat<(setugt RC:$lhs, RC:$rhs),
            (ANDi (CMP RC:$rhs, RC:$lhs), 1)>;*/

  def : Pat<(setugt RC:$lhs, RC:$rhs),
            (ORi_ns (LUi_al (CMP RC:$rhs, RC:$lhs), 0), 1)>;
}

// a >= b
multiclass SetgePatsCmp<RegisterClass RC, Instruction CMP> {
  def : Pat<(setge RC:$lhs, RC:$rhs),
// a >= b is equal to b <= a
            //(XORi (ANDi (CMP RC:$lhs, RC:$rhs), 1), 1)>;
		  (ORi_zs (ORi_ns (LUi_al (CMP RC:$rhs, RC:$lhs), 0), 1), 1)>;
  /*def : Pat<(setuge RC:$lhs, RC:$rhs),
            (XORi (ANDi (CMP RC:$lhs, RC:$rhs), 1), 1)>;*/

  def : Pat<(setuge RC:$lhs, RC:$rhs),
            (ORi_zs (ORi_ns (LUi_al (CMP RC:$rhs, RC:$lhs), 0), 1), 1)>;
}

/*// setcc for slt instruction
multiclass SeteqPatsSlt<RegisterClass RC, Instruction SLTiuOp, Instruction XOROp,
                     Instruction SLTuOp, Register ZEROReg> {
// a == b
  def : Pat<(seteq RC:$lhs, RC:$rhs),
                (SLTiuOp (XOROp RC:$lhs, RC:$rhs), 1)>;
// a != b
  def : Pat<(setne RC:$lhs, RC:$rhs),
                (SLTuOp ZEROReg, (XOROp RC:$lhs, RC:$rhs))>;
}

// a <= b
multiclass SetlePatsSlt<RegisterClass RC, Instruction SLTOp, Instruction SLTuOp> {
  def : Pat<(setle RC:$lhs, RC:$rhs),
// a <= b is equal to (XORi (b < a), 1)
                (XORi (SLTOp RC:$rhs, RC:$lhs), 1)>;
  def : Pat<(setule RC:$lhs, RC:$rhs),
                (XORi (SLTuOp RC:$rhs, RC:$lhs), 1)>;
}

// a > b
multiclass SetgtPatsSlt<RegisterClass RC, Instruction SLTOp, Instruction SLTuOp> {
  def : Pat<(setgt RC:$lhs, RC:$rhs),
// a > b is equal to b < a is equal to setlt(b, a)
                (SLTOp RC:$rhs, RC:$lhs)>;
  def : Pat<(setugt RC:$lhs, RC:$rhs),
                (SLTuOp RC:$rhs, RC:$lhs)>;
}

// a >= b
multiclass SetgePatsSlt<RegisterClass RC, Instruction SLTOp, Instruction SLTuOp> {
  def : Pat<(setge RC:$lhs, RC:$rhs),
// a >= b is equal to b <= a
                (XORi (SLTOp RC:$lhs, RC:$rhs), 1)>;
  def : Pat<(setuge RC:$lhs, RC:$rhs),
                (XORi (SLTuOp RC:$lhs, RC:$rhs), 1)>;
}

multiclass SetgeImmPatsSlt<RegisterClass RC, Instruction SLTiOp,
                        Instruction SLTiuOp> {
  def : Pat<(setge RC:$lhs, immSmallInt:$rhs),
                (XORi (SLTiOp RC:$lhs, immSmallInt:$rhs), 1)>;
  def : Pat<(setuge RC:$lhs, immSmallInt:$rhs),
                (XORi (SLTiuOp RC:$lhs, immSmallInt:$rhs), 1)>;
}

let Predicates = [HasSlt] in {
defm : SeteqPatsSlt<CPURegs, SLTiu, XOR, SLTu, ZERO>;
defm : SetlePatsSlt<CPURegs, SLT, SLTu>;
defm : SetgtPatsSlt<CPURegs, SLT, SLTu>;
defm : SetgePatsSlt<CPURegs, SLT, SLTu>;
defm : SetgeImmPatsSlt<CPURegs, SLTi, SLTiu>;
}*/

let Predicates = [HasCmp] in {
defm : SeteqPatsCmp<GPRAccRARB, CMP_i32>;
defm : SeteqPatsCmp<GPRAccRARB_FP, CMP_f32>;

defm : SetltPatsCmp<GPRAccRARB, CMP_i32>;
defm : SetltPatsCmp<GPRAccRARB_FP, CMP_f32>;

defm : SetlePatsCmp<GPRAccRARB, CMP_i32>;
defm : SetlePatsCmp<GPRAccRARB_FP, CMP_f32>;

defm : SetgtPatsCmp<GPRAccRARB, CMP_i32>;
defm : SetgtPatsCmp<GPRAccRARB_FP, CMP_f32>;

defm : SetgePatsCmp<GPRAccRARB, CMP_i32>;
defm : SetgePatsCmp<GPRAccRARB_FP, CMP_f32>;
}

include "QpuCondMov.td"

