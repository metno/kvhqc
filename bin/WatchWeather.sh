#!/bin/bash

export HQCDIR=$HQC/kvhqc
export KVALOBS=$HQCDIR
export KVDIR=$HQC/kvalobs
export LD_LIBRARY_PATH=/metno/local/lib/mesa::${HOME}/omniORB4/lib:${KVDIR}/lib:${KVDIR}/lib/kvalobs
export HIST=0
$HQCDIR/bin/Weather4
 
