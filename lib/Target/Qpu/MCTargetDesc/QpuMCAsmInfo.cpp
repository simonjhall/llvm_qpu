//===-- QpuMCAsmInfo.cpp - Qpu Asm Properties ---------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the declarations of the QpuMCAsmInfo properties.
//
//===----------------------------------------------------------------------===//

#include "QpuMCAsmInfo.h"
#include "llvm/ADT/Triple.h"

using namespace llvm;

void QpuMCAsmInfo::anchor() { }

QpuMCAsmInfo::QpuMCAsmInfo(StringRef TT) {
  Triple TheTriple(TT);
  if ((TheTriple.getArch() == Triple::qpu))
    IsLittleEndian = false;

  AlignmentIsInBytes          = false;
  Data16bitsDirective         = "\t.short\t";
  Data32bitsDirective         = "\t.word\t";
  Data64bitsDirective         = "\t.8byte\t";
  PrivateGlobalPrefix         = "$";
  CommentString               = "//";
  ZeroDirective               = "\t.space\t";
  GPRel32Directive            = "\t.gpword\t";
  GPRel64Directive            = "\t.gpdword\t";
  WeakRefDirective            = "\t.weak\t";

  SupportsDebugInformation = false;
  ExceptionsType = ExceptionHandling::DwarfCFI;
  HasLEB128 = false;
  DwarfRegNumForCFI = false;
}
