#!/bin/bash -e
pushd /opt/llvm/llvm-3.7.1.src/lib/Transforms/Vectorize
cp SLPVectorizer.cpp SLPVectorizer.previous
popd
cp SLPVectorizer.cpp /opt/llvm/llvm-3.7.1.src/lib/Transforms/Vectorize
cd /opt/llvm/build-good && ninja opt || \
  (cd /opt/llvm/llvm-3.7.1.src/lib/Transforms/Vectorize; cp SLPVectorizer.previous SLPVectorizer.cpp; exit 1)
cd /opt/llvm/build-bad && ninja opt || \
  (cd /opt/llvm/llvm-3.7.1.src/lib/Transforms/Vectorize; cp SLPVectorizer.previous SLPVectorizer.cpp; exit 1)
rm -rf /tmp/.wine*
diff /opt/llvm/build-good/good-output.txt <(timeout 20 wine /opt/llvm/build-good/bin/opt.exe -slp-vectorizer -S /opt/llvm/llvm-3.7.1.src/test/Transforms/SLPVectorizer/X86/vector.ll) || \
  (cd /opt/llvm/llvm-3.7.1.src/lib/Transforms/Vectorize; cp SLPVectorizer.previous SLPVectorizer.cpp; exit 1)
! timeout 20 wine /opt/llvm/build-bad/bin/opt.exe -slp-vectorizer -S /opt/llvm/llvm-3.7.1.src/test/Transforms/SLPVectorizer/X86/vector.ll || \
  (cd /opt/llvm/llvm-3.7.1.src/lib/Transforms/Vectorize; cp SLPVectorizer.previous SLPVectorizer.cpp; exit 1)
