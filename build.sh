#!/usr/bin/env bash

INSTALL_DIR=/usr/local
SRC_DIR=jlog
BUILD_DIR=build

if [ "$1" == "uninstall" ]
then
    sudo rm -rf $INSTALL_DIR/bin/jlogd
    sudo rm -rf $INSTALL_DIR/include/jlog*
    sudo rm -rf $INSTALL_DIR/lib/*jlog*
    exit
fi
if [ ! -d $BUILD_DIR ]
then
    mkdir $BUILD_DIR
fi

cd $BUILD_DIR && cmake .. && make && cd -

if [ "$1" == "install" ]
then
    if [ ! -d $INSTALL_DIR/include/$SRC_DIR ]
    then
        sudo mkdir $INSTALL_DIR/include/$SRC_DIR
    fi
    sudo cp $SRC_DIR/*.h $INSTALL_DIR/include/$SRC_DIR
    sudo cp jlog.h $INSTALL_DIR/include
    sudo cp $BUILD_DIR/lib/libjlog.so $INSTALL_DIR/lib
    sudo cp $BUILD_DIR/bin/jlogd $INSTALL_DIR/bin
fi
