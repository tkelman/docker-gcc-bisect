#!/bin/bash -e
cd /opt/llvm/build-good && ninja opt
cd /opt/llvm/build-bad && ninja opt
cd /opt/llvm/llvm-3.7.1.src/lib/Transforms/Vectorize
rm -rf /tmp/.wine*
diff /opt/llvm/build-good/good-output.txt <(timeout 20 wine /opt/llvm/build-good/bin/opt.exe -slp-vectorizer -S /opt/llvm/llvm-3.7.1.src/test/Transforms/SLPVectorizer/X86/vector.ll)
! timeout 20 wine /opt/llvm/build-bad/bin/opt.exe -slp-vectorizer -S /opt/llvm/llvm-3.7.1.src/test/Transforms/SLPVectorizer/X86/vector.ll
