//===-- QpuRegisterInfo.td - Qpu Register defs -----------*- tablegen -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
//  Declarations that describe the QPU register file
//===----------------------------------------------------------------------===//


// We have banks of 16 registers each.
class QpuReg<string n> : Register<n> {
  field bits<7> Num;
  let Namespace = "Qpu";
}

// Qpu CPU Registers
class QpuGPRReg<bits<7> num, string n> : QpuReg<n> {
  let Num = num;
}

//===----------------------------------------------------------------------===//
//  Registers
//===----------------------------------------------------------------------===//
// The register string, such as "9" or "gp" will show on "llvm-objdump -d"
let Namespace = "Qpu" in {
  // General Purpose Registers
  def ZERO_IN : QpuGPRReg<0,  "rd_nop">, DwarfRegNum<[0]>;
  def ZERO_OUT : QpuGPRReg<0,  "wr_nop">, DwarfRegNum<[0]>;
  def AT   : QpuGPRReg<1,  "at">,    DwarfRegNum<[1]>;
  def RA0   : QpuGPRReg<2,  "ra0">,    DwarfRegNum<[2]>;
  def RA1   : QpuGPRReg<3,  "ra1">,    DwarfRegNum<[3]>;
  def RA2   : QpuGPRReg<4,  "ra2">,    DwarfRegNum<[4]>;
  def RA3   : QpuGPRReg<5,  "ra3">,    DwarfRegNum<[5]>;
  def RB0   : QpuGPRReg<2,  "rb0">,    DwarfRegNum<[2]>;
  def RB1   : QpuGPRReg<3,  "rb1">,    DwarfRegNum<[3]>;
  def RB2   : QpuGPRReg<4,  "rb2">,    DwarfRegNum<[4]>;
  def RB3   : QpuGPRReg<5,  "rb3">,    DwarfRegNum<[5]>;
  def T9   : QpuGPRReg<6,  "ra27">,   DwarfRegNum<[6]>;
  def T0   : QpuGPRReg<7,  "7">,    DwarfRegNum<[7]>;
  def SW   : QpuGPRReg<10, "sw">,   DwarfRegNum<[10]>;
  def GP   : QpuGPRReg<11, "gp">,   DwarfRegNum<[11]>;
  def FP   : QpuGPRReg<12, "fp">,   DwarfRegNum<[12]>;
  def SP   : QpuGPRReg<13, "sp">,   DwarfRegNum<[13]>;
  def LR   : QpuGPRReg<14, "lr">,   DwarfRegNum<[14]>;
  def PC   : QpuGPRReg<15, "pc">,   DwarfRegNum<[15]>;
//  def MAR  : Register< 16, "mar">,  DwarfRegNum<[16]>;
//  def MDR  : Register< 17, "mdr">,  DwarfRegNum<[17]>;
  def VPM_LD_ADDR  : QpuGPRReg< 16, "vpm_ld_addr">,  DwarfRegNum<[16]>;
  def VPM_ST_ADDR  : QpuGPRReg< 17, "vpm_st_addr">,  DwarfRegNum<[17]>;
  def VPM_LD_WAIT  : QpuGPRReg< 18, "vpm_ld_wait">,  DwarfRegNum<[18]>;
  def VPM_ST_WAIT  : QpuGPRReg< 19, "vpm_st_wait">,  DwarfRegNum<[19]>;
  def VPM_DAT_RDA  : QpuGPRReg< 20, "rda_vpm_dat">,  DwarfRegNum<[20]>;
  def VPM_DAT_WRA  : QpuGPRReg< 21, "wra_vpm_dat">,  DwarfRegNum<[21]>;
  def VPM_LD_SETUP  : QpuGPRReg< 22, "vpm_ld_setup">,  DwarfRegNum<[22]>;
  def VPM_ST_SETUP  : QpuGPRReg< 23, "vpm_st_setup">,  DwarfRegNum<[23]>;

  def ACC0   : QpuGPRReg<24, "acc0">,   DwarfRegNum<[20]>;
  def ACC1   : QpuGPRReg<25, "acc1">,   DwarfRegNum<[21]>;
  def ACC2   : QpuGPRReg<26, "acc2">,   DwarfRegNum<[22]>;
  def ACC3   : QpuGPRReg<27, "acc3">,   DwarfRegNum<[23]>;
  def ACC5   : QpuGPRReg<28, "acc5">,   DwarfRegNum<[24]>;

  // Hi/Lo registers number and name
//  def HI   : Register<"hi">, DwarfRegNum<[18]>;
//  def LO   : Register<"lo">, DwarfRegNum<[19]>;
}

//===----------------------------------------------------------------------===//
// Register Classes
//===----------------------------------------------------------------------===//

def CPURegs : RegisterClass<"Qpu", [i32], 32, (add
  // Reserved
  ZERO_IN, ZERO_OUT, AT, 
  // Return Values and Arguments
  RA0, RA1, RA2, RA3,
  // Not preserved across procedure calls
  ACC0, ACC1, ACC2, ACC3, ACC5,
  // Callee save
  SW,
  // Reserved
  GP, FP, 
  SP, LR, PC)>;

// Hi/Lo Registers class
//def HILO : RegisterClass<"Qpu", [i32], 32, (add HI, LO)>;

//def LdAddrDest : RegisterClass<"Qpu", [i32], 32, (add VPM_LD_ADDR)>;
//def StAddrDest : RegisterClass<"Qpu", [i32], 32, (add VPM_ST_ADDR)>;

//def LdDest : RegisterClass<"Qpu", [i32], 32, (add VPM_DAT_RDA)>;

// Status Registers class
def SR   : RegisterClass<"Qpu", [i32], 32, (add SW)>;

// Status Registers class
def LINK   : RegisterClass<"Qpu", [i32], 32, (add LR)>;

def NOP_REG   : RegisterClass<"Qpu", [i32], 32, (add ZERO_OUT)>;
