set(LLVM_TARGET_DEFINITIONS QpuOther.td)

# Generate QpuGenRegisterInfo.inc and QpuGenInstrInfo.inc which included by 
#  your hand code C++ files. 
# QpuGenRegisterInfo.inc came from QpuRegisterInfo.td, QpuGenInstrInfo.inc 
#  came from QpuInstrInfo.td.
tablegen(LLVM QpuGenRegisterInfo.inc -gen-register-info)
tablegen(LLVM QpuGenInstrInfo.inc -gen-instr-info)

tablegen(LLVM QpuGenCodeEmitter.inc -gen-emitter)
tablegen(LLVM QpuGenMCCodeEmitter.inc -gen-emitter -mc-emitter)

tablegen(LLVM QpuGenAsmWriter.inc -gen-asm-writer)
tablegen(LLVM QpuGenDAGISel.inc -gen-dag-isel)
tablegen(LLVM QpuGenCallingConv.inc -gen-callingconv)
tablegen(LLVM QpuGenSubtargetInfo.inc -gen-subtarget)

# QpuCommonTableGen must be defined
add_public_tablegen_target(QpuCommonTableGen)

# QpuCodeGen should match with LLVMBuild.txt QpuCodeGen
add_llvm_target(QpuCodeGen
  QpuAnalyzeImmediate.cpp
  QpuAsmPrinter.cpp
  QpuDelUselessJMP.cpp
  QpuEmitGPRestore.cpp
  QpuInstrInfo.cpp
  QpuISelDAGToDAG.cpp
  QpuISelLowering.cpp
  QpuFrameLowering.cpp
  QpuMCInstLower.cpp
  QpuMachineFunction.cpp
  QpuRegisterInfo.cpp
  QpuSubtarget.cpp
  QpuTargetMachine.cpp
  QpuTargetObjectFile.cpp
  QpuSelectionDAGInfo.cpp
  )

# Should match with "subdirectories =  MCTargetDesc TargetInfo" in LLVMBuild.txt
add_subdirectory(InstPrinter)
add_subdirectory(TargetInfo)
add_subdirectory(MCTargetDesc)
