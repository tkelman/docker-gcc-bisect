FROM opensuse:42.1

# Install build deps
# wine depends on samba which depends on krb5 which conflicts with krb5-mini
RUN zypper addrepo obs://windows:mingw:win32/openSUSE_42.1 mingw32 && \
    zypper addrepo obs://Emulators/openSUSE_Leap_42.1 Emulators && \
    zypper addrepo obs://devel:tools/openSUSE_Leap_42.1 develtools && \
    zypper -n --gpg-auto-import-keys install git make flex tar curl wine -krb5-mini \
    mingw32-cross-gcc-c++ mingw32-libstdc++6 clang cmake ninja creduce python python-pip && \
    pip install scan-build

# Download sources
RUN mkdir -p /opt/llvm/build-good/bin /opt/llvm/build-bad/bin && cd /opt/llvm && \
    curl -L http://llvm.org/releases/3.7.1/llvm-3.7.1.src.tar.xz | tar -xJf -
# Put mingw runtime dlls in place, and prepare toolchain file
RUN cp /usr/i686-w64-mingw32/sys-root/mingw/bin/*.dll /opt/llvm/build-good/bin && \
    cp /usr/i686-w64-mingw32/sys-root/mingw/bin/*.dll /opt/llvm/build-bad/bin && \
    echo 'set(CMAKE_C_COMPILER clang)' > /opt/llvm/NATIVE.cmake && \
    echo 'set(CMAKE_CXX_COMPILER clang++)' >> /opt/llvm/NATIVE.cmake
# reduced copy of SLPVectorizer.cpp
RUN curl -L -o /opt/llvm/llvm-3.7.1.src/lib/Transforms/Vectorize/SLPVectorizer.cpp \
    https://raw.githubusercontent.com/tkelman/docker-gcc-bisect/master/Vectorize/SLPVectorizer.cpp

WORKDIR /opt/llvm/build-good
RUN cmake /opt/llvm/llvm-3.7.1.src -DCMAKE_SYSTEM_NAME=Windows \
    -DCMAKE_BUILD_TYPE=Release -DLLVM_TARGETS_TO_BUILD=host \
    -DCMAKE_C_COMPILER=i686-w64-mingw32-gcc \
    -DCMAKE_CXX_COMPILER=i686-w64-mingw32-g++ \
    -DCROSS_TOOLCHAIN_FLAGS_NATIVE=-DCMAKE_TOOLCHAIN_FILE=/opt/llvm/NATIVE.cmake \
    -G Ninja -DCMAKE_CXX_FLAGS=-fno-ipa-cp
RUN ninja -v opt
RUN wine /opt/llvm/build-good/bin/opt.exe -slp-vectorizer \
    -S /opt/llvm/llvm-3.7.1.src/test/Transforms/SLPVectorizer/X86/vector.ll

WORKDIR /opt/llvm/build-bad
RUN cmake /opt/llvm/llvm-3.7.1.src -DCMAKE_SYSTEM_NAME=Windows \
    -DCMAKE_BUILD_TYPE=Release -DLLVM_TARGETS_TO_BUILD=host \
    -DCMAKE_C_COMPILER=i686-w64-mingw32-gcc \
    -DCMAKE_CXX_COMPILER=i686-w64-mingw32-g++ \
    -DCROSS_TOOLCHAIN_FLAGS_NATIVE=-DCMAKE_TOOLCHAIN_FILE=/opt/llvm/NATIVE.cmake \
    -G Ninja
RUN ninja -v opt
RUN wine /opt/llvm/build-bad/bin/opt.exe -slp-vectorizer \
    -S /opt/llvm/llvm-3.7.1.src/test/Transforms/SLPVectorizer/X86/vector.ll

#RUN wine /opt/llvm/build-good/bin/opt.exe -slp-vectorizer \
#    -S /opt/llvm/llvm-3.7.1.src/test/Transforms/SLPVectorizer/X86/vector.ll > \
#    /opt/llvm/build-good/good-output.txt

#WORKDIR /opt/llvm/llvm-3.7.1.src/lib/Transforms/Vectorize
#RUN echo '#!/bin/bash -e' > script.sh && chmod +x script.sh && \
#    echo 'cd /opt/llvm/build-good && ninja opt' >> script.sh && \
#    echo 'cd /opt/llvm/build-bad && ninja opt' >> script.sh && \
#    echo 'cd /opt/llvm/llvm-3.7.1.src/lib/Transforms/Vectorize' >> script.sh && \
#    echo 'rm -rf /tmp/.wine*' >> script.sh && \
#    echo 'diff /opt/llvm/build-good/good-output.txt <(timeout 20 wine /opt/llvm/build-good/bin/opt.exe -slp-vectorizer -S /opt/llvm/llvm-3.7.1.src/test/Transforms/SLPVectorizer/X86/vector.ll)' >> script.sh && \
#    echo '! timeout 20 wine /opt/llvm/build-bad/bin/opt.exe -slp-vectorizer -S /opt/llvm/llvm-3.7.1.src/test/Transforms/SLPVectorizer/X86/vector.ll' >> script.sh
#RUN cd /opt/llvm/llvm-3.7.1.src/lib/Transforms/Vectorize && \
#    delta -verbose -in_place -test=./script.sh SLPVectorizer.cpp && \
#    cat SLPVectorizer.cpp

