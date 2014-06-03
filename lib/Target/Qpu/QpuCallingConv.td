//===-- QpuCallingConv.td - Calling Conventions for Qpu --*- tablegen -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
// This describes the calling conventions for Qpu architecture.
//===----------------------------------------------------------------------===//

/// CCIfSubtarget - Match if the current subtarget has a feature F.
class CCIfSubtarget<string F, CCAction A>:
  CCIf<!strconcat("State.getTarget().getSubtarget<QpuSubtarget>().", F), A>;

def RetCC_QpuEABI : CallingConv<[
  // i32 are returned in registers V0, V1, A0, A1
  CCIfType<[i32], CCAssignToReg<[RA0, RA1, RA2, RA3]>>
]>;

//===----------------------------------------------------------------------===//
// Qpu EABI Calling Convention
//===----------------------------------------------------------------------===//

def CC_QpuEABI : CallingConv<[
  // Promote i8/i16 arguments to i32.
  CCIfType<[i8, i16], CCPromoteToType<i32>>,
  // Integer values get stored in stack slots that are 4 bytes in
  // size and 4-byte aligned.
  CCIfType<[i32], CCAssignToStack<4, 4>>
]>;


//===----------------------------------------------------------------------===//
// Qpu Calling Convention Dispatch
//===----------------------------------------------------------------------===//

def CC_Qpu : CallingConv<[
  CCDelegateTo<CC_QpuEABI>
]>;


def RetCC_Qpu : CallingConv<[
  CCDelegateTo<RetCC_QpuEABI>
]>; // lbd document - mark - def RetCC_Qpu

def CSR_O32 : CalleeSavedRegs<(add LR, FP)>;

