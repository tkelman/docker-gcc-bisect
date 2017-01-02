FROM opensuse:42.1

# Install build-deps (source-install is way overkill here, oh well)
RUN zypper addrepo obs://windows:mingw:win32/openSUSE_42.1 mingw32
RUN zypper addrepo obs://Emulators/openSUSE_Leap_42.1 Emulators
RUN zypper -n --gpg-auto-import-keys source-install -d mingw32-cross-gcc-c++
# wine depends on samba which depends on krb5 which conflicts with krb5-mini
RUN zypper -n install git make flex tar curl wine -krb5-mini

# Download sources
RUN mkdir -p /opt/gcc/build /opt/llvm/build && cd /opt/llvm && \
    curl -L http://llvm.org/releases/3.7.1/llvm-3.7.1.src.tar.xz | tar -xJf -
RUN git clone git://github.com/gcc-mirror/gcc \
    -b gcc-5-branch /opt/gcc/src

WORKDIR /opt/gcc/src
ADD bisect-run.sh .
#RUN git bisect start && ./bisect-run.sh #|| git bisect bad && \
#    git checkout gcc-4_9-branch && ./bisect-run.sh && git bisect good && \
#    git bisect run ./bisect-run.sh && git bisect log && \
#    git bisect log | curl -F 'sprunge=<-' http://sprunge.us
