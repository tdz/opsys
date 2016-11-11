#!/bin/sh

#
#  opsys - A small, experimental operating system
#  Copyright (C) 2009-2010  Thomas Zimmermann
#  Copyright (C) 2016       Thomas Zimmermann
#
#  This program is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

imgfile=opsyshdd.img
basedir=$PWD
srcdir=$PWD
arch=`uname -m`

while [ $# -gt 0 ]; do
        case "$1" in
                -a) arch="$2"
                        shift
                        ;;
                -b) basedir="$2"
                        shift
                        ;;
                -i) imgfile="$2"
                        shift
                        ;;
                -s) srcdir="$2"
                        shift
                        ;;
                 *) break
                        ;;
        esac
        shift
done

if [[ -z "$1" ]]; then
        echo "genimage: no files specified"
fi

imgpath=`mktemp -td genimage.XXXXXXXXXX` || exit 1

mount -oloop,offset=32256 $imgfile $imgpath || exit 1

cp $basedir/menu.lst $imgpath/boot/grub/

while [ $# -gt 0 ]; do
        cp "$1" $imgpath/
        shift
done

umount $imgpath && rm -fr $imgpath
