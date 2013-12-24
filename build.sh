#!/bin/sh

export DVBTEE_ROOT="`pwd`"

if [ -e .mipsel ]; then
    export ARCH="mipsel-linux"
    export CROSS_COMPILE="mipsel-linux-"
    export CC="mipsel-linux-gcc"
    export GPP="mipsel-linux-g++"
    export LD="mipsel-linux-ld"
    export STRIP="mipsel-linux-strip"
else
    echo using default build environment...
fi

if [ -e Makefile ]; then
    echo Makefile already exists...
else
    qmake -r
fi

mkdir -p usr/bin
mkdir -p usr/lib
mkdir -p usr/include

if [ -e .clean ]; then
    make clean -C libdvbpsi
    if [ $? != 0 ]; then
        echo "make clean (libdvbpsi) failed"
    	#exit 1 // dont exit
    fi

    make clean -C dvbtee
    if [ $? != 0 ]; then
        echo "make clean (dvbtee) failed"
    	exit 1
    fi
fi

if [ -e libdvbpsi ]; then
    cd libdvbpsi
else
    git clone git://github.com/mkrufky/libdvbpsi.git
    cd libdvbpsi
    git checkout new_descriptors
    patch -p2 < ../libdvbpsi-silence-TS-discontinuity-messages.patch
fi

if [ -e .configured ]; then
    git log -1
else
    ./bootstrap
    if [ -e ../.x86 ]; then
	./configure --prefix=${DVBTEE_ROOT}/usr/  --disable-debug --disable-release
    else
	./configure --prefix=${DVBTEE_ROOT}/usr/ --host=mipsel-linux --target=mipsel-linux
    fi
    touch ./.configured
fi
cd ..


make -C libdvbpsi
if [ $? != 0 ]; then
    echo "make (libdvbpsi) failed"
    exit 1
fi

make -C libdvbpsi install

make -C . -I${DVBTEE_ROOT}/usr/include/dvbpsi/
if [ $? != 0 ]; then
    echo "make (dvbtee) failed"
    exit 1
fi

make -C dvbtee install
