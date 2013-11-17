#! /bin/bash

topdir=`dirname $0`/../
rm $topdir/* 2>/dev/null
mv $topdir/src/* $topdir
rm -rf $topdir/src

