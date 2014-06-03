//===-- QpuMCAsmInfo.h - Qpu Asm Info ------------------------*- C++ -*--===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the declaration of the QpuMCAsmInfo class.
//
//===----------------------------------------------------------------------===//

#ifndef QPUTARGETASMINFO_H
#define QPUTARGETASMINFO_H

#include "llvm/MC/MCAsmInfo.h"

namespace llvm {
  class StringRef;
  class Target;

  class QpuMCAsmInfo : public MCAsmInfo {
    virtual void anchor();
  public:
    explicit QpuMCAsmInfo(StringRef TT);
  };

} // namespace llvm

#endif
