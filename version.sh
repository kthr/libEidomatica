#!/bin/bash
GIT=`which git`
LOCALVER=`$GIT rev-list HEAD | wc -l`
if [ $LOCALVER \> 1 ] ; then
    VER=`$GIT rev-list origin/master | wc -l`
    VER_DIFF=$(($LOCALVER-$VER))
    if [ $VER_DIFF != 0 ] ; then
        VER="$VER+$VER_DIFF"
    fi
    if $GIT status | grep -q "modified:" ; then
        VER="${VER}M"
    fi
	VER=`echo ${VER} | sed 's/\ //'`
	echo "Revision=${VER}"
    echo "Creating revision.hpp."
    OUT="#ifndef REVISION_HPP_\n #define REVISION_HPP_ \n\n #define REVISION \"${VER}\" \n\n #endif" 
    echo -e $OUT > ./src/revision.hpp
fi
