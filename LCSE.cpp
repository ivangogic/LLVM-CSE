#include "LCSE.hpp"

#include <unordered_set>

using namespace llvm;

// Local common subexpression elimination
LCSE::LCSE(BasicBlock& BB)
  : BB { BB } {}

void LCSE::addExpression(const Expression& Expr) {
  Available.push_back(Expr);
}

Value* LCSE::findExpression(const Expression& Expr) const {
  auto Iter = std::find(Available.begin(), Available.end(), Expr);
  return Iter != Available.end() ? Iter->Result : nullptr;
}

// U principu rekurzivna procedura, krecemo od vrednosti koja je postala nevalidna,
// gde god se javlja kao operand ceo izraz postaje nevalidan pa i tu vrednost dodajemo u killedValues.
// Lako se optimizuje u iterativnu proceduru u jednom prolazu, jer kada neka vrednost postane nevalidna
// onda poze da ucestvuje samo u izrazima koji slede nakon izraza u kome je ta vrednost izrazucunata,
// zbog toga je available vector, a ne set.
void LCSE::kill(Value* V) {
  std::unordered_set<Value*> KilledValues {{V}};
  std::vector<std::vector<Expression>::iterator> Unavailable;

  for (auto Expr = Available.begin(); Expr != Available.end(); ++Expr)
    for (auto Arg : Expr->Args)
      if (KilledValues.find(Arg) != KilledValues.end()) {
        Unavailable.push_back(Expr);
        KilledValues.insert(Expr->Result);
      }

  for (auto Iter : Unavailable)
    Available.erase(Iter);
}

// prolazi kroz instrukcije, ako je neka instrukcija izraz proveri da li taj izraz vec dostupan
// ako jeste zameni ga, u suprotnom ga dodaj u listu dostupnih
// ako je u pitanju store instrukcija, izbaci iz dostupnih izraza sve izraze koje direktno ili
// indirektno kao operande ukljucuju ono u sta se store-uje
void LCSE::run() {
  std::vector<Instruction*> RedundantInst;
  for (auto& Inst : BB) {
    if (isExpression(Inst)) {
      auto Expr = Expression(Inst);
      if (auto ReplacementValue = findExpression(Expr)) {
        Inst.replaceAllUsesWith(ReplacementValue);
        RedundantInst.push_back(&Inst);
      } else {
        addExpression(Expr);
      }
    }
    else if (isa<StoreInst>(&Inst)) {
      kill(Inst.getOperand(1));
    }
  }

  std::for_each(RedundantInst.begin(), RedundantInst.end(), [](auto Inst) { Inst->eraseFromParent(); });

  Changed = !RedundantInst.empty();
}

// zbog nacina na koji se pass koristi, efikasnije od getAvailable koji bi vrsio kopiranje
std::vector<Expression>&& LCSE::moveAvailable() {
  return std::move(Available);
}

bool LCSE::hasChanged() const {
  return Changed;
}