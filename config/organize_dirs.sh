#! /bin/bash

topdir=`dirname $0`/../
mkdir -p $topdir/src
for f in `ls $topdir`; do
	if [ "$f" != "config" ] && [ "$f" != "src" ]; then
		mv $topdir/$f $topdir/src/$f 
	fi
done

cp -rf $topdir/config/* $topdir
rm -f $topdir/organize_dirs.sh $topdir/organize_dirs_inv.sh

