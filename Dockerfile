FROM opensuse:42.2

# Install build deps
RUN zypper addrepo obs://windows:mingw:win32/openSUSE_42.2 mingw32
RUN zypper addrepo obs://Emulators/openSUSE_Leap_42.2 Emulators
# wine depends on samba which depends on krb5 which conflicts with krb5-mini
#RUN zypper -n --gpg-auto-import-keys install python samba -krb5-mini
RUN zypper -n --gpg-auto-import-keys install git make flex tar curl wine -krb5-mini
RUN zypper -n install mingw32-cross-gcc-c++

# Download sources
RUN mkdir -p /opt/llvm/build01 && cd /opt/llvm && \
    curl -L http://llvm.org/releases/3.7.1/llvm-3.7.1.src.tar.xz | tar -xJf -

WORKDIR /opt/llvm/build01
RUN rm -rf /opt/llvm/build01/* && mkdir -p Release+Asserts/bin && \
    cp /usr/i686-w64-mingw32/sys-root/mingw/bin/*.dll Release+Asserts/bin && \
    cp /opt/gcc/usr/i686-w64-mingw32/lib/*.dll Release+Asserts/bin
RUN /opt/llvm/llvm-3.7.1.src/configure --host=i686-w64-mingw32 \
    --enable-optimized --enable-targets=host #> llvm-conf.log 2>&1 || exit 125
# ONLY_TOOLS=opt to avoid hitting an unrelated gtest macro expansion
# error that would cause build failures at some points in the history
RUN make -j`nproc` ONLY_TOOLS=opt #> llvm-build.log || exit 125
RUN wine /opt/llvm/build01/Release+Asserts/bin/opt.exe -slp-vectorizer \
    -S /opt/llvm/llvm-3.7.1.src/test/Transforms/SLPVectorizer/X86/vector.ll || exit 1
