#!/bin/sh
set -e
#git bisect log
git log -1
#if [ -e gcc/ipa-polymorphic-call.c ]; then
  # apply patch from SVN rev 220545 early to avoid internal compiler error
#  git show 41a1ab48345 gcc/ipa-polymorphic-call.c | git apply || true
#fi
cd /opt/gcc/build
rm -rf /opt/gcc/build/* /opt/gcc/usr
/opt/gcc/src/configure --prefix=/opt/gcc/usr \
  --target=i686-w64-mingw32 --enable-languages="c,c++" \
  --disable-multilib --with-sysroot=/usr/i686-w64-mingw32/sys-root \
  --with-as=/usr/bin/i686-w64-mingw32-as > gcc-conf.log #2>&1 || exit 125
make -j`nproc` > gcc-build.log #2>&1 || exit 125
make install > gcc-inst.log #2>&1 || exit 125
export PATH=/opt/gcc/usr/bin:$PATH

#cd /opt/gcc/src
#if [ -e gcc/ipa-polymorphic-call.c ]; then
  # put ipa-polymorphic-call.c back the way it was in the original history
#  git checkout -- gcc/ipa-polymorphic-call.c
#fi

cd /opt/llvm/build
rm -rf /opt/llvm/build/*
mkdir -p Release+Asserts/bin
cp /usr/i686-w64-mingw32/sys-root/mingw/bin/*.dll Release+Asserts/bin
cp /opt/gcc/usr/i686-w64-mingw32/lib/*.dll Release+Asserts/bin
/opt/llvm/llvm-3.7.1.src/configure --host=i686-w64-mingw32 \
  --enable-optimized --enable-targets=host #> llvm-conf.log #2>&1 || exit 125
# ONLY_TOOLS=opt to avoid hitting an unrelated gtest macro expansion
# error that would cause build failures at some points in the history
make -j`nproc` CXXFLAGS=-fno-derivization ONLY_TOOLS=opt V=1 #> llvm-build.log || exit 125
wine /opt/llvm/build/Release+Asserts/bin/opt.exe -slp-vectorizer \
  -S /opt/llvm/llvm-3.7.1.src/test/Transforms/SLPVectorizer/X86/vector.ll || exit 1
