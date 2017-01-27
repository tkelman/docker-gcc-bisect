FROM opensuse:42.1

# Install build deps
RUN zypper addrepo obs://windows:mingw:win32/openSUSE_42.1 mingw32
RUN zypper addrepo obs://Emulators/openSUSE_Leap_42.1 Emulators
RUN zypper addrepo obs://devel:tools/openSUSE_Leap_42.1 develtools
# wine depends on samba which depends on krb5 which conflicts with krb5-mini
RUN zypper -n --gpg-auto-import-keys install git make flex tar curl wine -krb5-mini \
    mingw32-cross-gcc-c++ mingw32-libstdc++6 clang cmake ninja creduce

# Download sources
RUN mkdir -p /opt/llvm/build-good /opt/llvm/build-bad && cd /opt/llvm && \
    curl -L http://llvm.org/releases/3.7.1/llvm-3.7.1.src.tar.xz | tar -xJf -

WORKDIR /opt/llvm/build-good
RUN rm -rf /opt/llvm/build-good/* && mkdir -p bin && \
    cp /usr/i686-w64-mingw32/sys-root/mingw/bin/*.dll bin && \
    echo 'set(CMAKE_C_COMPILER clang)' > NATIVE.cmake && \
    echo 'set(CMAKE_CXX_COMPILER clang++)' >> NATIVE.cmake
RUN cmake /opt/llvm/llvm-3.7.1.src -DCMAKE_SYSTEM_NAME=Windows \
    -DCMAKE_BUILD_TYPE=Release -DLLVM_TARGETS_TO_BUILD=host \
    -DCMAKE_C_COMPILER=i686-w64-mingw32-gcc \
    -DCMAKE_CXX_COMPILER=i686-w64-mingw32-g++ \
    -DCROSS_TOOLCHAIN_FLAGS_NATIVE=-DCMAKE_TOOLCHAIN_FILE=$PWD/NATIVE.cmake \
    -DCMAKE_VERBOSE_MAKEFILE=TRUE -G Ninja -DCMAKE_CXX_FLAGS=-fno-ipa-cp
RUN ninja -v opt
RUN wine /opt/llvm/build-good/bin/opt.exe -slp-vectorizer \
    -S /opt/llvm/llvm-3.7.1.src/test/Transforms/SLPVectorizer/X86/vector.ll

WORKDIR /opt/llvm/build-bad
RUN rm -rf /opt/llvm/build-bad/* && mkdir -p bin && \
    cp /usr/i686-w64-mingw32/sys-root/mingw/bin/*.dll bin && \
    echo 'set(CMAKE_C_COMPILER clang)' > NATIVE.cmake && \
    echo 'set(CMAKE_CXX_COMPILER clang++)' >> NATIVE.cmake
RUN cmake /opt/llvm/llvm-3.7.1.src -DCMAKE_SYSTEM_NAME=Windows \
    -DCMAKE_BUILD_TYPE=Release -DLLVM_TARGETS_TO_BUILD=host \
    -DCMAKE_C_COMPILER=i686-w64-mingw32-gcc \
    -DCMAKE_CXX_COMPILER=i686-w64-mingw32-g++ \
    -DCROSS_TOOLCHAIN_FLAGS_NATIVE=-DCMAKE_TOOLCHAIN_FILE=$PWD/NATIVE.cmake \
    -DCMAKE_VERBOSE_MAKEFILE=TRUE -G Ninja
RUN ninja -v opt

RUN wine /opt/llvm/build-good/bin/opt.exe -slp-vectorizer \
    -S /opt/llvm/llvm-3.7.1.src/test/Transforms/SLPVectorizer/X86/vector.ll > \
    /opt/llvm/build-good/good-output.txt

RUN touch /opt/llvm/llvm-3.7.1.src/lib/Transforms/Vectorize/SLPVectorizer.cpp && \
    cmd=$(ninja -v opt | grep '\[2/4\]' | sed 's|\[2/4\]||') && \
    $cmd -save-temps && \
    cp SLPVectorizer.ii /opt/llvm/llvm-3.7.1.src/lib/Transforms/Vectorize/SLPVectorizer.cpp

WORKDIR /opt/llvm/llvm-3.7.1.src/lib/Transforms/Vectorize
RUN echo '#!/bin/bash -e' > script.sh && chmod +x script.sh && \
    echo 'cd /opt/llvm/build-good && ninja opt' >> script.sh && \
    echo 'cd /opt/llvm/build-bad && ninja opt' >> script.sh && \
    echo 'cd /opt/llvm/llvm-3.7.1.src/lib/Transforms/Vectorize' >> script.sh && \
    echo 'rm -rf /tmp/.wine*' >> script.sh && \
    echo 'diff /opt/llvm/build-good/good-output.txt <(timeout 20 wine /opt/llvm/build-good/bin/opt.exe -slp-vectorizer -S /opt/llvm/llvm-3.7.1.src/test/Transforms/SLPVectorizer/X86/vector.ll)' >> script.sh && \
    echo '! timeout 20 wine /opt/llvm/build-bad/bin/opt.exe -slp-vectorizer -S /opt/llvm/llvm-3.7.1.src/test/Transforms/SLPVectorizer/X86/vector.ll' >> script.sh
RUN cd /opt/llvm/llvm-3.7.1.src/lib/Transforms/Vectorize && \
    creduce --verbose --timing ./script.sh SLPVectorizer.cpp
#    delta -verbose -in_place -test=./script.sh SLPVectorizer.cpp && \
#    cat SLPVectorizer.cpp

