//===-- Qpu.td - Describe the Qpu Target Machine ---------*- tablegen -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
// Calling Conv, Instruction Descriptions
//===----------------------------------------------------------------------===//

include "QpuSchedule.td"
include "QpuInstrInfo.td"
include "QpuCallingConv.td"

def QpuInstrInfo : InstrInfo;

//===----------------------------------------------------------------------===//
// Qpu Subtarget features                                                    //
//===----------------------------------------------------------------------===//

def FeatureCmp         : SubtargetFeature<"cmp", "HasCmp", "true",
                                "Enable 'cmp' instructions.">;
def FeatureQpu32I     : SubtargetFeature<"qpu32I", "QpuArchVersion", 
                                "Qpu32I", "Qpu32I ISA Support",
                                [FeatureCmp]>;

//===----------------------------------------------------------------------===//
// Qpu processors supported.
//===----------------------------------------------------------------------===//

class Proc<string Name, list<SubtargetFeature> Features>
 : Processor<Name, QpuGenericItineraries, Features>;

def : Proc<"qpu32I",  [FeatureQpu32I]>;
// Above make QpuGenSubtargetInfo.inc set feature bit as the following order
// enum {
//   FeatureCmp =  1ULL << 0,
//   FeatureQpu32I =  1ULL << 1,
//   FeatureQpu32II =  1ULL << 2,
//   FeatureSlt =  1ULL << 3
// };

def QpuAsmWriter : AsmWriter {
  string AsmWriterClassName  = "InstPrinter";
  bit isMCAsmWriter = 1;
}

// Will generate QpuGenAsmWrite.inc included by QpuInstPrinter.cpp, contents 
//  as follows,
// void QpuInstPrinter::printInstruction(const MCInst *MI, raw_ostream &O) {...}
// const char *QpuInstPrinter::getRegisterName(unsigned RegNo) {...}
def Qpu : Target {
// def QpuInstrInfo : InstrInfo as before.
  let InstructionSet = QpuInstrInfo;
  let AssemblyWriters = [QpuAsmWriter];
}
