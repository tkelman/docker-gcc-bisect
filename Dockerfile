FROM opensuse:42.2

# Install build-deps (source-install is way overkill here, oh well)
RUN zypper addrepo obs://windows:mingw:win32/openSUSE_42.2 mingw32
RUN zypper addrepo obs://Emulators/openSUSE_Leap_42.2 Emulators
# wine depends on samba which depends on krb5 which conflicts with krb5-mini
#RUN zypper -n --gpg-auto-import-keys install python samba -krb5-mini
RUN zypper -n --gpg-auto-import-keys install git make flex tar curl wine -krb5-mini
RUN zypper -n source-install -d mingw32-cross-gcc-c++

# Download sources
RUN mkdir -p /opt/gcc/build /opt/llvm/build && cd /opt/llvm && \
    curl -L http://llvm.org/releases/3.7.1/llvm-3.7.1.src.tar.xz | tar -xJf -
RUN git clone git://github.com/gcc-mirror/gcc \
    -b gcc-6-branch /opt/gcc/src
#    /opt/gcc/src

WORKDIR /opt/gcc/src
#RUN git checkout HEAD~3000
ADD bisect-run.sh .
RUN ./bisect-run.sh
#RUN git bisect start && ./bisect-run.sh || git bisect bad && \
#    git checkout gcc-4_9-branch && ./bisect-run.sh && git bisect good && \
#    git bisect run ./bisect-run.sh && git bisect log && \
#    git bisect log | curl -F 'sprunge=<-' http://sprunge.us
