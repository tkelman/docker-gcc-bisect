#!/bin/bash -e
/usr/bin/i686-w64-mingw32-g++ -DGTEST_HAS_RTTI=0 -D__STDC_CONSTANT_MACROS \
  -D__STDC_FORMAT_MACROS -D__STDC_LIMIT_MACROS -Wall -W -Wno-unused-parameter \
  -Wwrite-strings -Wcast-qual -Wno-missing-field-initializers -pedantic \
  -Wno-long-long -Wno-maybe-uninitialized -Wnon-virtual-dtor -Wno-comment \
  -std=gnu++11 -O2 -DNDEBUG -fno-exceptions -fno-rtti -MMD -MT opt_singlefile.cpp \
  -o opt_good.exe -fno-ipa-cp -Wl,--major-image-version,0,--minor-image-version,0 \
  -lpsapi -lshell32 -lole32 -lkernel32 -luser32 -lgdi32 -lwinspool -lshell32 \
  -lole32 -loleaut32 -luuid -lcomdlg32 -ladvapi32
cp /usr/i686-w64-mingw32/sys-root/mingw/bin/*.dll .
[[ "$(timeout 20 wine opt_good.exe -slp-vectorizer \
  -S /opt/llvm/llvm-3.7.1.src/test/Transforms/SLPVectorizer/X86/vector.ll)" = \
"; ModuleID = '/opt/llvm/llvm-3.7.1.src/test/Transforms/SLPVectorizer/X86/vector.ll'
target datalayout = \"e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128\"
target triple = \"x86_64-apple-macosx10.8.0\"

define void @test(<4 x i32> %in, <4 x i32> %in2) {
  %k = icmp eq <4 x i32> %in, %in2
  ret void
}" ]]
/usr/bin/i686-w64-mingw32-g++ -DGTEST_HAS_RTTI=0 -D__STDC_CONSTANT_MACROS \
  -D__STDC_FORMAT_MACROS -D__STDC_LIMIT_MACROS -Wall -W -Wno-unused-parameter \
  -Wwrite-strings -Wcast-qual -Wno-missing-field-initializers -pedantic \
  -Wno-long-long -Wno-maybe-uninitialized -Wnon-virtual-dtor -Wno-comment \
  -std=gnu++11 -O2 -DNDEBUG -fno-exceptions -fno-rtti -MMD -MT opt_singlefile.cpp \
  -o opt_bad.exe -Wl,--major-image-version,0,--minor-image-version,0 \
  -lpsapi -lshell32 -lole32 -lkernel32 -luser32 -lgdi32 -lwinspool -lshell32 \
  -lole32 -loleaut32 -luuid -lcomdlg32 -ladvapi32
! timeout 20 wine opt_bad.exe -slp-vectorizer \
  -S /opt/llvm/llvm-3.7.1.src/test/Transforms/SLPVectorizer/X86/vector.ll
