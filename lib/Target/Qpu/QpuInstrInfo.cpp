//===-- QpuInstrInfo.cpp - Qpu Instruction Information ------------------===//
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

#include "QpuInstrInfo.h"
#include "QpuTargetMachine.h"
#include "QpuMachineFunction.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#define GET_INSTRINFO_CTOR_DTOR
#include "QpuGenInstrInfo.inc"

using namespace llvm;

QpuInstrInfo::QpuInstrInfo(QpuTargetMachine &tm)
  : 
    QpuGenInstrInfo(Qpu::ADJCALLSTACKDOWN, Qpu::ADJCALLSTACKUP),
    TM(tm),
    RI(*TM.getSubtargetImpl(), *this) {}

const QpuRegisterInfo &QpuInstrInfo::getRegisterInfo() const {
  return RI;
} // lbd document - mark - getRegisterInfo()

void QpuInstrInfo::
copyPhysReg(MachineBasicBlock &MBB,
            MachineBasicBlock::iterator I, DebugLoc DL,
            unsigned DestReg, unsigned SrcReg,
            bool KillSrc) const {
  unsigned Opc = 0, ZeroReg = 0;

//  if (Qpu::CPURegsRegClass.contains(DestReg)) { // Copy to CPU Reg.
//    if (Qpu::CPURegsRegClass.contains(SrcReg))
//      Opc = Qpu::ADDu, ZeroReg = Qpu::ZERO_IN;
//    /*else if (SrcReg == Qpu::HI)
//      Opc = Qpu::MFHI, SrcReg = 0;
//    else if (SrcReg == Qpu::LO)
//      Opc = Qpu::MFLO, SrcReg = 0;*/
//  }
//  else if (Qpu::CPURegsRegClass.contains(SrcReg)) { // Copy from CPU Reg.
//   /* if (DestReg == Qpu::HI)
//      Opc = Qpu::MTHI, DestReg = 0;
//    else if (DestReg == Qpu::LO)
//      Opc = Qpu::MTLO, DestReg = 0;*/
//  }
//  assert(Opc && "Cannot copy registers");
//
//  MachineInstrBuilder MIB = BuildMI(MBB, I, DL, get(Opc));
//
//  if (DestReg)
//    MIB.addReg(DestReg, RegState::Define);
//
//  if (ZeroReg)
//    MIB.addReg(ZeroReg);
//
//  if (SrcReg)
//    MIB.addReg(SrcReg, getKillRegState(KillSrc));

  //load
  /*if (Qpu::CPURegsRegClass.contains(DestReg) && Qpu::LdAddrDestRegClass.contains(SrcReg))
  {
	  BuildMI(MBB, I, DL, get(Qpu::MOVE), Qpu::ZERO_OUT).addReg(Qpu::VPM_LD_WAIT).addReg(SrcReg, KillSrc ? RegState::ImplicitKill : RegState::Implicit).addReg(Qpu::VPM_DAT_WRA, RegState::Implicit).addReg(Qpu::VPM_ST_ADDR, RegState::ImplicitDefine);
	  BuildMI(MBB, I, DL, get(Qpu::LUi), Qpu::VPM_LD_SETUP).addReg(Qpu::VPM_LD_WAIT, RegState::ImplicitKill).addImm(1234);
	  BuildMI(MBB, I, DL, get(Qpu::MOVE), DestReg).addReg(Qpu::VPM_DAT_RDA).addReg(Qpu::VPM_LD_SETUP, RegState::ImplicitKill);
  }
  //store
  else if (Qpu::CPURegsRegClass.contains(SrcReg) && Qpu::StAddrDestRegClass.contains(DestReg))
  {
	  BuildMI(MBB, I, DL, get(Qpu::OR), Qpu::VPM_DAT_WRA).addReg(SrcReg, KillSrc ? RegState::Kill : RegState::Define).addReg(Qpu::VPM_ST_WAIT).addReg(Qpu::VPM_DAT_RDA, RegState::Implicit).addReg(Qpu::VPM_LD_ADDR, RegState::ImplicitDefine);
	  BuildMI(MBB, I, DL, get(Qpu::LUi), Qpu::VPM_ST_SETUP).addReg(Qpu::VPM_DAT_WRA, RegState::ImplicitKill).addImm(1234).addReg(DestReg, RegState::Implicit).addReg(Qpu::VPM_ST_WAIT, RegState::ImplicitKill);
  }
  //load->store
  else if (Qpu::StAddrDestRegClass.contains(DestReg) && Qpu::LdAddrDestRegClass.contains(SrcReg))
  {
	  BuildMI(MBB, I, DL, get(Qpu::MOVE), Qpu::ZERO_OUT).addReg(Qpu::VPM_LD_WAIT).addReg(SrcReg, KillSrc ? RegState::ImplicitKill : RegState::Implicit).addReg(Qpu::VPM_DAT_WRA, RegState::Implicit).addReg(Qpu::VPM_ST_ADDR, RegState::ImplicitDefine);
	  BuildMI(MBB, I, DL, get(Qpu::LUi), Qpu::VPM_LD_SETUP).addReg(Qpu::VPM_LD_WAIT, RegState::ImplicitKill).addImm(1234);

	  BuildMI(MBB, I, DL, get(Qpu::OR), Qpu::VPM_DAT_WRA).addReg(Qpu::VPM_DAT_RDA, RegState::Kill).addReg(Qpu::VPM_ST_WAIT).addReg(Qpu::VPM_LD_SETUP, RegState::ImplicitKill);
	  BuildMI(MBB, I, DL, get(Qpu::LUi), Qpu::VPM_ST_SETUP).addReg(Qpu::VPM_DAT_WRA, RegState::ImplicitKill).addImm(1234).addReg(DestReg, RegState::Implicit).addReg(Qpu::VPM_ST_WAIT, RegState::ImplicitKill);
  }
  //general
  else */if (Qpu::GPRAccRARBRegClass.contains(DestReg) && Qpu::GPRAccRARBRegClass.contains(SrcReg))
  {
	  MachineInstrBuilder MIB = BuildMI(MBB, I, DL, get(Qpu::MOVE));
	  MIB.addReg(DestReg, RegState::Define);
	  MIB.addReg(SrcReg, getKillRegState(KillSrc));
  }
  else if (DestReg == Qpu::T9 && Qpu::CPURegsRegClass.contains(SrcReg))
  {
	  MachineInstrBuilder MIB = BuildMI(MBB, I, DL, get(Qpu::MOVE));
	  MIB.addReg(DestReg, RegState::Define);
	  MIB.addReg(SrcReg, getKillRegState(KillSrc));
  }
  else
	  assert(!"cannot copy registers\n");

} // lbd document - mark - copyPhysReg

static MachineMemOperand* GetMemOperand(MachineBasicBlock &MBB, int FI,
                                        unsigned Flag) {
  MachineFunction &MF = *MBB.getParent();
  MachineFrameInfo &MFI = *MF.getFrameInfo();
  unsigned Align = MFI.getObjectAlignment(FI);

  return MF.getMachineMemOperand(MachinePointerInfo::getFixedStack(FI), Flag,
                                 MFI.getObjectSize(FI), Align);
} // lbd document - mark - GetMemOperand

//- st SrcReg, MMO(FI)
void QpuInstrInfo::
storeRegToStackSlot(MachineBasicBlock &MBB, MachineBasicBlock::iterator I,
                    unsigned SrcReg, bool isKill, int FI,
                    const TargetRegisterClass *RC,
                    const TargetRegisterInfo *TRI) const {
  DebugLoc DL;
  if (I != MBB.end()) DL = I->getDebugLoc();
  MachineMemOperand *MMO = GetMemOperand(MBB, FI, MachineMemOperand::MOStore);

  unsigned Opc = 0;

  Opc = Qpu::ST;
  assert(Opc && "Register class not handled!");
  BuildMI(MBB, I, DL, get(Opc)).addReg(SrcReg, getKillRegState(isKill))
    .addFrameIndex(FI).addImm(0).addMemOperand(MMO);
} // lbd document - mark - storeRegToStackSlot

void QpuInstrInfo:: // lbd document - mark - before loadRegFromStackSlot
loadRegFromStackSlot(MachineBasicBlock &MBB, MachineBasicBlock::iterator I,
                     unsigned DestReg, int FI,
                     const TargetRegisterClass *RC,
                     const TargetRegisterInfo *TRI) const
{
  DebugLoc DL;
  if (I != MBB.end()) DL = I->getDebugLoc();
  MachineMemOperand *MMO = GetMemOperand(MBB, FI, MachineMemOperand::MOLoad);
  unsigned Opc = 0;

  Opc = Qpu::LD;
  assert(Opc && "Register class not handled!");
  BuildMI(MBB, I, DL, get(Opc), DestReg).addFrameIndex(FI).addImm(0)
    .addMemOperand(MMO);
} // lbd document - mark - loadRegFromStackSlot

MachineInstr*
QpuInstrInfo::emitFrameIndexDebugValue(MachineFunction &MF, int FrameIx,
                                        uint64_t Offset, const MDNode *MDPtr,
                                        DebugLoc DL) const {
  MachineInstrBuilder MIB = BuildMI(MF, DL, get(Qpu::DBG_VALUE))
    .addFrameIndex(FrameIx).addImm(0).addImm(Offset).addMetadata(MDPtr);
  return &*MIB;
} // lbd document - mark - emitFrameIndexDebugValue

// QpuInstrInfo::expandPostRAPseudo
/// Expand Pseudo instructions into real backend instructions
bool QpuInstrInfo::expandPostRAPseudo(MachineBasicBlock::iterator MI) const {
  MachineBasicBlock &MBB = *MI->getParent();

  switch(MI->getDesc().getOpcode()) {
  default:
    return false;
  case Qpu::RetLR:
    ExpandRetLR(MBB, MI, Qpu::RET);
    break;
  }

  MBB.erase(MI);
  return true;
}

void QpuInstrInfo::ExpandRetLR(MachineBasicBlock &MBB,
                                MachineBasicBlock::iterator I,
                                unsigned Opc) const {
  BuildMI(MBB, I, I->getDebugLoc(), get(Opc)).addReg(Qpu::LR);
}

unsigned QpuInstrInfo::
InsertBranch(MachineBasicBlock &MBB, MachineBasicBlock *TBB,
             MachineBasicBlock *FBB,
             const SmallVectorImpl<MachineOperand> &Cond,
             DebugLoc DL) const {
  // Shouldn't be a fall through.
  assert(TBB && "InsertBranch must not be told to insert a fallthrough");

  // # of condition operands:
  //  Unconditional branches: 0
  //  Floating point branches: 1 (opc)
  //  Int BranchZero: 2 (opc, reg)
  //  Int Branch: 3 (opc, reg0, reg1)
  assert((Cond.size() <= 3) &&
         "# of Mips branch conditions must be <= 3!");

  // Two-way Conditional branch.
  if (FBB) {
    BuildCondBr(MBB, TBB, DL, Cond);
    BuildMI(&MBB, DL, get(Qpu::JMP)).addMBB(FBB);
    return 2;
  }

  // One way branch.
  // Unconditional branch.
  if (Cond.empty())
    BuildMI(&MBB, DL, get(Qpu::JMP)).addMBB(TBB);
  else // Conditional branch.
    BuildCondBr(MBB, TBB, DL, Cond);
  return 1;
}

void QpuInstrInfo::BuildCondBr(MachineBasicBlock &MBB,
                                MachineBasicBlock *TBB, DebugLoc DL,
                                const SmallVectorImpl<MachineOperand>& Cond)
  const {
  unsigned Opc = Cond[0].getImm();
  const MCInstrDesc &MCID = get(Opc);
  MachineInstrBuilder MIB = BuildMI(&MBB, DL, MCID);

  for (unsigned i = 1; i < Cond.size(); ++i) {
    if (Cond[i].isReg())
      MIB.addReg(Cond[i].getReg());
    else if (Cond[i].isImm())
      MIB.addImm(Cond[i].getImm());
    else
       assert(true && "Cannot copy operand");
  }
  MIB.addMBB(TBB);
}

/*unsigned QpuInstrInfo::
RemoveBranch(MachineBasicBlock &MBB) const
{
  MachineBasicBlock::reverse_iterator I = MBB.rbegin(), REnd = MBB.rend();
  MachineBasicBlock::reverse_iterator FirstBr;
  unsigned removed;

  // Skip all the debug instructions.
  while (I != REnd && I->isDebugValue())
    ++I;

  FirstBr = I;

  // Up to 2 branches are removed.
  // Note that indirect branches are not removed.
  for(removed = 0; I != REnd && removed < 2; ++I, ++removed)
    if (!getAnalyzableBrOpc(I->getOpcode()))
      break;

  MBB.erase(I.base(), FirstBr.base());

  return removed;
}
*/

