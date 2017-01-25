FROM opensuse:42.2

# Install build-deps (source-install is way overkill here, oh well)
RUN zypper addrepo obs://windows:mingw:win32/openSUSE_42.2 mingw32
RUN zypper addrepo obs://Emulators/openSUSE_Leap_42.2 Emulators
# wine depends on samba which depends on krb5 which conflicts with krb5-mini
#RUN zypper -n --gpg-auto-import-keys install python samba -krb5-mini
RUN zypper -n --gpg-auto-import-keys install git make flex tar curl wine -krb5-mini
RUN zypper -n source-install -d mingw32-cross-gcc-c++

# Download sources
RUN mkdir -p /opt/gcc6/build /opt/gcc49/build /opt/llvm/build && cd /opt/llvm && \
    curl -L http://llvm.org/releases/3.7.1/llvm-3.7.1.src.tar.xz | tar -xJf -
RUN git clone git://github.com/gcc-mirror/gcc \
    -b gcc-6-branch /opt/gcc6/src
RUN cp -r /opt/gcc6/src /opt/gcc49/src && cd /opt/gcc49/src && git checkout gcc-4_9-branch

WORKDIR /opt/gcc49/build
RUN /opt/gcc49/src/configure --prefix=/opt/gcc49/usr \
    --target=i686-w64-mingw32 --enable-languages="c,c++" \
    --disable-multilib --enable-threads=posix \
    --with-sysroot=/usr/i686-w64-mingw32/sys-root \
    --with-as=/usr/bin/i686-w64-mingw32-as > gcc49-conf.log 2>&1
RUN make -j`nproc` > gcc49-build.log 2>&1
RUN make install > gcc49-inst.log 2>&1

WORKDIR /opt/gcc6/build
RUN /opt/gcc6/src/configure --prefix=/opt/gcc6/usr \
    --target=i686-w64-mingw32 --enable-languages="c,c++" \
    --disable-multilib --enable-threads=posix \
    --with-sysroot=/usr/i686-w64-mingw32/sys-root \
    --with-as=/usr/bin/i686-w64-mingw32-as > gcc6-conf.log 2>&1
RUN make -j`nproc` > gcc6-build.log 2>&1
RUN make install > gcc6-inst.log 2>&1

#export PATH=/opt/gcc/usr/bin:$PATH

#cd /opt/llvm/build
#rm -rf /opt/llvm/build/*
#mkdir -p Release+Asserts/bin
#cp /usr/i686-w64-mingw32/sys-root/mingw/bin/*.dll Release+Asserts/bin
#cp /opt/gcc/usr/i686-w64-mingw32/lib/*.dll Release+Asserts/bin
#/opt/llvm/llvm-3.7.1.src/configure --host=i686-w64-mingw32 \
#  --enable-optimized --enable-targets=host #> llvm-conf.log 2>&1 || exit 125
# ONLY_TOOLS=opt to avoid hitting an unrelated gtest macro expansion
# error that would cause build failures at some points in the history
#make -j`nproc` ONLY_TOOLS=opt #> llvm-build.log || exit 125
#wine /opt/llvm/build/Release+Asserts/bin/opt.exe -slp-vectorizer \
#  -S /opt/llvm/llvm-3.7.1.src/test/Transforms/SLPVectorizer/X86/vector.ll || exit 1
