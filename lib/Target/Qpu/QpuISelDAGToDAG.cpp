//===-- QpuISelDAGToDAG.cpp - A Dag to Dag Inst Selector for Qpu --------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file defines an instruction selector for the QPU target.
//
//===----------------------------------------------------------------------===//

#define DEBUG_TYPE "qpu-isel"
#include "Qpu.h"
#include "QpuMachineFunction.h"
#include "QpuRegisterInfo.h"
#include "QpuSubtarget.h"
#include "QpuTargetMachine.h"
#include "MCTargetDesc/QpuBaseInfo.h"
#include "llvm/IR/GlobalValue.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/Support/CFG.h"
#include "llvm/IR/Type.h"
#include "llvm/CodeGen/MachineConstantPool.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/SelectionDAGISel.h"
#include "llvm/CodeGen/SelectionDAGNodes.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/raw_ostream.h"
using namespace llvm;

//===----------------------------------------------------------------------===//
// Instruction Selector Implementation
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
// QpuDAGToDAGISel - QPU specific code to select QPU machine
// instructions for SelectionDAG operations.
//===----------------------------------------------------------------------===//
namespace {

class QpuDAGToDAGISel : public SelectionDAGISel {

  /// TM - Keep a reference to QpuTargetMachine.
  QpuTargetMachine &TM;

  /// Subtarget - Keep a pointer to the QpuSubtarget around so that we can
  /// make the right decision when generating code for different targets.
  const QpuSubtarget &Subtarget;

public:
  explicit QpuDAGToDAGISel(QpuTargetMachine &tm) :
  SelectionDAGISel(tm),
  TM(tm), Subtarget(tm.getSubtarget<QpuSubtarget>()) {}

  // Pass Name
  virtual const char *getPassName() const {
    return "QPU DAG->DAG Pattern Instruction Selection";
  }

  virtual bool runOnMachineFunction(MachineFunction &MF);

private:
  // Include the pieces autogenerated from the target description.
  #include "QpuGenDAGISel.inc"

  /// getTargetMachine - Return a reference to the TargetMachine, casted
  /// to the target-specific type.
  const QpuTargetMachine &getTargetMachine() {
    return static_cast<const QpuTargetMachine &>(TM);
  }

  /// getInstrInfo - Return a reference to the TargetInstrInfo, casted
  /// to the target-specific type.
  const QpuInstrInfo *getInstrInfo() {
    return getTargetMachine().getInstrInfo();
  }

  SDNode *getGlobalBaseReg();

  std::pair<SDNode*, SDNode*> SelectMULT(SDNode *N, unsigned Opc, SDLoc DL,
                                         EVT Ty, bool HasLo, bool HasHi);

  SDNode *Select(SDNode *N);
  // Complex Pattern.
  bool SelectAddr(SDNode *Parent, SDValue N, SDValue &Base, SDValue &Offset);
  // getImm - Return a target constant with the specified value.
  inline SDValue getImm(const SDNode *Node, unsigned Imm) {
    return CurDAG->getTargetConstant(Imm, Node->getValueType(0));
  }
  void InitGlobalBaseReg(MachineFunction &MF);
};
}

bool QpuDAGToDAGISel::runOnMachineFunction(MachineFunction &MF) {
  bool Ret = SelectionDAGISel::runOnMachineFunction(MF);

  return Ret;
}

/// getGlobalBaseReg - Output the instructions required to put the
/// GOT address into a register.
SDNode *QpuDAGToDAGISel::getGlobalBaseReg() {
  unsigned GlobalBaseReg = MF->getInfo<QpuFunctionInfo>()->getGlobalBaseReg();
  return CurDAG->getRegister(GlobalBaseReg, getTargetLowering()->getPointerTy()).getNode();
} // lbd document - mark - getGlobalBaseReg()

/// ComplexPattern used on QpuInstrInfo
/// Used on Qpu Load/Store instructions
bool QpuDAGToDAGISel::
SelectAddr(SDNode *Parent, SDValue Addr, SDValue &Base, SDValue &Offset) {
  EVT ValTy = Addr.getValueType();

  // If Parent is an unaligned f32 load or store, select a (base + index)
  // floating point load/store instruction (luxc1 or suxc1).
  const LSBaseSDNode* LS = 0;

  if (Parent && (LS = dyn_cast<LSBaseSDNode>(Parent))) {
    EVT VT = LS->getMemoryVT();

    if (VT.getSizeInBits() / 8 > LS->getAlignment()) {
      assert(getTargetLowering()->allowsUnalignedMemoryAccesses(VT) &&
             "Unaligned loads/stores not supported for this type.");
      if (VT == MVT::f32)
        return false;
    }
  }

  bool isVpm = false;

  if (Parent)
  {
	unsigned AddrSpace = cast<MemSDNode>(Parent)->getPointerInfo().getAddrSpace();

	if (AddrSpace == 1)		//VPM
		isVpm = true;
  }

  // if Address is FI, get the TargetFrameIndex.
  if (FrameIndexSDNode *FIN = dyn_cast<FrameIndexSDNode>(Addr)) {
    Base   = CurDAG->getTargetFrameIndex(FIN->getIndex(), ValTy);
    Offset = CurDAG->getTargetConstant(0, ValTy);
    assert(!isVpm);
    return true;
  }

  // on PIC code Load GA
  if (Addr.getOpcode() == QpuISD::Wrapper) {
    Base   = Addr.getOperand(0);
    Offset = Addr.getOperand(1);
    assert(!isVpm);
    return true;
  }

  if (TM.getRelocationModel() != Reloc::PIC_) {
    if ((Addr.getOpcode() == ISD::TargetExternalSymbol ||
        Addr.getOpcode() == ISD::TargetGlobalAddress))
      return false;
  }
  
  // Addresses of the form FI+const or FI|const
  if (CurDAG->isBaseWithConstantOffset(Addr)) {
    ConstantSDNode *CN = dyn_cast<ConstantSDNode>(Addr.getOperand(1));
    if (CN->getSExtValue() <= 15 && CN->getSExtValue() >= -16) {

      // If the first operand is a FI, get the TargetFI Node
      if (FrameIndexSDNode *FIN = dyn_cast<FrameIndexSDNode>
                                  (Addr.getOperand(0)))
        Base = CurDAG->getTargetFrameIndex(FIN->getIndex(), ValTy);
      else
        Base = Addr.getOperand(0);

      Offset = CurDAG->getTargetConstant((CN->getZExtValue() & 0xffff) | (isVpm ? 0x100000 : 0), ValTy);
      return true;
    }
  } // lbd document - mark - if (CurDAG->isBaseWithConstantOffset(Addr))

  Base   = Addr;
  Offset = CurDAG->getTargetConstant(isVpm ? 0x100000 : 0, ValTy);
  return true;
} // lbd document - mark - SelectAddr

/// Select multiply instructions.
/*std::pair<SDNode*, SDNode*>
QpuDAGToDAGISel::SelectMULT(SDNode *N, unsigned Opc, SDLoc DL, EVT Ty,
                             bool HasLo, bool HasHi) {
  SDNode *Lo = 0, *Hi = 0;
  SDNode *Mul = CurDAG->getMachineNode(Opc, DL, MVT::Glue, N->getOperand(0),
                                       N->getOperand(1));
  SDValue InFlag = SDValue(Mul, 0);

  if (HasLo) {
    Lo = CurDAG->getMachineNode(Qpu::MFLO, DL,
                                Ty, MVT::Glue, InFlag);
    InFlag = SDValue(Lo, 1);
  }
  if (HasHi)
    Hi = CurDAG->getMachineNode(Qpu::MFHI, DL,
                                Ty, InFlag);

  return std::make_pair(Lo, Hi);
} // lbd document - mark - SelectMULT*/

/// Select instructions not customized! Used for
/// expanded, promoted and normal instructions
SDNode* QpuDAGToDAGISel::Select(SDNode *Node) {
  unsigned Opcode = Node->getOpcode();
  SDLoc DL(Node);

  // Dump information about the Node being selected
  DEBUG(errs() << "Selecting: "; Node->dump(CurDAG); errs() << "\n");

  // If we have a custom node, we already have selected!
  if (Node->isMachineOpcode()) {
    DEBUG(errs() << "== "; Node->dump(CurDAG); errs() << "\n");
    return NULL;
  }

  ///
  // Instruction Selection not handled by the auto-generated
  // tablegen selection should be handled here.
  ///
  //EVT NodeTy = Node->getValueType(0);
  //unsigned MultOpc;

  switch(Opcode) {
  default: break;

  case ISD::SUBE:
  case ISD::ADDE: {
    SDValue InFlag = Node->getOperand(2), CmpLHS;
    unsigned Opc = InFlag.getOpcode(); (void)Opc;
    assert(((Opc == ISD::ADDC || Opc == ISD::ADDE) ||
            (Opc == ISD::SUBC || Opc == ISD::SUBE)) &&
           "(ADD|SUB)E flag operand must come from (ADD|SUB)C/E insn");

    unsigned MOp;
    if (Opcode == ISD::ADDE) {
      CmpLHS = InFlag.getValue(0);
      MOp = Qpu::ADDu;
    } else {
      CmpLHS = InFlag.getOperand(0);
      MOp = Qpu::SUBu;
    }

    SDValue Ops[] = { CmpLHS, InFlag.getOperand(1) };

    SDValue LHS = Node->getOperand(0);
    SDValue RHS = Node->getOperand(1);

    EVT VT = LHS.getValueType();
    const QpuTargetMachine &TM = getTargetMachine();
    const QpuSubtarget &Subtarget = TM.getSubtarget<QpuSubtarget>();
    SDNode *Carry;
    {
      SDNode *StatusWord = CurDAG->getMachineNode(Qpu::CMP_INTERNAL, DL, VT, Ops);
      SDValue Constant0 = CurDAG->getTargetConstant(0, VT);
      SDValue Constant1 = CurDAG->getTargetConstant(1, VT);
      Carry = CurDAG->getMachineNode(Qpu::LUi_al, DL, VT,
                                             SDValue(StatusWord,0), Constant0);
      Carry = CurDAG->getMachineNode(Qpu::LUi_cs, DL, VT,
                                             SDValue(Carry,0), Constant1);
    }
    SDNode *AddCarry = CurDAG->getMachineNode(Qpu::ADDu, DL, VT,
                                              SDValue(Carry,0), RHS);

    return CurDAG->SelectNodeTo(Node, MOp, VT, MVT::Glue,
                                LHS, SDValue(AddCarry,0));
  }

  /// Mul with two results
  /*case ISD::SMUL_LOHI:
  case ISD::UMUL_LOHI: {
    if (NodeTy == MVT::i32)
      MultOpc = (Opcode == ISD::UMUL_LOHI ? Qpu::MULTu : Qpu::MULT);

    std::pair<SDNode*, SDNode*> LoHi = SelectMULT(Node, MultOpc, DL, NodeTy,
                                                  true, true);

    if (!SDValue(Node, 0).use_empty())
      ReplaceUses(SDValue(Node, 0), SDValue(LoHi.first, 0));

    if (!SDValue(Node, 1).use_empty())
      ReplaceUses(SDValue(Node, 1), SDValue(LoHi.second, 0));

    return NULL;
  }

  case ISD::MULHS:
  case ISD::MULHU: {
    MultOpc = (Opcode == ISD::MULHU ? Qpu::MULTu : Qpu::MULT);
    return SelectMULT(Node, MultOpc, DL, NodeTy, false, true).second;
  }*/

  // Get target GOT address.
  // For global variables as follows,
  //- @gI = global i32 100, align 4
  //- %2 = load i32* @gI, align 4
  // =>
  //- .cpload	$gp
  //- ld	$2, %got(gI)($gp)
  case ISD::GLOBAL_OFFSET_TABLE:
    return getGlobalBaseReg();

  case ISD::Constant: {
    const ConstantSDNode *CN = dyn_cast<ConstantSDNode>(Node);
    unsigned Size = CN->getValueSizeInBits(0);

    if (Size == 32)
      break;
  }
  }

  // Select the default instruction
  SDNode *ResNode = SelectCode(Node);

  DEBUG(errs() << "=> ");
  if (ResNode == NULL || ResNode == Node)
    DEBUG(Node->dump(CurDAG));
  else
    DEBUG(ResNode->dump(CurDAG));
  DEBUG(errs() << "\n");
  return ResNode;
}

/// createQpuISelDag - This pass converts a legalized DAG into a
/// QPU-specific DAG, ready for instruction scheduling.
FunctionPass *llvm::createQpuISelDag(QpuTargetMachine &TM) {
  return new QpuDAGToDAGISel(TM);
}
