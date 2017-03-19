FROM opensuse:42.2

# Install build-deps (source-install is way overkill here, oh well)
RUN zypper addrepo obs://windows:mingw:win32/openSUSE_42.2 mingw32
RUN zypper addrepo obs://Emulators/openSUSE_Leap_42.2 Emulators
# wine depends on samba which depends on krb5 which conflicts with krb5-mini
RUN zypper -n --gpg-auto-import-keys install git make flex tar curl wine -krb5-mini dejagnu autogen
RUN zypper -n source-install -d mingw32-cross-gcc-c++

# Download sources
RUN mkdir -p /opt/gcc6/build && \
    git clone git://github.com/gcc-mirror/gcc -b gcc-6-branch /opt/gccsrc && \
    cd /opt/gcc6/build && \
    /opt/gccsrc/configure --prefix=/opt/gcc6/usr \
    --target=i686-w64-mingw32 --enable-languages="c,c++,fortran,lto,objc,obj-c++" \
    --disable-multilib --enable-threads=posix \
    --with-sysroot=/usr/i686-w64-mingw32/sys-root \
    --with-as=/usr/bin/i686-w64-mingw32-as && \
    make -j`nproc` && make install && make check && cd .. && rm -rf build && \
    cd /opt/gccsrc && curl https://gist.githubusercontent.com/anonymous/4621fc4aba09827c7bde0e8acb9d2a88/raw/- | git apply -C2 && \
    rm -rf .git && \
    rm -rf /opt/gccsrc
