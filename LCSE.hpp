#ifndef LCSE_HPP
#define LCSE_HPP

#include "Expression.hpp"

using namespace llvm;

class LCSE {
  BasicBlock& BB;
  std::vector<Expression> Available;
  bool Changed;

public:
  LCSE(BasicBlock&);

  void addExpression(const Expression&);
  Value* findExpression(const Expression&) const;
  void kill(Value*);

  void run();

  std::vector<Expression>&& moveAvailable();

  bool hasChanged() const;
};

#endif