FROM opensuse:42.2

# Install build-deps (source-install is way overkill here, oh well)
RUN zypper addrepo obs://windows:mingw:win32/openSUSE_42.2 mingw32
RUN zypper addrepo obs://Emulators/openSUSE_Leap_42.2 Emulators
# wine depends on samba which depends on krb5 which conflicts with krb5-mini
RUN zypper -n --gpg-auto-import-keys install git make m4 tar which patch curl wine -krb5-mini \
    mingw32-cross-gcc-c++ gcc-c++ mingw32-cross-gcc-fortran mingw32-libstdc++ cmake \
    mingw32-libgfortran mingw32-libssp

# Download sources
RUN git clone git://github.com/JuliaLang/julia \
    /src/julia

WORKDIR /src/julia
RUN mkdir -p usr/bin usr/tools && \
    cp /usr/i686-w64-mingw32/sys-root/mingw/bin/lib*.dll usr/bin && \
    cp /usr/i686-w64-mingw32/sys-root/mingw/bin/lib*.dll usr/tools
RUN echo 'XC_HOST = i686-w64-mingw32' >> Make.user && \
    echo 'override MARCH = pentium4' >> Make.user
RUN make -j `nproc` -C deps || make -C deps
#RUN make -j `nproc`
#ADD bisect-run.sh .
#RUN git bisect start && ./bisect-run.sh || git bisect new && \
#    git checkout release_38 && ./bisect-run.sh && git bisect old && \
#    git bisect run ./bisect-run.sh && git bisect log && \
#    git bisect log | curl -F 'sprunge=<-' http://sprunge.us
