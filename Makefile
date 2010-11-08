#--------------------------------
# Toplevel Makefile for HQC
#--------------------------------

# includefile contains LOCALDIR-definition etc.
include etc/make.${OSTYPE}

# Definerer et par ekstra variable som ikke står i ../make.${OSTYPE}

DESTDIR:=/disk1/local
ifndef HQCDIR
HQCDIR:=$(PWD)
endif

QTINC= /usr/include/qt4
AUTHDIR= $(HQCDIR)/authentication
FAILDIR= $(HQCDIR)/FailInfo
WATCHDIR= $(HQCDIR)/WatchRR
WEATHERDIR= $(HQCDIR)/Weather

OMNIORB_HOME=/usr
OMNI_INCLUDE= $(OMNIORB_HOME)/include 
OMNI_LIB= $(OMNIORB_HOME)/lib 

BOOST_HOME=/usr
BOOST_INCLUDE= $(BOOST_HOME)/include
BOOST_LIB= $(BOOST_HOME)/lib

SRCDIR=src
LIBDIR=lib$(PLT)
OBJDIR=obj$(PLT)
BINDIR=bin$(PLT)
INCDIR=../include

LOCLIB=$(HQC)/../lib

BITSDIR= $(LOCALDIR)/bits

DEPENDSFILE=$(OBJDIR)/make.depends
MOCFILE=$(OBJDIR)/make.moc
DEFINES=-DQT_GENUINE_STR -DWITH_STD_BOOL -D_STANDARD_C_PLUS_PLUS

#METLIBS_PKG_CONFIG = qUtilities qTimeseries glText puTools tsData pets2 glp puMet puCtools puDatatypes
METLIBS_PKG_CONFIG = qUtilities qTimeseries glText puTools glp puMet puCtools puDatatypes ftgl

INCLUDE=-I$(INCDIR) \
	-I$(AUTHDIR) \
	-I$(FAILDIR)/src \
	-I$(WATCHDIR)/src \
	-I$(WEATHERDIR)/src \
	-I$(BOOST_INCLUDE) \
	-I$(OMNI_INCLUDE) \
	-I$(QTINC) \
	-I$(QTINC)/Qt3Support \
	-I$(QTINC)/Qt \
	-I$(QTINC)/QtCore \
	-I$(QTINC)/QtGui \
	-I$(QTINC)/QtOpenGL \
	-I$(QTINC)/QtNetwork \
	`pkg-config --cflags libkvcpp $(METLIBS_PKG_CONFIG)`

LINKS:= -L$(WATCHDIR) -lWatchRR \
	-L$(WEATHERDIR) -lWeather \
	-L$(FAILDIR) -lFailInfo \
	-L$(AUTHDIR) -lauthentication -lldap \
	-L$(BOOST_LIB) -lboost_thread \
	`pkg-config --libs libkvcpp $(METLIBS_PKG_CONFIG)` -lgfortran \
	$(QTLIBDIR) -lQt3Support -lQtCore -lQtGui -lQtOpenGL -lm \
	`pkg-config --libs libxml++-2.6`
###	`pkg-config --libs libkvcpp qutilities qtimeseries gltext putools tsdata pets2 glp pumet puctools parameter pudatatypes` \

OPTIONS="CXX=${CXX}" "CCFLAGS=${CXXFLAGS} ${DEFINES}" "CC=${CC}" "CFLAGS=${CFLAGS} ${DEFINES}" "LDFLAGS=${CXXLDFLAGS}" "AR=${AR}" "ARFLAGS=${ARFLAGS}" "INCLUDE=${INCLUDE}" "LIBDIR=${LIBDIR}" "DEPENDSFILE=../${DEPENDSFILE}" "BINDIR=../${BINDIR}" "LOCALDIR=${LOCALDIR}" "INCDIR=${INCDIR}" "LINKS=${LINKS}" "MOC=${MOC}" "MOCFILE=../${MOCFILE}" "AUTHDIR=$(AUTHDIR)" "FAILDIR=$(FAILDIR)" "WATCHDIR=$(WATCHDIR)" "WEATHERDIR=$(WEATHERDIR)"


.PHONY:	all auth fail mocs depends directories hqc.bin \
	install pretty clean veryclean mark bin/WatchRR bin/Weather

all: mark auth fail watch weather directories mocs mark hqc bin/WatchRR bin/Weather

bin/WatchRR:
	cd $(WATCHDIR); $(MAKE) WatchRR
	cp $(WATCHDIR)/WatchRR $(HQCDIR)/$(BINDIR)/WatchRR.bin

bin/Weather:
	cd $(WEATHERDIR); $(MAKE) Weather
	cp $(WEATHERDIR)/Weather $(HQCDIR)/$(BINDIR)/Weather.bin

auth:
	cd $(AUTHDIR); qmake-qt4; $(MAKE)

fail:
	cd $(FAILDIR); $(MAKE)

watch:
	cd $(WATCHDIR); $(MAKE)

weather:
	cd $(WEATHERDIR); $(MAKE)

mocs:
	cd $(OBJDIR); make $(OPTIONS) mocs

depends:
	if [ ! -f $(DEPENDSFILE) ] ; \
	then touch $(DEPENDSFILE) ; fi
	cd $(SRCDIR); make $(OPTIONS) depends

directories:
	if [ ! -d $(OBJDIR) ] ; then mkdir $(OBJDIR) ; fi
	if [ ! -d $(LIBDIR) ] ; then mkdir $(LIBDIR) ; fi
	if [ ! -d $(BINDIR) ] ; then mkdir $(BINDIR) ; fi
	touch $(MOCFILE)
	if [ ! -f $(DEPENDSFILE) ] ; \
	then touch $(DEPENDSFILE) ; make depends ; fi
	cd $(OBJDIR) ; ln -sf ../$(SRCDIR)/* .

hqc:	
	cd $(OBJDIR); make $(OPTIONS) all

install:
	install bin/hqc.bin $(DESTDIR)/bin
	install bin/WatchRR.bin $(DESTDIR)/bin
	install bin/Weather.bin $(DESTDIR)/bin
	install bin/hqcdiana $(DESTDIR)/bin
	install bin/WatchRR $(DESTDIR)/bin
	install bin/WatchWeather $(DESTDIR)/bin
	install	etc/kvhqc/diana.setup $(DESTDIR)/etc/kvhqc
	install	etc/kvhqc/hqc_stations $(DESTDIR)/etc/kvhqc
	install	etc/kvhqc/typeids $(DESTDIR)/etc/kvhqc
	install	etc/kvhqc/slimits $(DESTDIR)/etc/kvhqc
	install	etc/kvhqc/paramorder $(DESTDIR)/etc/kvhqc
	install	etc/kvhqc/kvalobs.conf $(DESTDIR)/etc/kvhqc
	install	etc/kvhqc/kvhist.conf $(DESTDIR)/etc/kvhqc
	install	etc/kvhqc/hqc.png $(DESTDIR)/etc/kvhqc

pretty:
	find . \( -name 'core' -o -name '*~' \) -exec rm -f {} \;

clean:	pretty
	@make pretty
	cd $(OBJDIR); rm -f *.o ; rm -f *_moc.cc 

veryclean: clean 
	cd $(AUTHDIR); $(MAKE) clean
	cd $(FAILDIR); $(MAKE) clean
	cd $(WATCHDIR); $(MAKE) clean
	cd $(WEATHERDIR); $(MAKE) clean
	rm -rf $(OBJDIR)
	rm -f $(DEPENDSFILE)

mark:	
	@echo "[1;34m = = = = = = = = = = = = = = = = = = = = = = [0m "



