FROM opensuse:42.2

# Install build-deps (source-install is way overkill here, oh well)
RUN zypper addrepo obs://windows:mingw:win32/openSUSE_42.2 mingw32
RUN zypper addrepo obs://Emulators/openSUSE_Leap_42.2 Emulators
# wine depends on samba which depends on krb5 which conflicts with krb5-mini
RUN zypper -n --gpg-auto-import-keys install git make flex tar curl wine -krb5-mini
RUN zypper -n install mingw32-cross-gcc-c++ gcc-c++ mingw32-libstdc++ cmake

# Download sources
RUN git clone git://github.com/llvm-mirror/llvm \
    -b release_39 /opt/llvm/src

RUN mkdir -p /opt/llvm/build
WORKDIR /opt/llvm/src
ADD bisect-run.sh .
RUN git bisect start && ./bisect-run.sh || git bisect new && \
    git checkout release_38 && ./bisect-run.sh && git bisect old && \
    git bisect run ./bisect-run.sh && git bisect log && \
    git bisect log | curl -F 'sprunge=<-' http://sprunge.us
