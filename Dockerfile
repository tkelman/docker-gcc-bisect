FROM ubuntu:14.04

RUN dpkg --add-architecture i386 && apt-get update && \
    apt-get install -y git m4 bzip2 curl gcc-mingw-w64-i686 make g++ file wine

# Download sources
RUN git clone git://github.com/JuliaLang/julia /src/julia

WORKDIR /src/julia
RUN echo 'XC_HOST = i686-w64-mingw32' >> Make.user && \
    echo 'override MARCH = pentium4' >> Make.user && \
    make -C deps install-gmp && file usr/bin/libgmp.dll
