#include "Expression.hpp"

using namespace llvm;

bool isExpression(Instruction& Inst) {
  return isa<BinaryOperator>(Inst) ||
         isa<CmpInst>(Inst) ||
         isa<UnaryOperator>(Inst) ||
         isa<CastInst>(Inst) ||
         isa<LoadInst>(Inst) ||
         isa<GetElementPtrInst>(Inst) ||
         isa<SelectInst>(Inst);
}

Expression::Expression(Instruction& Inst) {
  if (isa<BinaryOperator>(&Inst))           ExprTyp = BINARY_OP;
  else if (isa<CmpInst>(&Inst))             ExprTyp = CMP_INST;
  else if (isa<UnaryOperator>(&Inst))       ExprTyp = UNARY_OP;
  else if (isa<CastInst>(&Inst))            ExprTyp = CAST_OP;
  else if (isa<LoadInst>(&Inst))            ExprTyp = LOAD_INST;
  else if (isa<GetElementPtrInst>(&Inst))   ExprTyp = GETELEMENTPTR_INST;
  else if (isa<SelectInst>(&Inst))          ExprTyp = SELECT_INST;

  Opcode = Inst.getOpcode();
  Typ = Inst.getType();
  IsCommutative = Inst.isCommutative();
  Args = { Inst.value_op_begin(), Inst.value_op_end() };
  Result = &Inst;

  if (IsCommutative && Args[0] > Args[1])
    std::swap(Args[0], Args[1]);

  if (ExprTyp == CMP_INST) {
    auto CMPI = cast<CmpInst>(Result);
    if (CMPI->getPredicate() > CMPI->getSwappedPredicate()) {
      Opcode = CMPI->getSwappedPredicate();
      std::swap(Args[0], Args[1]);
    } else {
      Opcode = CMPI->getPredicate();
    }
  }
}

bool Expression::operator ==(const Expression& Other) const {
  if (ExprTyp != Other.ExprTyp || Typ != Other.Typ)
    return false;

  if (Opcode == Other.Opcode && Typ == Other.Typ && Args == Other.Args)
    return true;

  // // pokriva slucajeve x + y <=> y + x
  // if (ExprTyp == BINARY_OP && Opcode == Other.Opcode && Typ == Other.Typ && IsCommutative)
  //   return Args[0] == Other.Args[1] && Args[1] == Other.Args[0];

  // // pokriva slucajeve x < y <=> y > x
  // if (ExprTyp == CMP_INST) {
  //   auto CMPI = cast<CmpInst>(Result);
  //   if (CMPI->getPredicate() == CMPI->getSwappedPredicate())
  //     return false;
  //   auto otherCMPI = cast<CmpInst>(Result);
  //   if (otherCMPI->getPredicate() == CMPI->getSwappedPredicate())
  //     return Args[0] == Other.Args[1] && Args[1] == Other.Args[0];
  // }

  return false;
}

void Expression::swapArgs(const std::vector<Value*>& Args)
{
  this->Args = Args;
  if (ExprTyp == BINARY_OP && IsCommutative && Args[0] > Args[1])
    std::swap(this->Args[0], this->Args[1]);
}

std::size_t Expression::hash() const {
  return hash_combine(Opcode, Typ, hash_combine_range(Args.begin(), Args.end()));
}