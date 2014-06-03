# MCTargetDesc/CMakeLists.txt
add_llvm_library(LLVMQpuDesc
  QpuAsmBackend.cpp
  QpuMCAsmInfo.cpp
  QpuMCCodeEmitter.cpp
  QpuMCTargetDesc.cpp
  QpuELFObjectWriter.cpp
  )

add_dependencies(LLVMQpuDesc QpuCommonTableGen)
