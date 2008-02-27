#!/bin/bash

#export HQCDIR=/metno/local/kvhqc/
export KVALOBS=$HQCDIR
export LD_LIBRARY_PATH=/metno/local/lib/mesa::${HOME}/omniORB4/lib
export HIST=0
$HQCDIR/bin/WatchRR 
