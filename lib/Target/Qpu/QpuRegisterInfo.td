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
  let Namespace = "Qpu";
}


//===----------------------------------------------------------------------===//
//  Registers
//===----------------------------------------------------------------------===//
// The register string, such as "9" or "gp" will show on "llvm-objdump -d"
let Namespace = "Qpu" in {
  // General Purpose Registers
  def ZERO_IN : QpuReg<"rd_nop">, DwarfRegNum<[0]>;
  def ZERO_OUT : QpuReg<"wr_nop">, DwarfRegNum<[0]>;
  def AT   : QpuReg<"at">,    DwarfRegNum<[1]>;
  def RA0   : QpuReg<"ra0">,    DwarfRegNum<[40]>;
  def RA1   : QpuReg<"ra1">,    DwarfRegNum<[41]>;
  def RA2   : QpuReg<"ra2">,    DwarfRegNum<[42]>;
  def RA3   : QpuReg<"ra3">,    DwarfRegNum<[43]>;
  def RA4   : QpuReg<"ra4">,    DwarfRegNum<[44]>;
  def RA5   : QpuReg<"ra5">,    DwarfRegNum<[45]>;
  def RA6   : QpuReg<"ra6">,    DwarfRegNum<[46]>;
  def RA7   : QpuReg<"ra7">,    DwarfRegNum<[47]>;
  def RB0   : QpuReg<"rb0">,    DwarfRegNum<[80]>;
  def RB1   : QpuReg<"rb1">,    DwarfRegNum<[81]>;
  def RB2   : QpuReg<"rb2">,    DwarfRegNum<[82]>;
  def RB3   : QpuReg<"rb3">,    DwarfRegNum<[83]>;
  def RB4   : QpuReg<"rb4">,    DwarfRegNum<[84]>;
  def RB5   : QpuReg<"rb5">,    DwarfRegNum<[85]>;
  def RB6   : QpuReg<"rb6">,    DwarfRegNum<[86]>;
  def RB7   : QpuReg<"rb7">,    DwarfRegNum<[87]>;
  def T9   : QpuReg<"ra27">,   DwarfRegNum<[10]>;
  def T0   : QpuReg<"7">,    DwarfRegNum<[11]>;
  def SW   : QpuReg<"sw">,   DwarfRegNum<[12]>;
  def GP   : QpuReg<"gp">,   DwarfRegNum<[13]>;
  def FP   : QpuReg<"fp">,   DwarfRegNum<[14]>;
  def SP   : QpuReg<"sp">,   DwarfRegNum<[15]>;
  def LR   : QpuReg<"lr">,   DwarfRegNum<[16]>;
  def PC   : QpuReg<"pc">,   DwarfRegNum<[17]>;
  def VPM_LD_ADDR  : QpuReg<"vpm_ld_addr">,  DwarfRegNum<[18]>;
  def VPM_ST_ADDR  : QpuReg<"vpm_st_addr">,  DwarfRegNum<[19]>;
  def VPM_LD_WAIT  : QpuReg<"vpm_ld_wait">,  DwarfRegNum<[20]>;
  def VPM_ST_WAIT  : QpuReg<"vpm_st_wait">,  DwarfRegNum<[21]>;
  def VPM_DAT_RDA  : QpuReg<"rda_vpm_dat">,  DwarfRegNum<[22]>;
  def VPM_DAT_WRA  : QpuReg<"wra_vpm_dat">,  DwarfRegNum<[23]>;
  def VPM_LD_SETUP  : QpuReg<"vpm_ld_setup">,  DwarfRegNum<[24]>;
  def VPM_ST_SETUP  : QpuReg<"vpm_st_setup">,  DwarfRegNum<[25]>;

  def ACC0   : QpuReg<"acc0">,   DwarfRegNum<[26]>;
  def ACC1   : QpuReg<"acc1">,   DwarfRegNum<[27]>;
  def ACC2   : QpuReg<"acc2">,   DwarfRegNum<[28]>;
  def ACC3   : QpuReg<"acc3">,   DwarfRegNum<[29]>;
  def ACC5   : QpuReg<"acc5">,   DwarfRegNum<[30]>;
}

//===----------------------------------------------------------------------===//
// Register Classes
//===----------------------------------------------------------------------===//

def CPURegs : RegisterClass<"Qpu", [i32], 32, (add
  // Reserved
  ZERO_IN, ZERO_OUT, AT, 
  // Return Values and Arguments

  // Not preserved across procedure calls
  RA0, RA1, RA2, RA3, RA4, RA5, RA6, RA7,
  RB0, RB1, RB2, RB3, RB4, RB5, RB6, RB7,
  ACC0, ACC1, ACC2, ACC3, ACC5,
  SW,
  // Callee save
  // Reserved
  GP, FP, 
  SP, LR, PC)>;


/*def CPURegsFP : RegisterClass<"Qpu", [f32], 32, (add
  // Reserved
  ZERO_IN, ZERO_OUT, AT,
  // Return Values and Arguments

  // Not preserved across procedure calls
  RA0, RA1, RA2, RA3, RA4, RA5, RA6, RA7,
  RB0, RB1, RB2, RB3, RB4, RB5, RB6, RB7,
  ACC0, ACC1, ACC2, ACC3, ACC5,
  SW)>;*/

multiclass vec_regfile<ValueType type, int alignment>
{
 def _rf : RegisterClass<"Qpu", [type], alignment, (add
  // Reserved
  ZERO_IN, ZERO_OUT, AT,
  // Return Values and Arguments

  // Not preserved across procedure calls
  RA0, RA1, RA2, RA3, RA4, RA5, RA6, RA7,
  RB0, RB1, RB2, RB3, RB4, RB5, RB6, RB7,
  ACC0, ACC1, ACC2, ACC3, ACC5,
  SW)>;
}
  
defm F32x1 : vec_regfile<f32, 32>;
defm F32x2 : vec_regfile<v2f32, 512>;
defm F32x4 : vec_regfile<v4f32, 512>;
defm F32x8 : vec_regfile<v8f32, 512>;
defm F32x16 : vec_regfile<v16f32, 512>;
  

// Status Registers class
def SR   : RegisterClass<"Qpu", [i32], 32, (add SW)>;
