#!/bin/bash

export LD_LIBRARY_PATH=/metno/local/lib/mesa:${HOME}/omniORB4/lib
export DIANADIR=/metno/local/diana
if [ -z $HQCDIR ]; then export HQCDIR=/metno/local/kvhqc; fi

if [ -z $KVALOBS ]; then export KVALOBS=/metno/local/src/kvalobs; fi
if [ -z $KVDIR   ]; then export KVDIR=${KVALOBS}; fi
export HIST=0

$DIANADIR/bin/diana -s ${HQCDIR}/diana.setup& &> ~/diana.err
$HQCDIR/bin/hqc &

