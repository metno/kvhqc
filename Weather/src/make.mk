MAIN= src/main.cc
SRC+=  src/cellvalueprovider.cc \
	src/timeobs.cc \
	src/dataconsistencyverifier.cc \
	src/fdchecktableitem.cc \
	src/tnchecktableitem.cc \
	src/weathertableitem.cc \
	src/flagitem.cc \
	src/weathertabletooltip.cc \
	../src/BusyIndicator.cc \
	../src/identifyUser.cc \
	../src/HqcDataReinserter.cc

QSRC+=  src/MultiStationSelection.cc \
	src/weatherdialog.cc \
	src/weathertable.cc \
	src/centralwidget.cc \
	src/StationSelection.cc

#	src/RRCheckTableItem.cc \
#SRC+= src/cellvalueprovider.cc \
#	src/FDCheckTableItem.cc \
#	src/RR_24DataTableItem.cc \
#	src/RRTableItem.cc\
#	src/ControlFlagCell.cc \
#	src/RRTableToolTip.cc \
#	src/VxKvDataTableItem.cc \
#	src/dataconsistencyverifier.cc \
#	src/SADataTableItem.cc \
#	src/DayObs.cc \
#	src/OkCheckTableItem.cc \
#	src/SDDataTableItem.cc \
