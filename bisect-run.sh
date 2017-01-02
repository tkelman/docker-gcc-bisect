#!/bin/sh
set -e
git bisect log
git log -1

cd /opt/llvm/build
rm -rf /opt/llvm/build/*
mkdir -p bin
cp /usr/i686-w64-mingw32/sys-root/mingw/bin/*.dll bin
echo 'set(CMAKE_C_COMPILER cc)' > NATIVE.cmake
echo 'set(CMAKE_CXX_COMPILER c++)' >> NATIVE.cmake
cmake /opt/llvm/src -DCMAKE_SYSTEM_NAME=Windows \
  -DCMAKE_BUILD_TYPE=Release \
  -DCROSS_TOOLCHAIN_FLAGS_NATIVE=-DCMAKE_TOOLCHAIN_FILE=$PWD/NATIVE.cmake \
  -DCMAKE_C_COMPILER=i686-w64-mingw32-gcc \
  -DCMAKE_CXX_COMPILER=i686-w64-mingw32-g++ \
  -DLLVM_TARGETS_TO_BUILD=host > llvm-conf.log || exit 125
make -j`nproc` ONLY_TOOLS=opt > llvm-build.log || exit 125
! wine /opt/llvm/build/bin/opt.exe -slp-vectorizer \
  -S /opt/llvm/src/test/Transforms/SLPVectorizer/X86/vector.ll
