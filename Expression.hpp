#ifndef EXPRESSION_HPP
#define EXPRESSION_HPP

#include "llvm/ADT/Hashing.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Value.h"

#include <algorithm>
#include <functional>
#include <vector>

using namespace llvm;

enum ExpressionType {
  BINARY_OP,
  CMP_INST,
  UNARY_OP,
  CAST_OP,
  LOAD_INST,
  GETELEMENTPTR_INST,
  SELECT_INST
};

bool isExpression(Instruction&);

struct Expression {
  ExpressionType ExprTyp;
  unsigned Opcode;
  Type* Typ;
  bool IsCommutative;
  std::vector<Value*> Args;
  Value* Result;

  Expression(Instruction&);
  Expression() = default;

  void swapArgument(const unsigned, Value*);

  bool operator ==(const Expression&) const;
  std::size_t hash() const;
};

namespace std {
  template <>
  struct hash<Expression> {
    std::size_t operator()(const Expression& E) const {
      return E.hash();
    }
  };
}

#endif