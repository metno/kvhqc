#!/bin/bash

#export HQCDIR=/metno/local/kvhqc/
export KVALOBS=$HQCDIR
export LD_LIBRARY_PATH=/metno/local/lib/mesa::${HOME}/omniORB4/lib:${KVDIR}/lib:${KVDIR}/lib/kvalobs
export KVDIR=/disk1/QT4/kvalobs/kvalobs-drift-1-0
export KVALOBS=/disk1/QT4/kvhqc
export HQCDIR=${KVALOBS}
export HIST=0
$HQCDIR/bin/WatchRR4
 
