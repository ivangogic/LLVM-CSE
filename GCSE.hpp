#include "LCSE.hpp"

#include "llvm/ADT/BitVector.h"
#include "llvm/IR/Dominators.h"
#include "llvm/Support/raw_ostream.h"

#include <unordered_set>
#include <unordered_map>

class GCSE {
  Function& F;
  std::unordered_map<Expression, int> ExprVN;
  std::unordered_map<Expression, std::unordered_set<Value*>> ExprResults;
  std::unordered_map<BasicBlock*, BitVector> Gen;
  // Killed je zapravo NotKilled, 1 znaci da nije ubijen, zbog efikasnosti kod analize
  std::unordered_map<BasicBlock*, BitVector> Killed;
  std::unordered_map<BasicBlock*, BitVector> AvailIN;
  std::unordered_map<BasicBlock*, BitVector> AvailOUT;
  DominatorTree DT;

  BitVector calculateKillSet(BasicBlock& BB);

  bool isKilledInBB(Instruction* Inst);

  bool hasPhiOperands(const Expression& Expr);

  PHINode* findCompositePhi(const Expression& Expr, BasicBlock* BB);

  Value* findReplacementValue(const Expression& Expr, BasicBlock* BB, bool InitialCall = true);

  bool isKilledOnPath(const int EVN, BasicBlock* Src, BasicBlock* Dest);

  bool isKilledOnPath(const int EVN, BasicBlock* Src, BasicBlock* Dest,
                      std::unordered_set<BasicBlock*>& Visited,
                      bool IsKilled, bool InitialCall);

  void Sanitize(Function& F);

public:
  GCSE(Function&);

  void runAnalysis();
  
  bool runPass();
};