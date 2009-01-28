#!/bin/bash

export HQC=/disk1/ubuntu
export HQCDIR=$HQC/kvhqc
export KVALOBS=$HQC/kvhqc
export KVDIR=$HQC/kvalobs
#export PATH=/usr/lib/qt4/bin:$PATH	
#export QTDIR=/usr/lib/qt4
#export QTLIB=/usr/lib
#export QTINC=/usr/include
#export BOOST_HOME=$HQC/apps
#export PUTOOLS=$HQC/apps

#export LD_LIBRARY_PATH=/metno/local/lib/mesa:${HQC}/omniORB4/lib:${KVDIR}/lib:${KVDIR}/lib/kvalobs
#export LD_LIBRARY_PATH=${HQC}/omniORB4/lib:${KVDIR}/lib:${KVDIR}/lib/kvalobs
#export LD_LIBRARY_PATH=${KVDIR}/lib:${KVDIR}/src/service-libs/kvcpp/.libs
export LD_LIBRARY_PATH=${KVDIR}/src/service-libs/kvcpp/.libs:${KVDIR}/src/lib/kvskel/.libs:${KVDIR}/src/lib/decodeutility/.libs:${KVDIR}/src/lib/kvalobs/.libs:${KVDIR}/src/lib/kvdb/.libs:${KVDIR}/src/lib/fileutil/.libs:${KVDIR}/src/lib/dnmithread/.libs:${KVDIR}/src/lib/milog/.libs:${KVDIR}/src/lib/corbahelper/.libs:${HQC}/local/lib
export DIANADIR=/usr/local
if [ -z $HQCDIR ]; then export HQCDIR=/metno/local/kvhqc; fi

if [ -z $KVALOBS ]; then export KVALOBS=/metno/local/src/kvalobs; fi
if [ -z $KVDIR   ]; then export KVDIR=${KVALOBS}; fi
export HIST=0

$DIANADIR/bin/diana.bin -style cleanlooks -s ${HQCDIR}/diana.setup& > ~/diana.err 2>/dev/null
$HQCDIR/bin/hqc4 -style cleanlooks &

