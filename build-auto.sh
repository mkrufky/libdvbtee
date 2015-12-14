#!/bin/sh

export DVBTEE_ROOT="`pwd`"

mkdir -p usr/bin
mkdir -p usr/lib
mkdir -p usr/include

if [ -e .clean ]; then
    make clean -C libdvbpsi
    if [ $? != 0 ]; then
        echo "make clean (libdvbpsi) failed"
        #exit 1 // dont exit
    fi

    make clean
    if [ $? != 0 ]; then
        echo "make clean failed"
        #exit 1 // dont exit
    fi
fi

if [ -e libdvbpsi/bootstrap ]; then
    cd libdvbpsi
else
    rm -rf libdvbpsi
    git clone git://github.com/mkrufky/libdvbpsi.git
    cd libdvbpsi
    touch .dont_del
fi

./bootstrap
./configure --prefix=${DVBTEE_ROOT}/usr/ --enable-static
cd ..

make -C libdvbpsi -j
if [ $? != 0 ]; then
    echo "make (libdvbpsi) failed"
    exit 1
fi

make -C libdvbpsi install

mkdir -p m4
autoreconf --install

if [ -e .staticlib ]; then
    ./configure CPPFLAGS=-I${DVBTEE_ROOT}/usr/include/dvbpsi/ LDFLAGS="-L${DVBTEE_ROOT}/usr/lib/ -ldvbpsi" --enable-dvbpsibuild "$@" --enable-static
else
    ./configure CPPFLAGS=-I${DVBTEE_ROOT}/usr/include/dvbpsi/ LDFLAGS="-L${DVBTEE_ROOT}/usr/lib/ -ldvbpsi" --enable-dvbpsibuild "$@"
fi

if [ -e .clean ]; then
    make clean
    if [ $? != 0 ]; then
        echo "make clean failed"
        #exit 1 // dont exit
    fi
fi

make -C . -j
if [ $? != 0 ]; then
    echo "make failed"
    exit 1
fi

make -C dvbtee
