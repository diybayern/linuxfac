#!/bin/bash
#

BUILD_PROCESSES=8
TOPDIR=$PWD
DEBTMPDIR=deb_tmp
WORKDIR=$TOPDIR/$DEBTMPDIR/usr/local/bin/factory/
pkgName=rainsys
VERSION=1.0.3

rm -f *.deb

cd factory_test

make clean
qmake
make -j$BUILD_PROCESSES
if [ $? -ne 0 ]; then
    echo
    echo "Error occured while compiling. Stop!"
    echo
    exit 1
fi

cd $TOPDIR
mkdir -p $WORKDIR
cp -rf DEBIAN $DEBTMPDIR/
chmod +x $DEBTMPDIR/DEBIAN/*

sed -i "s/packname/$pkgName/g" $DEBTMPDIR/DEBIAN/control
sed -i "s/currentversion/$VERSION/g" $DEBTMPDIR/DEBIAN/control

cp -rf factory_test/res $WORKDIR
cp -f factory_test/scripts/* $WORKDIR
cp -f factory_test/factory_test $WORKDIR

dpkg -b $DEBTMPDIR/ $pkgName.deb

rm -rf $DEBTMPDIR/

if [ ! -f $pkgName.deb ]; then
    echo "$pkgName compiling failed!" 1>&2
    exit 1
fi

echo "$pkgName packaging OK! find output: ${pkgName}.deb"
