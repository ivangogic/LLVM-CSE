add_llvm_library( LLVMCSE MODULE
  Expression.cpp
  LCSE.cpp
  GCSE.cpp
  CSE.cpp

  PLUGIN_TOOL
  opt
  )