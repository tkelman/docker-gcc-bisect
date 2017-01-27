#include "llvm/ADT/PostOrderIterator.h"
#include "llvm/ADT/SetVector.h"
#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/TargetTransformInfo.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/IRBuilder.h"
using namespace llvm;
namespace {
class BoUpSLP {
public:
  typedef SmallVector<Value *, 8> ValueList;
  BoUpSLP(Function *Func, ScalarEvolution *Se, TargetTransformInfo *Tti,
          TargetLibraryInfo *TLi, AliasAnalysis *Aa, LoopInfo *Li,
          DominatorTree *Dt, AssumptionCache *AC)
      : NumLoadsWantToKeepOrder(0), NumLoadsWantToChangeOrder(0), F(Func),
        Builder(Se->getContext()) {}
  void getTreeCost();
  void buildTree(ArrayRef<Value *> Roots,
                 ArrayRef<Value *> UserIgnoreLst = None);
  bool shouldReorder() const {}
  struct TreeEntry {
    ValueList Scalars;
  };
  std::vector<TreeEntry> VectorizableTree;
  SmallDenseMap<Value *, int> ScalarToTreeEntry;
  struct ExternalUser {
    Value *Scalar;
    int Lane;
  };
  typedef SmallVector<ExternalUser, 16> UserList;
  UserList ExternalUses;
  SetVector<Instruction *> GatherSeq;
  int NumLoadsWantToKeepOrder;
  int NumLoadsWantToChangeOrder;
  Function *F;
  TargetTransformInfo *TTI;
  TargetLibraryInfo *TLI;
  DominatorTree *DT;
  IRBuilder<> Builder;
};
void BoUpSLP::buildTree(ArrayRef<Value *> Roots,
                        ArrayRef<Value *> UserIgnoreLst) {
  for (int EIdx = 0, EE = VectorizableTree.size(); EIdx < EE; ++EIdx) {
    TreeEntry *Entry = &VectorizableTree[EIdx];
    for (int Lane = 0, LE = Entry->Scalars.size(); Lane != LE; ++Lane) {
      for (User *U : Entry->Scalars[Lane]->users()) {
      }
    }
  }
}
void BoUpSLP::getTreeCost() {
  unsigned BundleWidth = VectorizableTree[0].Scalars.size();
  for (UserList::iterator I = ExternalUses.begin(), E = ExternalUses.end();
       I != E; ++I) {
    VectorType::get(I->Scalar->getType(), BundleWidth);
  }
}
struct SLPVectorizer : public FunctionPass {
  static char ID;
  explicit SLPVectorizer() : FunctionPass(ID) {}
  ScalarEvolution *SE;
  TargetTransformInfo *TTI;
  TargetLibraryInfo *TLI;
  AliasAnalysis *AA;
  LoopInfo *LI;
  DominatorTree *DT;
  AssumptionCache *AC;
  bool runOnFunction(Function &F) override {
    SE = &getAnalysis<ScalarEvolution>();
    TTI = &getAnalysis<TargetTransformInfoWrapperPass>().getTTI(F);
    if (!TTI->getNumberOfRegisters(true))
      return false;
    BoUpSLP R(&F, SE, TTI, TLI, AA, LI, DT, AC);
    for (auto BB : post_order(&F.getEntryBlock())) {
      vectorizeChainsInBlock(BB, R);
    }
  }
  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<ScalarEvolution>();
    AU.addRequired<TargetTransformInfoWrapperPass>();
  }
  void tryToVectorizeList(ArrayRef<Value *> VL, BoUpSLP &R,
                          ArrayRef<Value *> BuildVector = None,
                          bool allowReorder = false);
  void tryToVectorize(BinaryOperator *V, BoUpSLP &R);
  void vectorizeChainsInBlock(BasicBlock *BB, BoUpSLP &R);
};
void SLPVectorizer::tryToVectorizeList(ArrayRef<Value *> VL, BoUpSLP &R,
                                       ArrayRef<Value *> BuildVector,
                                       bool allowReorder) {
  Instruction *I0 = dyn_cast<Instruction>(VL[0]);
  if (!I0)
    return;
  for (unsigned i = 0, e = VL.size(); i < e; ++i) {
    unsigned OpsWidth = 0;
    ArrayRef<Value *> Ops = VL.slice(i, OpsWidth);
    if (allowReorder && R.shouldReorder()) {
      Value *ReorderedOps[] = {Ops[1], Ops[0]};
      R.buildTree(ReorderedOps, None);
    }
    R.getTreeCost();
  }
}
void SLPVectorizer::tryToVectorize(BinaryOperator *V, BoUpSLP &R) {
  BinaryOperator *A = dyn_cast<BinaryOperator>(V->getOperand(0));
  BinaryOperator *B = dyn_cast<BinaryOperator>(V->getOperand(1));
  if (B && B->hasOneUse()) {
    BinaryOperator *A0 = dyn_cast<BinaryOperator>(A->getOperand(0));
    tryToVectorizeList({A0, B}, R, None, true);
  }
}
void SLPVectorizer::vectorizeChainsInBlock(BasicBlock *BB, BoUpSLP &R) {
  for (BasicBlock::iterator it = BB->begin(), e = BB->end(); it != e; it++) {
    if (PHINode *P = dyn_cast<PHINode>(it)) {
      Value *Rdx = (P->getIncomingBlock(0) == BB
                        ? (P->getIncomingValue(0))
                        : (P->getIncomingBlock(1) == BB ? P->getIncomingValue(1)
                                                        : nullptr));
      BinaryOperator *BI = dyn_cast_or_null<BinaryOperator>(Rdx);
      tryToVectorize(dyn_cast<BinaryOperator>(BI->getOperand(0)), R);
    }
    if (StoreInst *SI = dyn_cast<StoreInst>(it))
      if (BinaryOperator *BinOp =
              dyn_cast<BinaryOperator>(SI->getValueOperand())) {
        tryToVectorize(BinOp, R);
      }
    if (dyn_cast<CmpInst>(it)) {
      SmallVector<Value *, 16> BuildVector;
      SmallVector<Value *, 16> BuildVectorOpds;
      tryToVectorizeList(BuildVectorOpds, R, BuildVector);
    }
  }
}
} // end anonymous namespace
char SLPVectorizer::ID = 0;
static const char lv_name[] = "SLP Vectorizer";
INITIALIZE_PASS_BEGIN(SLPVectorizer, "slp-vectorizer", lv_name, false, false)
INITIALIZE_PASS_END(SLPVectorizer, "slp-vectorizer", lv_name, false, false)
namespace llvm {
Pass *createSLPVectorizerPass() { return new SLPVectorizer(); }
}
