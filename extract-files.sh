#!/bin/sh

VENDOR=pantech
DEVICE=a890

BASE=../../../vendor/$VENDOR/$DEVICE/proprietary
rm -rf $BASE/*

for FILE in `cat proprietary-blobs.txt | grep -v ^# | grep -v ^$ `; do
    DIR=`dirname $FILE`
    if [ ! -d $BASE/$DIR ]; then
        mkdir -p $BASE/$DIR
    fi
    cp ~/IM-A890L_S1237219/system/$FILE $BASE/$FILE
done

./setup-makefiles.sh
