FROM opensuse:42.2

# Install build deps
RUN zypper addrepo obs://windows:mingw:win32/openSUSE_42.2 mingw32
RUN zypper addrepo obs://Emulators/openSUSE_Leap_42.2 Emulators
# wine depends on samba which depends on krb5 which conflicts with krb5-mini
RUN zypper -n --gpg-auto-import-keys install git make flex tar curl wine -krb5-mini
RUN zypper -n install mingw32-cross-gcc-c++ mingw32-libstdc++6 gcc-c++ cmake ninja

# Download sources
RUN mkdir -p /opt/llvm/build01 && cd /opt/llvm && \
    curl -L http://llvm.org/releases/3.7.1/llvm-3.7.1.src.tar.xz | tar -xJf -

WORKDIR /opt/llvm/build01
RUN rm -rf /opt/llvm/build01/* && mkdir -p bin && \
    cp /usr/i686-w64-mingw32/sys-root/mingw/bin/*.dll bin && \
    echo 'set(CMAKE_C_COMPILER cc)' > NATIVE.cmake && \
    echo 'set(CMAKE_CXX_COMPILER c++)' >> NATIVE.cmake
RUN cmake /opt/llvm/llvm-3.7.1.src -DCMAKE_SYSTEM_NAME=Windows \
    -DCMAKE_BUILD_TYPE=Release -DLLVM_TARGETS_TO_BUILD=host \
    -DCMAKE_C_COMPILER=i686-w64-mingw32-gcc \
    -DCMAKE_CXX_COMPILER=i686-w64-mingw32-g++ \
    -DCROSS_TOOLCHAIN_FLAGS_NATIVE=-DCMAKE_TOOLCHAIN_FILE=$PWD/NATIVE.cmake \
    -DCMAKE_VERBOSE_MAKEFILE=TRUE -G Ninja
#-DCMAKE_CXX_FLAGS=-fno-ipa-cp
# CXXFLAGS=-fno-ipa-cp
#CXXFLAGS=-fno-devirtualize
# ONLY_TOOLS=opt to avoid hitting an unrelated gtest macro expansion
# error that would cause build failures at some points in the history
RUN ninja -v opt
RUN wine /opt/llvm/build01/bin/opt.exe -slp-vectorizer \
    -S /opt/llvm/llvm-3.7.1.src/test/Transforms/SLPVectorizer/X86/vector.ll
