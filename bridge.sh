#!/bin/sh

if [ -e "bridge.log" ]; then
    rm bridge.log
fi
./bridge start
tail -1cf bridge.log
