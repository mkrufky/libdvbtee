#!/bin/sh

cd "$(dirname "$0")"
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
    git clone https://github.com/mkrufky/libdvbpsi.git
    cd libdvbpsi
    touch .dont_del
fi

./bootstrap
./configure --prefix=${DVBTEE_ROOT}/usr/ --enable-static --with-pic CPPFLAGS="-D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE"

cd ..

make -C libdvbpsi -j
if [ $? != 0 ]; then
    echo "make (libdvbpsi) failed"
    exit 1
fi

make -C libdvbpsi install

mkdir -p m4
if [ -e aclocal.m4 ]; then
    echo "configuration installed..."
else
    autoreconf --install
fi

if [ -e .staticlib ]; then
    LD_LIBRARY_PATH=${DVBTEE_ROOT}/usr/lib ./configure CPPFLAGS="-I${DVBTEE_ROOT}/usr/include/dvbpsi/ -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE" LDFLAGS="-L${DVBTEE_ROOT}/usr/lib/ -ldvbpsi" --enable-dvbpsibuild "$@" --with-pic --enable-static --disable-shared
else
    LD_LIBRARY_PATH=${DVBTEE_ROOT}/usr/lib ./configure CPPFLAGS="-I${DVBTEE_ROOT}/usr/include/dvbpsi/ -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE" LDFLAGS="-L${DVBTEE_ROOT}/usr/lib/ -ldvbpsi" --enable-dvbpsibuild "$@" --with-pic
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
