#include "GCSE.hpp"

GCSE::GCSE(Function& F)
  : F { F }, DT { F } { }

// analiza dostupnih izraza
void GCSE::runAnalysis() {
  auto VN = 0;
  ExprVN.clear();
  ExprResults.clear();

  // pokrecemo lokalnu analizu, dva cilja:
  // 1. obavljamo deo posla oko eliminacije izraza, pritom uspostavljamo invarijantu da nijedan izraz nema zajednicki podizraz u istom bloku
  // 2. prikupljamo informacije o generisanim izrazima u svakom od blokova
  std::unordered_map<BasicBlock*, std::vector<Expression>> BBEval;
  for (auto& BB : F) {
    LCSE lcse { BB };
    lcse.run();
    auto Eval = lcse.moveAvailable();
    for (auto& Expr : Eval) {
      if (ExprVN.find(Expr) == ExprVN.end()) {
        ExprVN[Expr] = VN++;
        ExprResults[Expr] = { Expr.Result };
      } else {
        ExprResults[Expr].insert(Expr.Result);
      }
    }
    BBEval[&BB] = std::move(Eval);
  }

  // inicijalizujemo potrebne strukture, za analizu
  // radi efikasnosti koriste se bitvector-i
  for (auto& [BB, Eval] : BBEval) {
    Gen[BB] = BitVector(VN, false);
    for (auto& E : Eval)
      Gen[BB].set(ExprVN[E]);
  }

  for (auto& BB : F)
    Killed[&BB] = calculateKillSet(BB);
  
  // pocetni uslovi analize
  for (auto& BB : F) {
    // prvi uslov (AvailIN) nije neophodan u teoriji, moze biti bilo sta
    // ali s obzirom da je jedina operacija koja se vrsi nad njim operacija preseka
    // BitVector(VN, true) je neutral, pa je implementacija jednostavnija
    AvailIN[&BB] = BitVector(VN, true);
    AvailOUT[&BB] = BitVector(VN, true);
  }
  // u teoriji se dodaje prazan pocetni blok i postavlja se AvailOUT tog novog pocetnog blokana BitVector(VN, false)
  // zbog nacina kako LLVM generise IR (ne postoji povratna grana ka pocetnom bloku),
  // to je ekvivalentno zadatom uslovu
  AvailIN[&F.getEntryBlock()] = BitVector(VN, false);

  // analiza toka podataka unapred
  // funkcija prenosa je AvailIN[BB] = (presek svih prethodnika P; AvailOUT[P]) U (Gen[BB] - Kill[BB])
  // jos jednom napomena Kill je u programu NotKill, da bi uslov Gen[BB] - Kill[BB] mogli da zapisemo kao
  // Gen[BB] & Kill[BB] (operacija koju nam nudi BitVector)
  // prolazimo kroz sve BB dok god ima promena na izlazu bilo kog, moze se dokazati da ce u nekom trenutku doci do zaustavljanja 
  bool Changed;
  do {
    Changed = false;
    for (auto& BB : F) {
      for (auto Pred : predecessors(&BB))
        AvailIN[&BB] &= AvailOUT[Pred];

      auto AvailOUTPrev { AvailOUT[&BB] };

      AvailOUT[&BB] = AvailIN[&BB];
      AvailOUT[&BB] &= Killed[&BB];
      AvailOUT[&BB] |= Gen[&BB];

      AvailOUTPrev ^= AvailOUT[&BB];
      Changed |= AvailOUTPrev.any();
    }
  } while (Changed);
}

// pokrecemo nakon analize
// vracamo bool koji oznacava da li je doslo do promene koda, zato sto se pass moze pokrenuti vise puta,
// jer zamena izraza na nekom mestu moze prouzrokovati da se kasnije jos neki izrazi mogu zameniti
bool GCSE::runPass() {
  std::unordered_map<Value*, Value*> ReplacementValues;
  std::vector<Instruction*> RedundantInst;

  for (auto& BB : F) {
    for (auto& Inst : BB) {
      if (isExpression(Inst) && !isKilledInBB(&Inst)) {
        auto Expr = Expression(Inst);
        if (ExprVN.find(Expr) != ExprVN.end() && AvailIN[&BB][ExprVN[Expr]]) {
          auto Replacement = findReplacementValue(Expr, Inst.getParent());
          ReplacementValues[&Inst] = Replacement;
          RedundantInst.push_back(&Inst);
          ExprResults[Expr].erase(&Inst);
        } else if (hasPhiOperands(Expr)) {
          if (auto Replacement = findCompositePhi(Expr, &BB)) {
            ReplacementValues[&Inst] = Replacement;
            RedundantInst.push_back(&Inst);
            ExprResults[Expr].erase(&Inst);
          }
        }
      }
    }
  }

  for (auto [Use, Def] : ReplacementValues) {
    Use->replaceAllUsesWith(Def);
  }

  std::for_each(RedundantInst.begin(), RedundantInst.end(), [](auto Inst) { Inst->eraseFromParent(); });

  Sanitize(F);

  return !RedundantInst.empty();
}

BitVector GCSE::calculateKillSet(BasicBlock& BB) {
  std::unordered_set<Value*> KilledValues;
  for (auto& Inst : BB)
    if (isa<StoreInst>(&Inst))
      KilledValues.insert(Inst.getOperand(1));

  auto Killed = BitVector(ExprVN.size(), true);
  bool Changed;
  do {
    Changed = false;
    for (auto& [Expr, VN] : ExprVN)
      for (auto Arg : Expr.Args)
        if (KilledValues.find(Arg) != KilledValues.end()) {
          Changed |= Killed[VN];
          Killed.reset(VN);
          for (auto Result : ExprResults[Expr])
            KilledValues.insert(Result);
        }
  } while (Changed);
  
  return Killed;
}

// Proverava za datu instrukciju da li su njeni operandi ubijeni u BB od pocetka do te instrukcije,
// iduci unazad. Rekurzivna procedura, moraju da budu zivi operandi zadate instrukcije, i ako je operand
// rezultat neke druge instrukcije, onda i njegovi operandi moraju da budu zivi.
bool GCSE::isKilledInBB(Instruction* Inst) {
  auto BB = Inst->getParent();
  auto Iter = Inst->getReverseIterator();
  auto Expr = Expression(*Inst);
  std::unordered_set<Value*> LiveValues { Expr.Args.begin(), Expr.Args.end() };
  while (++Iter != BB->rend()) {
    if (LiveValues.find(&*Iter) != LiveValues.end()) {
      Expr = Expression(*Iter);
      for (auto V : Expr.Args)
        LiveValues.insert(V);
    } else if (isa<StoreInst>(&*Iter) && LiveValues.find(&*Iter->getOperand(1)) != LiveValues.end()) {
      return true;
    }
  }
  return false;
}

bool GCSE::hasPhiOperands(const Expression& Expr) {
  for (auto V : Expr.Args) {
    if (isa<PHINode>(*V))
      return true;
  }
  return false;
}

// "razmotava" phi operande
// najlakse objasniti na konkretnom primeru (pseudo IR):
// blokA:
//   %0 = ...
//   %1 = add %0, x
//   ...
// blokB:
//   %2 = ...
//   %3 = add %2, x
//   ...
// blokC:
//   %4 = phi [%0, %blokA], [%2, %blokB]
//   %5 = add %4, x
//   ...
// ideja je da prepoznamo da %5 mozemo da zamenimo sa phi [%1, %blokA], [%2, %blokB]
PHINode* GCSE::findCompositePhi(const Expression& Expr, BasicBlock* BB) {
  auto Predecessors = predecessors(BB);
  PHINode* PN = PHINode::Create(Expr.Typ, std::distance(Predecessors.begin(), Predecessors.end()));
  for (auto PBB : Predecessors) {
    auto E = Expr;
    auto Args = E.Args;
    for (auto i = 0u; i < Args.size(); ++i)
      if (auto PNArg = dyn_cast<PHINode>(E.Args[i]))
        if (std::find(PNArg->block_begin(), PNArg->block_end(), PBB) != PNArg->block_end())
          Args[i] = PNArg->getIncomingValueForBlock(PBB);

    E.swapArgs(Args);
    if (ExprVN.find(E) != ExprVN.end() && AvailOUT[PBB][ExprVN[E]]) {
      auto Replacement = findReplacementValue(E, PBB, false);
      PN->addIncoming(Replacement, PBB);
    } else {
      PN->deleteValue();
      return nullptr;
    }
  }

  PN->insertBefore(&BB->front());
  return PN;
}

// induktivno-rekurzivni algoritam
Value* GCSE::findReplacementValue(const Expression& Expr, BasicBlock* BB, bool InitialCall) {
  // baza indukcije
  if (!InitialCall && Gen[BB][ExprVN[Expr]]) {
    for (auto Other : ExprResults[Expr]) {
      auto OtherInst = cast<Instruction>(Other);
      if (OtherInst->getParent() == BB)
        return Other;
    }
  }

  // ako instr. dominira instr. koju menjamo i nije ubijena ni na jednoj putanji do nje, mozemo da je zamenimo
  for (auto Other : ExprResults[Expr]) {
    auto OtherInst = cast<Instruction>(Other);
    if (DT.dominates(OtherInst, BB) && !isKilledOnPath(ExprVN[Expr], OtherInst->getParent(), BB))
      return Other;
  }

  // u suprotnom zbog prethodne analize znamo da je izraz dostupan na svim putanjama do bloka, pa rekurzivno trazimo
  // instrukcije zbog kojih je izraz dostupan i "spajamo" ih pomocu phi instrukcije
  PHINode* PN = PHINode::Create(Expr.Typ, 0);
  ExprResults[Expr].insert(PN);
  PN->insertBefore(&BB->front());
  for (auto PBB : predecessors(BB)) {
    auto Replacement = findReplacementValue(Expr, PBB, false);
    if (Replacement != PN)
      PN->addIncoming(Replacement, PBB);
  }
  return PN;
}

// interfejs za narednu metodu
bool GCSE::isKilledOnPath(const int EVN, BasicBlock* Src, BasicBlock* Dest) {
  std::unordered_set<BasicBlock*> Visited;
  return isKilledOnPath(EVN, Src, Dest, Visited, false, true);
}

// klasican DFS, izraz je ubijen, ako je ubijen na barem jednoj putanji od Src do Dest
bool GCSE::isKilledOnPath(const int EVN, BasicBlock* Src, BasicBlock* Dest,
                    std::unordered_set<BasicBlock*>& Visited,
                    bool IsKilled, bool InitialCall) {
  if (Src == Dest)
    return IsKilled;
  
  Visited.insert(Src);
  auto KilledOnAnyPath = false;
  for (auto BB : successors(Src))
    if (Visited.find(BB) == Visited.end())
      KilledOnAnyPath |= isKilledOnPath(EVN, BB, Dest, Visited, IsKilled || (!InitialCall && !Killed[Src][EVN]), false);
  return KilledOnAnyPath;
}

// nakon sto zamenimo sve instrukcije za koje smo nasli prethodna izracunavanja, neke load i phi instrukcije
// mogu da postanu suvisne
void GCSE::Sanitize(Function& F) {
  std::vector<Instruction*> RedundantInst;
  for (auto& BB: F)
    for (auto& Inst : BB)
      if ((isa<LoadInst>(&Inst) || isa<PHINode>(&Inst)) && Inst.use_empty())
        RedundantInst.push_back(&Inst);

  std::for_each(RedundantInst.begin(), RedundantInst.end(), [](auto Inst) { Inst->eraseFromParent(); });
}