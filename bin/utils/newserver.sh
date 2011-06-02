#!/bin/sh

if [ "x$1" = "x" ]
then
    echo "usage: $0 <new server directory>"
    exit 1
fi

DEST=$1


v_system=$(uname)
f_starter=$(basename $0)
d_pwd=$(pwd)
d_main=

if echo $v_system | grep -q "BSD"
then
    d_main=$(dirname $(dirname $(cd $(dirname $0) ; pwd)))
    cd $d_pwd
else
    d_main=$(dirname $(dirname $(dirname $(readlink -f $0))))
fi


mkdir -p $DEST

mkdir $DEST/bin
for aa in utils env.sh sauer_server
do
    ln -s $d_main/bin/$aa $DEST/bin/$aa
done
cp $d_main/bin/server $DEST/bin/

ln -s $d_main/lib $DEST/lib
ln -s $d_main/script $DEST/script
ln -s $d_main/share $DEST/share

mkdir $DEST/conf
mkdir $DEST/log
mkdir $DEST/log/game
mkdir $DEST/log/demo

cp $d_main/conf/server.conf $DEST/conf/server.conf
cp $d_main/conf/maps.conf $DEST/conf/maps.conf
