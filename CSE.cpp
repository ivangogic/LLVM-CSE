#include "GCSE.hpp"

#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/Function.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

namespace {
  struct CSE : public FunctionPass {
    static char ID;
    CSE() : FunctionPass(ID) {}

    bool runOnFunction(Function &F) override {
      GCSE gcse { F };

      do {
        gcse.runAnalysis();
      } while (gcse.runPass());

      return true;
    }
  };
}

char CSE::ID = 0;
static RegisterPass<CSE> X("cse", "Global Common Subexpression Elimination");