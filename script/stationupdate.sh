#! /bin/bash
export HQCDIR=/home/knutj/kvhqc
export PGCLIENTENCODING=LATIN5
psql -h stinfosys -U pstinfosys -d stinfosys -p 5435 -o $HQCDIR/stinfo_stations_fylke -t -c "select distinct x.stationid, y.name from station x, municip y where (x.municipid/100=y.municipid or (x.municipid<100 and x.municipid=y.municipid) or (x.municipid = 2800 and y.municipid = 2800)) order by x.stationid" <<EOF
info12
EOF
psql -h stinfosys -U pstinfosys -d stinfosys -p 5435 -o $HQCDIR/stinfo_stations_kommune -t -c "select distinct x.stationid, y.name from station x, municip y where ((x.municipid=y.municipid or (x.municipid=2300 and x.municipid/100=y.municipid))) order by x.stationid" <<EOF
info12
EOF
cat $HQCDIR/stinfo_stations_fylke | sed -e 's/|//g' -e '/.*Ukjent/d' > $HQCDIR/stinfo_fylke
cat $HQCDIR/stinfo_stations_kommune | sed -e 's/|//g' -e '/.*Ukjent/d' > $HQCDIR/stinfo_kommune
rm $HQCDIR/stinfo_stations_fylke $HQCDIR/stinfo_stations_kommune
$HQCDIR/mergestat > $HQCDIR/stinfo_stations
diff $HQCDIR/stinfo_stations $HQCDIR/hqc_stations > $HQCDIR/stinfostationdiff
dfs=`cat $HQCDIR/stinfostationdiff | wc -l`
echo $dfs
if [ $dfs != '0' ] 
  then
    mv $HQCDIR/stinfo_stations $HQCDIR/hqc_stations
    scp $HQCDIR/hqc_stations knutj@eurus8:/usr/local/stow/kvhqc-1.6.9/etc/kvhqc/hqc_stations <<EOF
vg1L#16ua9An
EOF
    rm $HQCDIR/stinfostationdiff
    echo "Stinfo er oppdatert, sendt til eurus8" | mail -s "Stinfo_stations oppdatert"  knut.johansen@met.no
  else
    rm $HQCDIR/stinfo_stations  $HQCDIR/stinfostationdiff
    echo "Ingen endringer i stinfo siste døgn." | mail -s "Stinfo_stations uforandret"  knut.johansen@met.no
fi
rm $HQCDIR/stinfo_fylke $HQCDIR/stinfo_kommune



