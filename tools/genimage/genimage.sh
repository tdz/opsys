#!/bin/sh

imgfile=opsyshdd.img
basedir=$PWD
srcdir=$PWD

while [ $# -gt 0 ]
do
        case "$1" in
                -b) basedir="$2"
                        ;;
                -i) imagefile="$2"
                        ;;
                -s) srcdir="$2"
                        ;;
        esac
        shift
done

imgpath=`mktemp -td genimage.XXXXXXXXXX` || exit 1

mount -oloop,offset=32256 $imgfile $imgpath || exit 1

cp $basedir/menu.lst $imgpath/boot/grub/
cp $srcdir/kernel/oskernel $imgpath/
cp $srcdir/apps/helloworld/helloworld $imgpath/

umount $imgpath && rm -fr $imgpath

