#!/bin/bash

#export HQCDIR=/metno/local/kvhqc
if [ -z $HQCDIR ]; then export HQCDIR=$HOME/kvhqc; fi

# FIXME: Vi kan ikke være avhengige av ting fra en kildekode-katalog!
# FIXME: Vi skal bruke KVALOBS-variabelen, ikke KVDIR. Rett i .cc-koden
#export KVALOBS=/metno/local/src/kvalobs
if [ -z $KVALOBS ]; then export KVALOBS=/metno/local/src/kvalobs; fi
if [ -z $KVDIR   ]; then export KVDIR=${KVALOBS}; fi

#$HQCDIR/bin/hqc &> ~/hqc.err
$HQCDIR/bin/hqc
