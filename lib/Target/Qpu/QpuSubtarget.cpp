//===-- QpuSubtarget.cpp - Qpu Subtarget Information --------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements the Qpu specific subclass of TargetSubtargetInfo.
//
//===----------------------------------------------------------------------===//

#include "QpuSubtarget.h"
#include "Qpu.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/CommandLine.h"

#define GET_SUBTARGETINFO_TARGET_DESC
#define GET_SUBTARGETINFO_CTOR
#include "QpuGenSubtargetInfo.inc"

using namespace llvm;
 
static cl::opt<bool> UseSmallSectionOpt
                ("qpu-use-small-section", cl::Hidden, cl::init(false),
                 cl::desc("Use small section. Only work when -relocation-model="
                 "static. pic always not use small section."));

static cl::opt<bool> ReserveGPOpt
                ("qpu-reserve-gp", cl::Hidden, cl::init(false),
                 cl::desc("Never allocate $gp to variable"));

static cl::opt<bool> NoCploadOpt
                ("qpu-no-cpload", cl::Hidden, cl::init(false),
                 cl::desc("No issue .cpload"));

bool QpuReserveGP;
bool QpuNoCpload;

extern bool FixGlobalBaseReg;

void QpuSubtarget::anchor() { }

QpuSubtarget::QpuSubtarget(const std::string &TT, const std::string &CPU,
                             const std::string &FS, bool little, 
                             Reloc::Model _RM) :
  QpuGenSubtargetInfo(TT, CPU, FS),
  QpuABI(UnknownABI), IsLittle(little), RM(_RM)
{
  std::string CPUName = CPU;
  QpuArchVersion = Qpu32I;
  CPUName = "qpu32I";

  // Parse features string.
  ParseSubtargetFeatures(CPUName, FS);

  // Initialize scheduling itinerary for the specified CPU.
  InstrItins = getInstrItineraryForCPU(CPUName);

  // Set QpuABI if it hasn't been set yet.
  if (QpuABI == UnknownABI)
    QpuABI = O32;

  // Set UseSmallSection.
  UseSmallSection = UseSmallSectionOpt;
  QpuReserveGP = ReserveGPOpt;
  QpuNoCpload = NoCploadOpt;
  if (RM == Reloc::Static && !UseSmallSection && !QpuReserveGP)
    FixGlobalBaseReg = false;
  else
    FixGlobalBaseReg = true;
}

