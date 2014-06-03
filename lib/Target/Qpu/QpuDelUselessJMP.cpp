//===-- QpuDelUselessJMP.cpp - Qpu DelJmp -------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// Simple pass to fills delay slots with useful instructions.
//
//===----------------------------------------------------------------------===//

#define DEBUG_TYPE "del-jmp"

#include "Qpu.h"
#include "QpuTargetMachine.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetInstrInfo.h"
#include "llvm/ADT/SmallSet.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"

using namespace llvm;

STATISTIC(NumDelJmp, "Number of useless jmp deleted");

static cl::opt<bool> EnableDelJmp(
  "enable-qpu-del-useless-jmp",
  cl::init(true),
  cl::desc("Delete useless jmp instructions: jmp 0."),
  cl::Hidden);

namespace {
  struct DelJmp : public MachineFunctionPass {

    TargetMachine &TM;
    const TargetInstrInfo *TII;

    static char ID;
    DelJmp(TargetMachine &tm)
      : MachineFunctionPass(ID), TM(tm), TII(tm.getInstrInfo()) { }

    virtual const char *getPassName() const {
      return "Qpu Del Useless jmp";
    }

    bool runOnMachineBasicBlock(MachineBasicBlock &MBB, MachineBasicBlock &MBBN, const QpuInstrInfo &TII);
    bool runOnMachineFunction(MachineFunction &F) {
    	const QpuInstrInfo &TII = *static_cast<const QpuInstrInfo*>(F.getTarget().getInstrInfo());

      bool Changed = false;
      if (EnableDelJmp) {
        MachineFunction::iterator FJ = F.begin();
        if (FJ != F.end())
          FJ++;
        if (FJ == F.end())
          return Changed;
        for (MachineFunction::iterator FI = F.begin(), FE = F.end();
             FJ != FE; ++FI, ++FJ)
          // In STL style, F.end() is the dummy BasicBlock() like '\0' in 
          //  C string. 
          // FJ is the next BasicBlock of FI; When FI range from F.begin() to 
          //  the PreviousBasicBlock of F.end() call runOnMachineBasicBlock().
          Changed |= runOnMachineBasicBlock(*FI, *FJ, TII);
      }
      return Changed;
    }

  };
  char DelJmp::ID = 0;

  class Nopper : public MachineFunctionPass {
  public:
    Nopper(TargetMachine &tm)
      : MachineFunctionPass(ID), TM(tm) { }

    virtual const char *getPassName() const {
      return "Qpu NOP inserter";
    }

    bool runOnMachineFunction(MachineFunction &F) {
      bool Changed = false;

      const QpuInstrInfo &TII = *static_cast<const QpuInstrInfo*>(F.getTarget().getInstrInfo());

      for (MachineFunction::iterator FI = F.begin(), FE = F.end();
           FI != FE; ++FI)
        Changed |= runOnMachineBasicBlock(*FI, TII);
      return Changed;
    }

  private:
    bool runOnMachineBasicBlock(MachineBasicBlock &MBB, const QpuInstrInfo &TII) {
	  bool Changed = false;

	  for (MachineBasicBlock::iterator I = MBB.begin(); I != MBB.end(); ++I) {

		// Bundle the NOP to the instruction with the delay slot.
		BuildMI(MBB, llvm::next(I), I->getDebugLoc(), TII.get(Qpu::NOP));
		MIBundleBuilder(MBB, I, llvm::next(llvm::next(I)));
	  }

	  return Changed;
    }

	  TargetMachine &TM;

    static char ID;
  };

  char Nopper::ID = 0;

} // end of anonymous namespace

bool DelJmp::
runOnMachineBasicBlock(MachineBasicBlock &MBB, MachineBasicBlock &MBBN, const QpuInstrInfo &TII) {
  bool Changed = false;

  MachineBasicBlock::iterator I = MBB.end();
  if (I != MBB.begin())
    I--;	// set I to the last instruction
  else
    return Changed;
    
  if (I->getOpcode() == Qpu::JMP && I->getOperand(0).getMBB() == &MBBN) {
    // I is the instruction of "jmp #offset=0", as follows,
    //     jmp	$BB0_3
    // $BB0_3:
    //     ld	$4, 28($sp)
    ++NumDelJmp;
    MBB.erase(I);	// delete the "JMP 0" instruction
    Changed = true;	// Notify LLVM kernel Changed
  }
  return Changed;

}

/// createQpuDelJmpPass - Returns a pass that DelJmp in Qpu MachineFunctions
FunctionPass *llvm::createQpuDelJmpPass(QpuTargetMachine &tm) {
  return new DelJmp(tm);
}

FunctionPass *llvm::createQpuNopperPass(QpuTargetMachine &tm) {
  return new Nopper(tm);
}
