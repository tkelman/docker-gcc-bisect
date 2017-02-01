FROM opensuse:42.1

# Install build deps
# wine depends on samba which depends on krb5 which conflicts with krb5-mini
RUN zypper addrepo obs://windows:mingw:win32/openSUSE_42.1 mingw32 && \
    zypper addrepo obs://Emulators/openSUSE_Leap_42.1 Emulators && \
    zypper addrepo obs://devel:tools/openSUSE_Leap_42.1 develtools && \
    zypper -n --gpg-auto-import-keys install git make flex tar curl wine -krb5-mini \
    mingw32-cross-gcc-c++ mingw32-libstdc++6 clang cmake ninja creduce patch

# Download sources
RUN mkdir -p /opt/llvm/build && cd /opt/llvm && \
    curl -L http://llvm.org/releases/3.7.1/llvm-3.7.1.src.tar.xz | tar -xJf -
# Patch for C++ compatibility so everything can be built in one g++ invocation
WORKDIR /opt/llvm/llvm-3.7.1.src
ADD llvm-cppcompat.patch .
RUN patch -p1 < llvm-cppcompat.patch
# Put mingw runtime dlls in place, and prepare toolchain file
RUN cp /usr/i686-w64-mingw32/sys-root/mingw/bin/*.dll /opt/llvm/build && \
    echo 'set(CMAKE_C_COMPILER clang)' > /opt/llvm/NATIVE.cmake && \
    echo 'set(CMAKE_CXX_COMPILER clang++)' >> /opt/llvm/NATIVE.cmake
WORKDIR /opt/llvm/build
ADD build.sh .
RUN ./build.sh
ADD filelist.txt .
ADD test.sh .
RUN delta -verbose -in_place -test=./test.sh filelist.txt && \
    cat filelist.txt
