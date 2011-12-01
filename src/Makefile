# Slave Makefile for HQC

COBJ=

CCOBJ= 	hqc4.o 


QOBJ= 	hqcmain.o ListDialog.o ClockDialog.o dianashowdialog.o TimeseriesDialog.o rejecttimeseriesdialog.o qtQTUtil.o parameterdialog.o identifyUser.o errorlist.o helptree.o BusyIndicator.o HqcDataReinserter.o ExtendedFunctionalityHandler.o missingtable.o  GetData.o textdatadialog.o textdatatable.o rejectdialog.o rejecttable.o GetTextData.o discarddialog.o connect2stinfosys.o \
	KvalobsData.o KvalobsDataModel.o KvalobsDataView.o KvalobsDataDelegate.o

# Removed:  datatable.o tabwindow.o

#----------------------------------------------------------------------------

CSRC=$(COBJ:.o=.c)
CCSRC= $(CCOBJ:.o=.cc)
QSRC=  $(QOBJ:.o=.cc)


MOCOBJ=$(QOBJ:.o=_moc.o)

ALLSRC= $(CSRC) $(CCSRC) $(QSRC)
ALLOBJ= $(COBJ) $(CCOBJ) $(QOBJ) $(MOCOBJ)
INTERNALLIBS= $(AUTHDIR)/libauthentication.a $(FAILDIR)/libFailInfo.a $(WATCHDIR)/libWatchRR.a $(WEATHERDIR)/libWeather.a

#----------------------------------------------------------------------------

.SUFFIXES: .cc .o 

# From .cc to .o
.cc.o:
	@echo $(INCLUDE)
	$(CXX) -ggdb $(CCFLAGS) -c $< $(INCLUDE)

# From .c to .o
.c.o:
	$(CC) -c $(CCFLAGS) $< $(INCLUDE)

all: $(BINDIR)/hqc.bin


#----------------------------------------------------------------------------

mocs:	
	echo "#automatic generated moc targets " > $(MOCFILE)
	for ENTRY in $(QOBJ:.o=.h) ; \
	do \
	echo                                                       >> $(MOCFILE) ; \
	echo $${ENTRY%.h}_moc.cc: $(INCDIR)/$${ENTRY}              >> $(MOCFILE) ; \
	echo "	"$(MOC) $(INCDIR)/$${ENTRY} -o $${ENTRY%.h}_moc.cc >> $(MOCFILE) ; \
	done

include $(MOCFILE)

#----------------------------------------------------------------------------

depends:
	rm -f $(DEPENDSFILE)
	echo '# Automatically-generated dependencies list:' > $(DEPENDSFILE)
	$(CXX) $(CCFLAGS) $(INCLUDE) -M $(ALLSRC) >> $(DEPENDSFILE)


include $(DEPENDSFILE)

#----------------------------------------------------------------------------

$(BINDIR)/hqc.bin: $(ALLOBJ) $(INTERNALLIBS)
	@echo $(INTERNALLIBS)
	$(CXX) -o $@ $(CCFLAGS) $(ALLOBJ)  $(LINKS)
