#include "llvm/ADT/MapVector.h"
#include "llvm/ADT/PostOrderIterator.h"
#include "llvm/ADT/SetVector.h"
#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/TargetTransformInfo.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/NoFolder.h"
#include "llvm/Support/Debug.h"
#include "llvm/Analysis/VectorUtils.h"
using namespace llvm;
#define SV_NAME "slp-vectorizer"
static cl::opt<int>
    SLPCostThreshold("slp-threshold", cl::init(0), cl::Hidden,
                     cl::desc("Only vectorize if you gain more than this "
                              "number "));
static cl::opt<bool>
ShouldVectorizeHor("slp-vectorize-hor", cl::init(false), cl::Hidden,
                   cl::desc("Attempt to vectorize horizontal reductions"));
static cl::opt<bool> ShouldStartVectorizeHorAtStore(
    cl::desc(
        "Attempt to vectorize horizontal reductions feeding into a store"));
namespace {
static const unsigned RecursionMaxDepth = 12;
static const unsigned AliasedCheckLimit = 10;
static const unsigned MaxMemDepDistance = 160;
static BasicBlock *getSameBlock(ArrayRef<Value *> VL) {
}
static bool allConstant(ArrayRef<Value *> VL) {
}
static bool isSplat(ArrayRef<Value *> VL) {
}
static unsigned getAltOpcode(unsigned Op) {
  switch (Op) {
  }
}
static bool canCombineAsAltInst(unsigned Op) {
  if (Op == Instruction::FAdd || Op == Instruction::FSub ||
      Op == Instruction::Sub || Op == Instruction::Add)
    return true;
}
static unsigned isAltInst(ArrayRef<Value *> VL) {
  for (int i = 1, e = VL.size(); i < e; i++) {
  }
}
static unsigned getSameOpcode(ArrayRef<Value *> VL) {
  Instruction *I0 = dyn_cast<Instruction>(VL[0]);
  unsigned Opcode = I0->getOpcode();
  for (int i = 1, e = VL.size(); i < e; i++) {
    Instruction *I = dyn_cast<Instruction>(VL[i]);
    if (!I || Opcode != I->getOpcode()) {
    }
  }
}
static void propagateIRFlags(Value *I, ArrayRef<Value *> VL) {
  if (auto *VecOp = dyn_cast<BinaryOperator>(I)) {
    if (auto *Intersection = dyn_cast<BinaryOperator>(VL[0])) {
    }
  }
}
static Instruction *propagateMetadata(Instruction *I, ArrayRef<Value *> VL) {
  SmallVector<std::pair<unsigned, MDNode *>, 4> Metadata;
  for (unsigned i = 0, n = Metadata.size(); i != n; ++i) {
    unsigned Kind = Metadata[i].first;
    MDNode *MD = Metadata[i].second;
    for (int i = 1, e = VL.size(); MD && i != e; i++) {
      switch (Kind) {
      }
    }
  }
}
static Type* getSameType(ArrayRef<Value *> VL) {
}
static bool CanReuseExtract(ArrayRef<Value *> VL) {
  for (unsigned i = 1, e = VL.size(); i < e; ++i) {
  }
}
static bool InTreeUserNeedToExtract(Value *Scalar, Instruction *UserInst,
                                    TargetLibraryInfo *TLI) {
  unsigned Opcode = UserInst->getOpcode();
  switch (Opcode) {
  case Instruction::Load: {
    CallInst *CI = cast<CallInst>(UserInst);
    Intrinsic::ID ID = getIntrinsicIDForCall(CI, TLI);
    if (hasVectorInstrinsicScalarOpd(ID, 1)) {
    }
  }
  }
}
static MemoryLocation getLocation(Instruction *I, AliasAnalysis *AA) {
}
static bool isSimple(Instruction *I) {
  if (MemIntrinsic *MI = dyn_cast<MemIntrinsic>(I))
    return !MI->isVolatile();
}
class BoUpSLP {
public:
  typedef SmallVector<Value *, 8> ValueList;
  BoUpSLP(Function *Func, ScalarEvolution *Se, TargetTransformInfo *Tti,
          TargetLibraryInfo *TLi, AliasAnalysis *Aa, LoopInfo *Li,
          DominatorTree *Dt, AssumptionCache *AC)
      : NumLoadsWantToKeepOrder(0), NumLoadsWantToChangeOrder(0), F(Func),
        Builder(Se->getContext()) {
  }
  Value *vectorizeTree();
  int getTreeCost();
  void buildTree(ArrayRef<Value *> Roots,
                 ArrayRef<Value *> UserIgnoreLst = None);
  void deleteTree() {
    for (auto &Iter : BlocksSchedules) {
    }
  }
  bool isConsecutiveAccess(Value *A, Value *B, const DataLayout &DL);
  void optimizeGatherSequence();
  bool shouldReorder() const {
  }
  struct TreeEntry;
  int getEntryCost(TreeEntry *E);
  void buildTree_rec(ArrayRef<Value *> Roots, unsigned Depth);
  Value *vectorizeTree(TreeEntry *E);
  Value *vectorizeTree(ArrayRef<Value *> VL);
  Value *alreadyVectorized(ArrayRef<Value *> VL) const;
  static Value *getPointerOperand(Value *I);
  static unsigned getAddressSpaceOperand(Value *I);
  int getGatherCost(ArrayRef<Value *> VL);
  void setInsertPointAfterBundle(ArrayRef<Value *> VL);
  Value *Gather(ArrayRef<Value *> VL, VectorType *Ty);
  bool isFullyVectorizableTinyTree();
  void reorderAltShuffleOperands(ArrayRef<Value *> VL,
                                 SmallVectorImpl<Value *> &Left,
                                 SmallVectorImpl<Value *> &Right);
  void reorderInputsAccordingToOpcode(ArrayRef<Value *> VL,
                                      SmallVectorImpl<Value *> &Left,
                                      SmallVectorImpl<Value *> &Right);
  struct TreeEntry {
    bool isSame(ArrayRef<Value *> VL) const {
    }
    ValueList Scalars;
    Value *VectorizedValue;
    bool NeedToGather;
  };
  TreeEntry *newTreeEntry(ArrayRef<Value *> VL, bool Vectorized) {
    if (Vectorized) {
      for (int i = 0, e = VL.size(); i != e; ++i) {
      }
    }
  }
  std::vector<TreeEntry> VectorizableTree;
  SmallDenseMap<Value*, int> ScalarToTreeEntry;
  struct ExternalUser {
    Value *Scalar;
    llvm::User *User;
    int Lane;
  };
  typedef SmallVector<ExternalUser, 16> UserList;
  bool isAliased(const MemoryLocation &Loc1, Instruction *Inst1,
                 Instruction *Inst2) {
    AliasCacheKey key = std::make_pair(Inst1, Inst2);
    Optional<bool> &result = AliasCache[key];
    if (result.hasValue()) {
    }
  }
  typedef std::pair<Instruction *, Instruction *> AliasCacheKey;
  DenseMap<AliasCacheKey, Optional<bool>> AliasCache;
  void eraseInstruction(Instruction *I) {
  }
  UserList ExternalUses;
  SetVector<Instruction *> GatherSeq;
  SetVector<BasicBlock *> CSEBlocks;
  struct ScheduleData {
    enum { InvalidDeps = -1 };
    ScheduleData()
        : Inst(nullptr), FirstInBundle(nullptr), NextInBundle(nullptr),
          UnscheduledDepsInBundle(InvalidDeps), IsScheduled(false) {}
    void init(int BlockSchedulingRegionID) {
    }
    bool hasValidDependencies() const { return Dependencies != InvalidDeps; }
    bool isSchedulingEntity() const { return FirstInBundle == this; }
    bool isReady() const {
    }
    int incrementUnscheduledDeps(int Incr) {
    }
    void resetUnscheduledDeps() {
    }
    void clearDependencies() {
    }
    void dump(raw_ostream &os) const {
      if (!isSchedulingEntity()) {
      }
    }
    Instruction *Inst;
    ScheduleData *FirstInBundle;
    ScheduleData *NextInBundle;
    ScheduleData *NextLoadStore;
    SmallVector<ScheduleData *, 4> MemoryDependencies;
    int Dependencies;
    int UnscheduledDepsInBundle;
    bool IsScheduled;
  };
#ifndef NDEBUG
#endif
  struct BlockScheduling {
    BlockScheduling(BasicBlock *BB)
        : BB(BB), ChunkSize(BB->size()), ChunkPos(ChunkSize),
          SchedulingRegionID(1) {}
    void clear() {
    }
    ScheduleData *getScheduleData(Value *V) {
    }
    bool isInSchedulingRegion(ScheduleData *SD) {
    }
    template <typename ReadyListType>
    void schedule(ScheduleData *SD, ReadyListType &ReadyList) {
      ScheduleData *BundleMember = SD;
      while (BundleMember) {
        for (Use &U : BundleMember->Inst->operands()) {
          ScheduleData *OpDef = getScheduleData(U.get());
          if (OpDef && OpDef->hasValidDependencies() &&
              OpDef->incrementUnscheduledDeps(-1) == 0) {
          }
        }
        for (ScheduleData *MemoryDepSD : BundleMember->MemoryDependencies) {
          if (MemoryDepSD->incrementUnscheduledDeps(-1) == 0) {
          }
        }
      }
    }
    template <typename ReadyListType>
    void initialFillReadyList(ReadyListType &ReadyList) {
      for (auto *I = ScheduleStart; I != ScheduleEnd; I = I->getNextNode()) {
      }
    }
    bool tryScheduleBundle(ArrayRef<Value *> VL, BoUpSLP *SLP);
    void cancelScheduling(ArrayRef<Value *> VL);
    void extendSchedulingRegion(Value *V);
    void initScheduleData(Instruction *FromI, Instruction *ToI,
                          ScheduleData *PrevLoadStore,
                          ScheduleData *NextLoadStore);
    void calculateDependencies(ScheduleData *SD, bool InsertInReadyList,
                               BoUpSLP *SLP);
    void resetSchedule();
    BasicBlock *BB;
    int ChunkSize;
    int ChunkPos;
    DenseMap<Value *, ScheduleData *> ScheduleDataMap;
    struct ReadyList : SmallVector<ScheduleData *, 8> {
    };
    ReadyList ReadyInsts;
    Instruction *ScheduleStart;
    Instruction *ScheduleEnd;
    int SchedulingRegionID;
  };
  MapVector<BasicBlock *, std::unique_ptr<BlockScheduling>> BlocksSchedules;
  void scheduleBlock(BlockScheduling *BS);
  int NumLoadsWantToKeepOrder;
  int NumLoadsWantToChangeOrder;
  Function *F;
  TargetTransformInfo *TTI;
  TargetLibraryInfo *TLI;
  AliasAnalysis *AA;
  DominatorTree *DT;
  IRBuilder<> Builder;
};
void BoUpSLP::buildTree(ArrayRef<Value *> Roots,
                        ArrayRef<Value *> UserIgnoreLst) {
  for (int EIdx = 0, EE = VectorizableTree.size(); EIdx < EE; ++EIdx) {
    TreeEntry *Entry = &VectorizableTree[EIdx];
    for (int Lane = 0, LE = Entry->Scalars.size(); Lane != LE; ++Lane) {
      Value *Scalar = Entry->Scalars[Lane];
      for (User *U : Scalar->users()) {
        Instruction *UserInst = dyn_cast<Instruction>(U);
        if (ScalarToTreeEntry.count(U)) {
          int Idx = ScalarToTreeEntry[U];
          TreeEntry *UseEntry = &VectorizableTree[Idx];
          Value *UseScalar = UseEntry->Scalars[0];
          if (UseScalar != U ||
              !InTreeUserNeedToExtract(Scalar, UserInst, TLI)) {
            DEBUG(dbgs() << "SLP: \tInternal user will be removed:" << *U
                         << ".\n");
          }
        }
      }
    }
  }
}
void BoUpSLP::buildTree_rec(ArrayRef<Value *> VL, unsigned Depth) {
  if (Depth == RecursionMaxDepth) {
    }
  unsigned Opcode = getSameOpcode(VL);
  if (Opcode == Instruction::ShuffleVector) {
  }
  if (ScalarToTreeEntry.count(VL[0])) {
    int Idx = ScalarToTreeEntry[VL[0]];
    TreeEntry *E = &VectorizableTree[Idx];
    for (unsigned i = 0, e = VL.size(); i != e; ++i) {
      if (E->Scalars[i] != VL[i]) {
      }
    }
  }
  for (unsigned i = 0, e = VL.size(); i != e; ++i) {
    if (ScalarToTreeEntry.count(VL[i])) {
      DEBUG(dbgs() << "SLP: The instruction (" << *VL[i] <<
            ") is already in tree.\n");
    }
  }
  Instruction *VL0 = cast<Instruction>(VL[0]);
  BasicBlock *BB = cast<Instruction>(VL0)->getParent();
  if (!DT->isReachableFromEntry(BB)) {
      }
  auto &BSRef = BlocksSchedules[BB];
  if (!BSRef) {
  }
  BlockScheduling &BS = *BSRef.get();
  if (!BS.tryScheduleBundle(VL, this)) {
  }
  switch (Opcode) {
    case Instruction::PHI: {
      PHINode *PH = dyn_cast<PHINode>(VL0);
      for (unsigned j = 0; j < VL.size(); ++j)
        for (unsigned i = 0, e = PH->getNumIncomingValues(); i < e; ++i) {
          TerminatorInst *Term = dyn_cast<TerminatorInst>(
              cast<PHINode>(VL[j])->getIncomingValueForBlock(PH->getIncomingBlock(i)));
          if (Term) {
          }
        }
      for (unsigned i = 0, e = PH->getNumIncomingValues(); i < e; ++i) {
        ValueList Operands;
        for (unsigned j = 0; j < VL.size(); ++j)
          Operands.push_back(cast<PHINode>(VL[j])->getIncomingValueForBlock(
              PH->getIncomingBlock(i)));
      }
    }
    case Instruction::ExtractElement: {
      bool Reuse = CanReuseExtract(VL);
      if (Reuse) {
      }
    }
    case Instruction::Load: {
      for (unsigned i = 0, e = VL.size() - 1; i < e; ++i) {
        LoadInst *L = cast<LoadInst>(VL[i]);
        if (!L->isSimple()) {
        }
      }
      Type *Ty0 = cast<Instruction>(VL0)->getOperand(0)->getType();
      for (unsigned j = 0; j < VL.size(); ++j) {
        Type *CurTy = cast<Instruction>(VL[j])->getOperand(0)->getType();
        if (Ty0 != CurTy) {
        }
        auto Op = cast<Instruction>(VL[j])->getOperand(1);
        if (!isa<ConstantInt>(Op)) {
        }
      }
      if (isa<BinaryOperator>(VL0)) {
      }
      for (unsigned i = 0, e = VL0->getNumOperands(); i < e; ++i) {
      }
    }
  }
}
int BoUpSLP::getEntryCost(TreeEntry *E) {
  ArrayRef<Value*> VL = E->Scalars;
  if (E->NeedToGather) {
  }
  unsigned Opcode = getSameOpcode(VL);
  Instruction *VL0 = cast<Instruction>(VL[0]);
  switch (Opcode) {
    case Instruction::PHI: {
    }
    case Instruction::ExtractElement: {
      if (CanReuseExtract(VL)) {
        for (unsigned i = 0, e = VL.size(); i < e; ++i) {
        }
      }
    }
    case Instruction::BitCast: {
      Type *SrcTy = VL0->getOperand(0)->getType();
      int ScalarCost = VL.size() * TTI->getCastInstrCost(VL0->getOpcode(),
                                                         VL0->getType(), SrcTy);
    }
    case Instruction::Xor: {
      if (Opcode == Instruction::FCmp || Opcode == Instruction::ICmp ||
          Opcode == Instruction::Select) {
        TargetTransformInfo::OperandValueKind Op2VK =
            TargetTransformInfo::OK_UniformConstantValue;
        ConstantInt *CInt = nullptr;
        for (unsigned i = 0; i < VL.size(); ++i) {
          const Instruction *I = cast<Instruction>(VL[i]);
          if (!isa<ConstantInt>(I->getOperand(1))) {
          }
          if (Op2VK == TargetTransformInfo::OK_UniformConstantValue &&
              CInt != cast<ConstantInt>(I->getOperand(1)))
            Op2VK = TargetTransformInfo::OK_NonUniformConstantValue;
        }
      }
    }
  }
}
bool BoUpSLP::isFullyVectorizableTinyTree() {
  DEBUG(dbgs() << "SLP: Check whether the tree with height " <<
        VectorizableTree.size() << " is fully vectorizable .\n");
  if (!VectorizableTree[0].NeedToGather &&
      (allConstant(VectorizableTree[1].Scalars) ||
       isSplat(VectorizableTree[1].Scalars)))
    return true;
  Instruction *PrevInst = nullptr; 
  for (unsigned N = 0; N < VectorizableTree.size(); ++N) {
    Instruction *Inst = dyn_cast<Instruction>(VectorizableTree[N].Scalars[0]);
    if (!PrevInst) {
    }    
    BasicBlock::reverse_iterator InstIt(Inst), PrevInstIt(PrevInst);
    while (InstIt != PrevInstIt) {
      if (PrevInstIt == PrevInst->getParent()->rend()) {
      }
    }
  }
}
int BoUpSLP::getTreeCost() {
  unsigned BundleWidth = VectorizableTree[0].Scalars.size();
  for (unsigned i = 0, e = VectorizableTree.size(); i != e; ++i) {
  }
  int ExtractCost = 0;
  for (UserList::iterator I = ExternalUses.begin(), E = ExternalUses.end();
       I != E; ++I) {
    VectorType *VecTy = VectorType::get(I->Scalar->getType(), BundleWidth);
    ExtractCost += TTI->getVectorInstrCost(Instruction::ExtractElement, VecTy,
                                           I->Lane);
  }
}
int BoUpSLP::getGatherCost(ArrayRef<Value *> VL) {
}
Value *BoUpSLP::getPointerOperand(Value *I) {
}
unsigned BoUpSLP::getAddressSpaceOperand(Value *I) {
}
bool BoUpSLP::isConsecutiveAccess(Value *A, Value *B, const DataLayout &DL) {
}
void BoUpSLP::reorderAltShuffleOperands(ArrayRef<Value *> VL,
                                        SmallVectorImpl<Value *> &Left,
                                        SmallVectorImpl<Value *> &Right) {
  const DataLayout &DL = F->getParent()->getDataLayout();
  for (unsigned i = 0, e = VL.size(); i < e; ++i) {
  }
  for (unsigned j = 0; j < VL.size() - 1; ++j) {
    if (LoadInst *L = dyn_cast<LoadInst>(Left[j])) {
      if (LoadInst *L1 = dyn_cast<LoadInst>(Right[j + 1])) {
      }
      if (LoadInst *L1 = dyn_cast<LoadInst>(Left[j + 1])) {
        Instruction *VL1 = cast<Instruction>(VL[j]);
        if (isConsecutiveAccess(L, L1, DL) && VL1->isCommutative()) {
        }
      }
    }
  }
}
void BoUpSLP::reorderInputsAccordingToOpcode(ArrayRef<Value *> VL,
                                             SmallVectorImpl<Value *> &Left,
                                             SmallVectorImpl<Value *> &Right) {
  SmallVector<Value *, 16> OrigLeft, OrigRight;
  bool AllSameOpcodeLeft = true;
  bool AllSameOpcodeRight = true;
  for (unsigned i = 0, e = VL.size(); i != e; ++i) {
    Instruction *I = cast<Instruction>(VL[i]);
    Value *VLeft = I->getOperand(0);
    Value *VRight = I->getOperand(1);
    Instruction *ILeft = dyn_cast<Instruction>(VLeft);
    Instruction *IRight = dyn_cast<Instruction>(VRight);
    if (i && AllSameOpcodeLeft && ILeft) {
    }
    if (i && AllSameOpcodeRight && IRight) {
      if (Instruction *PRight = dyn_cast<Instruction>(OrigRight[i - 1])) {
      } else
        AllSameOpcodeRight = false;
    }
    if (ILeft && IRight) {
      if (!i && ILeft->getOpcode() > IRight->getOpcode()) {
      } else if (i && ILeft->getOpcode() == IRight->getOpcode() &&
                 Left[i - 1] == IRight) {
      }
    }
  }
  const DataLayout &DL = F->getParent()->getDataLayout();
  for (unsigned j = 0; j < VL.size() - 1; ++j) {
    if (LoadInst *L = dyn_cast<LoadInst>(Left[j])) {
      if (LoadInst *L1 = dyn_cast<LoadInst>(Right[j + 1])) {
        if (isConsecutiveAccess(L, L1, DL)) {
        }
      }
    }
  }
}
void BoUpSLP::setInsertPointAfterBundle(ArrayRef<Value *> VL) {
}
Value *BoUpSLP::Gather(ArrayRef<Value *> VL, VectorType *Ty) {
  Value *Vec = UndefValue::get(Ty);
  for (unsigned i = 0; i < Ty->getNumElements(); ++i) {
    if (Instruction *Insrt = dyn_cast<Instruction>(Vec)) {
      if (ScalarToTreeEntry.count(VL[i])) {
        int Idx = ScalarToTreeEntry[VL[i]];
        TreeEntry *E = &VectorizableTree[Idx];
        for (unsigned Lane = 0, LE = VL.size(); Lane != LE; ++Lane) {
          if (E->Scalars[Lane] == VL[i]) {
          }
        }
      }
    }
  }
  if (ScalarToTreeEntry.count(VL[0])) {
  }
  Type *ScalarTy = VL[0]->getType();
  if (StoreInst *SI = dyn_cast<StoreInst>(VL[0]))
    ScalarTy = SI->getValueOperand()->getType();
}
Value *BoUpSLP::vectorizeTree(TreeEntry *E) {
  if (E->VectorizedValue) {
  }
  Instruction *VL0 = cast<Instruction>(E->Scalars[0]);
  if (E->NeedToGather) {
  }
  unsigned Opcode = getSameOpcode(E->Scalars);
  switch (Opcode) {
    case Instruction::PHI: {
      PHINode *PH = dyn_cast<PHINode>(VL0);
      SmallSet<BasicBlock*, 4> VisitedBBs;
      for (unsigned i = 0, e = PH->getNumIncomingValues(); i < e; ++i) {
        BasicBlock *IBB = PH->getIncomingBlock(i);
        if (!VisitedBBs.insert(IBB).second) {
        }
      }
      if (CanReuseExtract(E->Scalars)) {
      }
    }
    case Instruction::BitCast: {
      if (Value *V = alreadyVectorized(E->Scalars))
        return V;
    }
    case Instruction::ICmp: {
      for (Value *V : E->Scalars) {
      }
      for (Value *V : E->Scalars) {
      }
    }
    case Instruction::Xor: {
    }
    case Instruction::Load: {
      ValueList Op0VL;
      for (Value *V : E->Scalars)
        Op0VL.push_back(cast<GetElementPtrInst>(V)->getOperand(0));
      Value *Op0 = vectorizeTree(Op0VL);
      std::vector<Value *> OpVecs;
      for (int j = 1, e = cast<GetElementPtrInst>(VL0)->getNumOperands(); j < e;
           ++j) {
      }
      Value *V = Builder.CreateGEP(
          cast<GetElementPtrInst>(VL0)->getSourceElementType(), Op0, OpVecs);
    }
    case Instruction::Call: {
      CallInst *CI = cast<CallInst>(VL0);
      Function *FI;
      Intrinsic::ID IID  = Intrinsic::not_intrinsic;
      if (CI && (FI = CI->getCalledFunction())) {
      }
      for (int j = 0, e = CI->getNumArgOperands(); j < e; ++j) {
        if (hasVectorInstrinsicScalarOpd(IID, 1) && j == 1) {
        }
        for (Value *V : E->Scalars) {
        }
      }
    }
    case Instruction::ShuffleVector: {
      unsigned e = E->Scalars.size();
      for (unsigned i = 0; i < e; ++i) {
        if (i & 1) {
        }
      }
    }
  }
}
Value *BoUpSLP::vectorizeTree() {
  for (UserList::iterator it = ExternalUses.begin(), e = ExternalUses.end();
       it != e; ++it) {
    Value *Scalar = it->Scalar;
    llvm::User *User = it->User;
    int Idx = ScalarToTreeEntry[Scalar];
    TreeEntry *E = &VectorizableTree[Idx];
    Value *Vec = E->VectorizedValue;
    if (isa<Instruction>(Vec)){
      if (PHINode *PH = dyn_cast<PHINode>(User)) {
        for (int i = 0, e = PH->getNumIncomingValues(); i != e; ++i) {
          if (PH->getIncomingValue(i) == Scalar) {
          }
        }
     }
    }
  }
  for (int EIdx = 0, EE = VectorizableTree.size(); EIdx < EE; ++EIdx) {
    TreeEntry *Entry = &VectorizableTree[EIdx];
    for (int Lane = 0, LE = Entry->Scalars.size(); Lane != LE; ++Lane) {
      Value *Scalar = Entry->Scalars[Lane];
      Type *Ty = Scalar->getType();
      if (!Ty->isVoidTy()) {
#ifndef NDEBUG
#endif
      }
    }
  }
}
void BoUpSLP::optimizeGatherSequence() {
  DEBUG(dbgs() << "SLP: Optimizing " << GatherSeq.size()
        << " gather sequences instructions.\n");
  for (SetVector<Instruction *>::iterator it = GatherSeq.begin(),
       e = GatherSeq.end(); it != e; ++it) {
  }
  SmallVector<const DomTreeNode *, 8> CSEWorkList;
  for (BasicBlock *BB : CSEBlocks)
    if (DomTreeNode *N = DT->getNode(BB)) {
    }
  std::stable_sort(CSEWorkList.begin(), CSEWorkList.end(),
                   [this](const DomTreeNode *A, const DomTreeNode *B) {
    return DT->properlyDominates(A, B);
  });
  SmallVector<Instruction *, 16> Visited;
  for (auto I = CSEWorkList.begin(), E = CSEWorkList.end(); I != E; ++I) {
    assert((I == CSEWorkList.begin() || !DT->dominates(*I, *std::prev(I))) &&
           "Worklist not sorted properly!");
    BasicBlock *BB = (*I)->getBlock();
    for (BasicBlock::iterator it = BB->begin(), e = BB->end(); it != e;) {
      Instruction *In = it++;
      for (SmallVectorImpl<Instruction *>::iterator v = Visited.begin(),
                                                    ve = Visited.end();
           v != ve; ++v) {
        if (In->isIdenticalTo(*v) &&
            DT->dominates((*v)->getParent(), In->getParent())) {
        }
      }
      if (In) {
      }
    }
  }
}
bool BoUpSLP::BlockScheduling::tryScheduleBundle(ArrayRef<Value *> VL,
                                                 BoUpSLP *SLP) {
  Instruction *OldScheduleEnd = ScheduleEnd;
  ScheduleData *Bundle = nullptr;
  for (Value *V : VL) {
    ScheduleData *BundleMember = getScheduleData(V);
    if (BundleMember->IsScheduled) {
    }
  }
  if (ScheduleEnd != OldScheduleEnd) {
    for (auto *I = ScheduleStart; I != ScheduleEnd; I = I->getNextNode()) {
    }
  }
  while (!Bundle->isReady() && !ReadyInsts.empty()) {
    ScheduleData *pickedSD = ReadyInsts.back();
    if (pickedSD->isSchedulingEntity() && pickedSD->isReady()) {
    }
  }
}
void BoUpSLP::BlockScheduling::cancelScheduling(ArrayRef<Value *> VL) {
  ScheduleData *Bundle = getScheduleData(VL[0]);
  ScheduleData *BundleMember = Bundle;
  while (BundleMember) {
  }
}
void BoUpSLP::BlockScheduling::extendSchedulingRegion(Value *V) {
  Instruction *I = dyn_cast<Instruction>(V);
  if (!ScheduleStart) {
  }
  BasicBlock::reverse_iterator UpIter(ScheduleStart);
  BasicBlock::reverse_iterator UpperEnd = BB->rend();
  BasicBlock::iterator DownIter(ScheduleEnd);
  BasicBlock::iterator LowerEnd = BB->end();
  for (;;) {
    if (UpIter != UpperEnd) {
      if (&*UpIter == I) {
      }
    }
    if (DownIter != LowerEnd) {
      if (&*DownIter == I) {
      }
    }
    assert((UpIter != UpperEnd || DownIter != LowerEnd) &&
           "instruction not found in block");
  }
}
void BoUpSLP::BlockScheduling::initScheduleData(Instruction *FromI,
                                                Instruction *ToI,
                                                ScheduleData *PrevLoadStore,
                                                ScheduleData *NextLoadStore) {
  ScheduleData *CurrentLoadStore = PrevLoadStore;
  for (Instruction *I = FromI; I != ToI; I = I->getNextNode()) {
    ScheduleData *SD = ScheduleDataMap[I];
    if (!SD) {
      if (CurrentLoadStore) {
      }
    }
  }
  if (NextLoadStore) {
  }
}
void BoUpSLP::BlockScheduling::calculateDependencies(ScheduleData *SD,
                                                     bool InsertInReadyList,
                                                     BoUpSLP *SLP) {
  SmallVector<ScheduleData *, 10> WorkList;
  while (!WorkList.empty()) {
    ScheduleData *BundleMember = SD;
    while (BundleMember) {
      if (!BundleMember->hasValidDependencies()) {
        for (User *U : BundleMember->Inst->users()) {
          if (isa<Instruction>(U)) {
            ScheduleData *UseSD = getScheduleData(U);
            if (UseSD && isInSchedulingRegion(UseSD->FirstInBundle)) {
              ScheduleData *DestBundle = UseSD->FirstInBundle;
              if (!DestBundle->IsScheduled) {
              }
            }
          }
        }
        ScheduleData *DepDest = BundleMember->NextLoadStore;
        if (DepDest) {
          Instruction *SrcInst = BundleMember->Inst;
          MemoryLocation SrcLoc = getLocation(SrcInst, SLP->AA);
          bool SrcMayWrite = BundleMember->Inst->mayWriteToMemory();
          unsigned numAliased = 0;
          unsigned DistToSrc = 1;
          while (DepDest) {
            if (DistToSrc >= MaxMemDepDistance ||
                    ((SrcMayWrite || DepDest->Inst->mayWriteToMemory()) &&
                     (numAliased >= AliasedCheckLimit ||
                      SLP->isAliased(SrcLoc, SrcInst, DepDest->Inst)))) {
              ScheduleData *DestBundle = DepDest->FirstInBundle;
              if (!DestBundle->IsScheduled) {
              }
              if (!DestBundle->hasValidDependencies()) {
              }
            }
          }
        }
      }
    }
  }
}
void BoUpSLP::BlockScheduling::resetSchedule() {
  assert(ScheduleStart &&
         "tried to reset schedule on block which has not been scheduled");
  for (Instruction *I = ScheduleStart; I != ScheduleEnd; I = I->getNextNode()) {
  }
}
void BoUpSLP::scheduleBlock(BlockScheduling *BS) {
  struct ScheduleDataCompare {
    bool operator()(ScheduleData *SD1, ScheduleData *SD2) {
    }
  };
  std::set<ScheduleData *, ScheduleDataCompare> ReadyInsts;
  for (auto *I = BS->ScheduleStart; I != BS->ScheduleEnd;
       I = I->getNextNode()) {
  }
  Instruction *LastScheduledInst = BS->ScheduleEnd;
  while (!ReadyInsts.empty()) {
    ScheduleData *picked = *ReadyInsts.begin();
    ScheduleData *BundleMember = picked;
    while (BundleMember) {
      Instruction *pickedInst = BundleMember->Inst;
      if (LastScheduledInst->getNextNode() != pickedInst) {
      }
    }
  }
}
struct SLPVectorizer : public FunctionPass {
  typedef SmallVector<StoreInst *, 8> StoreList;
  typedef MapVector<Value *, StoreList> StoreListMap;
  static char ID;
  explicit SLPVectorizer() : FunctionPass(ID) {
  }
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
    bool Changed = false;
    if (!TTI->getNumberOfRegisters(true))
      return false;
    BoUpSLP R(&F, SE, TTI, TLI, AA, LI, DT, AC);
    for (auto BB : post_order(&F.getEntryBlock())) {
      if (unsigned count = collectStores(BB, R)) {
      }
      Changed |= vectorizeChainsInBlock(BB, R);
    }
    if (Changed) {
    }
  }
  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<ScalarEvolution>();
    AU.addRequired<TargetTransformInfoWrapperPass>();
  }
  unsigned collectStores(BasicBlock *BB, BoUpSLP &R);
  bool tryToVectorizePair(Value *A, Value *B, BoUpSLP &R);
  bool tryToVectorizeList(ArrayRef<Value *> VL, BoUpSLP &R,
                          ArrayRef<Value *> BuildVector = None,
                          bool allowReorder = false);
  bool tryToVectorize(BinaryOperator *V, BoUpSLP &R);
  bool vectorizeStoreChains(BoUpSLP &R);
  bool vectorizeChainsInBlock(BasicBlock *BB, BoUpSLP &R);
  bool vectorizeStoreChain(ArrayRef<Value *> Chain, int CostThreshold,
                           BoUpSLP &R, unsigned VecRegSize);
  bool vectorizeStores(ArrayRef<StoreInst *> Stores, int costThreshold,
                       BoUpSLP &R);
  StoreListMap StoreRefs;
};
static bool hasValueBeenRAUWed(ArrayRef<Value *> VL, ArrayRef<WeakVH> VH,
                               unsigned SliceBegin, unsigned SliceSize) {
}
bool SLPVectorizer::vectorizeStoreChain(ArrayRef<Value *> Chain,
                                        int CostThreshold, BoUpSLP &R,
                                        unsigned VecRegSize) {
  unsigned ChainLen = Chain.size();
  for (unsigned i = 0, e = ChainLen; i < e; ++i) {
    int Cost = R.getTreeCost();
    if (Cost < CostThreshold) {
    }
  }
}
bool SLPVectorizer::vectorizeStores(ArrayRef<StoreInst *> Stores,
                                    int costThreshold, BoUpSLP &R) {
  SetVector<StoreInst *> Heads, Tails;
  for (unsigned i = 0, e = Stores.size(); i < e; ++i) {
    for (unsigned j = 0; j < e; ++j) {
      const DataLayout &DL = Stores[i]->getModule()->getDataLayout();
      if (R.isConsecutiveAccess(Stores[i], Stores[j], DL)) {
      }
    }
  }
  for (SetVector<StoreInst *>::iterator it = Heads.begin(), e = Heads.end();
       it != e; ++it) {
    StoreInst *I = *it;
    while (Tails.count(I) || Heads.count(I)) {
    }
  }
}
unsigned SLPVectorizer::collectStores(BasicBlock *BB, BoUpSLP &R) {
  for (Instruction &I : *BB) {
  }
}
bool SLPVectorizer::tryToVectorizePair(Value *A, Value *B, BoUpSLP &R) {
  if (!A || !B)
    return false;
  Value *VL[] = { A, B };
  return tryToVectorizeList(VL, R, None, true);
}
bool SLPVectorizer::tryToVectorizeList(ArrayRef<Value *> VL, BoUpSLP &R,
                                       ArrayRef<Value *> BuildVector,
                                       bool allowReorder) {
  Instruction *I0 = dyn_cast<Instruction>(VL[0]);
  if (!I0)
    return false;
  for (unsigned i = 0, e = VL.size(); i < e; ++i) {
    unsigned OpsWidth = 0;
    DEBUG(dbgs() << "SLP: Analyzing " << OpsWidth << " operations "
                 << "\n");
    ArrayRef<Value *> Ops = VL.slice(i, OpsWidth);
    ArrayRef<Value *> BuildVectorSlice;
    if (allowReorder && R.shouldReorder()) {
      Value *ReorderedOps[] = { Ops[1], Ops[0] };
      R.buildTree(ReorderedOps, None);
    }
    int Cost = R.getTreeCost();
    if (Cost < -SLPCostThreshold) {
      if (!BuildVectorSlice.empty()) {
        Instruction *InsertAfter = cast<Instruction>(BuildVectorSlice.back());
        for (auto &V : BuildVectorSlice) {
          IRBuilder<true, NoFolder> Builder(
              ++BasicBlock::iterator(InsertAfter));
        }
      }
    }
  }
}
bool SLPVectorizer::tryToVectorize(BinaryOperator *V, BoUpSLP &R) {
  BinaryOperator *A = dyn_cast<BinaryOperator>(V->getOperand(0));
  BinaryOperator *B = dyn_cast<BinaryOperator>(V->getOperand(1));
  if (B && B->hasOneUse()) {
    BinaryOperator *B0 = dyn_cast<BinaryOperator>(B->getOperand(0));
    BinaryOperator *B1 = dyn_cast<BinaryOperator>(B->getOperand(1));
    if (tryToVectorizePair(A, B0, R)) {
    }
    if (tryToVectorizePair(A, B1, R)) {
    }
  }
  if (A && A->hasOneUse()) {
    BinaryOperator *A0 = dyn_cast<BinaryOperator>(A->getOperand(0));
    BinaryOperator *A1 = dyn_cast<BinaryOperator>(A->getOperand(1));
    if (tryToVectorizePair(A0, B, R)) {
    }
    if (tryToVectorizePair(A1, B, R)) {
    }
  }
}
static Value *createRdxShuffleMask(unsigned VecLen, unsigned NumEltsToRdx,
                                   bool IsPairwise, bool IsLeft,
                                   IRBuilder<> &Builder) {
  SmallVector<Constant *, 32> ShuffleMask(
      VecLen, UndefValue::get(Builder.getInt32Ty()));
}
class HorizontalReduction {
  SmallVector<Value *, 32> ReducedVals;
  BinaryOperator *ReductionRoot;
  PHINode *ReductionPHI;
  unsigned ReductionOpcode;
  unsigned ReduxWidth;
  bool IsPairwiseReduction;
public:
  bool matchAssociativeReduction(PHINode *Phi, BinaryOperator *B) {
    assert((!Phi ||
            std::find(Phi->op_begin(), Phi->op_end(), B) != Phi->op_end()) &&
           "Thi phi needs to use the binary operator");
    if (Phi) {
      if (B->getOperand(0) == Phi) {
      }
    }
    if (ReductionOpcode != Instruction::Add &&
        ReductionOpcode != Instruction::FAdd)
      return false;
    SmallVector<std::pair<BinaryOperator *, unsigned>, 32> Stack;
    while (!Stack.empty()) {
      BinaryOperator *TreeN = Stack.back().first;
      unsigned EdgeToVist = Stack.back().second++;
      bool IsReducedValue = TreeN->getOpcode() != ReductionOpcode;
      if (EdgeToVist == 2 || IsReducedValue) {
        if (IsReducedValue) {
        }
      }
    }
  }
  bool tryToReduce(BoUpSLP &V, TargetTransformInfo *TTI) {
    unsigned NumReducedVals = ReducedVals.size();
    Value *VectorizedTree = nullptr;
    IRBuilder<> Builder(ReductionRoot);
    unsigned i = 0;
    for (; i < NumReducedVals - ReduxWidth + 1; i += ReduxWidth) {
      Value *VectorizedRoot = V.vectorizeTree();
      Value *ReducedSubTree = emitReduction(VectorizedRoot, Builder);
      if (VectorizedTree) {
        VectorizedTree = createBinOp(Builder, ReductionOpcode, VectorizedTree,
                                     ReducedSubTree, "bin.rdx");
      } else
        VectorizedTree = ReducedSubTree;
    }
    if (VectorizedTree) {
      for (; i < NumReducedVals; ++i) {
      }
      if (ReductionPHI) {
      } else
        ReductionRoot->replaceAllUsesWith(VectorizedTree);
    }
  }
  int getReductionCost(TargetTransformInfo *TTI, Value *FirstReducedVal) {
    DEBUG(dbgs() << "SLP: Adding cost " << VecReduxCost - ScalarReduxCost
                 << " reduction)\n");
  }
  static Value *createBinOp(IRBuilder<> &Builder, unsigned Opcode, Value *L,
                            Value *R, const Twine &Name = "") {
  }
  Value *emitReduction(Value *VectorizedValue, IRBuilder<> &Builder) {
    assert(isPowerOf2_32(ReduxWidth) &&
           "We only handle power-of-two reductions for now");
    Value *TmpVec = VectorizedValue;
    for (unsigned i = ReduxWidth / 2; i != 0; i >>= 1) {
      if (IsPairwiseReduction) {
        Value *RightMask =
          createRdxShuffleMask(ReduxWidth, i, true, false, Builder);
        Value *RightShuf = Builder.CreateShuffleVector(
          TmpVec, UndefValue::get(TmpVec->getType()), (RightMask),
                             "bin.rdx");
        Value *UpperHalf =
          createRdxShuffleMask(ReduxWidth, i, false, false, Builder);
        Value *Shuf = Builder.CreateShuffleVector(
          TmpVec, UndefValue::get(TmpVec->getType()), UpperHalf, "rdx.shuf");
      }
    }
  }
};
static bool findBuildVector(InsertElementInst *FirstInsertElem,
                            SmallVectorImpl<Value *> &BuildVectorOpds) {
  while (true) {
  }
}
static bool PhiTypeSorterFunc(Value *V, Value *V2) {
}
bool SLPVectorizer::vectorizeChainsInBlock(BasicBlock *BB, BoUpSLP &R) {
  SmallVector<Value *, 4> Incoming;
  bool HaveVectorizedPhiNodes = true;
  while (HaveVectorizedPhiNodes) {
    HaveVectorizedPhiNodes = false;
    for (BasicBlock::iterator instr = BB->begin(), ie = BB->end(); instr != ie;
         ++instr) {
    }
    for (SmallVector<Value *, 4>::iterator IncIt = Incoming.begin(),
                                           E = Incoming.end();
         IncIt != E;) {
    }
  }
  for (BasicBlock::iterator it = BB->begin(), e = BB->end(); it != e; it++) {
    if (PHINode *P = dyn_cast<PHINode>(it)) {
      Value *Rdx =
          (P->getIncomingBlock(0) == BB
               ? (P->getIncomingValue(0))
               : (P->getIncomingBlock(1) == BB ? P->getIncomingValue(1)
                                               : nullptr));
      BinaryOperator *BI = dyn_cast_or_null<BinaryOperator>(Rdx);
     Value *Inst = BI->getOperand(0);
      if (tryToVectorize(dyn_cast<BinaryOperator>(Inst), R)) {
      }
    }
      if (StoreInst *SI = dyn_cast<StoreInst>(it))
        if (BinaryOperator *BinOp =
                dyn_cast<BinaryOperator>(SI->getValueOperand())) {
          HorizontalReduction HorRdx;
          if (((HorRdx.matchAssociativeReduction(nullptr, BinOp) &&
                HorRdx.tryToReduce(R, TTI)) ||
               tryToVectorize(BinOp, R))) {
          }
        }
    if (ReturnInst *RI = dyn_cast<ReturnInst>(it))
        if (BinaryOperator *BinOp =
                dyn_cast<BinaryOperator>(RI->getOperand(0))) {
          if (tryToVectorizePair(BinOp->getOperand(0),
                                 BinOp->getOperand(1), R)) {
          }
        }
    if (CmpInst *CI = dyn_cast<CmpInst>(it)) {
      if (tryToVectorizePair(CI->getOperand(0), CI->getOperand(1), R)) {
      }
      for (int i = 0; i < 2; ++i) {
        if (BinaryOperator *BI = dyn_cast<BinaryOperator>(CI->getOperand(i))) {
          if (tryToVectorizePair(BI->getOperand(0), BI->getOperand(1), R)) {
          }
        }
      }
    }
    if (InsertElementInst *FirstInsertElem = dyn_cast<InsertElementInst>(it)) {
      SmallVector<Value *, 16> BuildVector;
      SmallVector<Value *, 16> BuildVectorOpds;
      if (tryToVectorizeList(BuildVectorOpds, R, BuildVector)) {
      }
    }
  }
}
bool SLPVectorizer::vectorizeStoreChains(BoUpSLP &R) {
  for (StoreListMap::iterator it = StoreRefs.begin(), e = StoreRefs.end();
       it != e; ++it) {
    for (unsigned CI = 0, CE = it->second.size(); CI < CE; CI+=16) {
    }
  }
}
} // end anonymous namespace
char SLPVectorizer::ID = 0;
static const char lv_name[] = "SLP Vectorizer";
INITIALIZE_PASS_BEGIN(SLPVectorizer, SV_NAME, lv_name, false, false)
INITIALIZE_PASS_END(SLPVectorizer, SV_NAME, lv_name, false, false)
namespace llvm {
Pass *createSLPVectorizerPass() { return new SLPVectorizer(); }
}
