#--------------------------------
# Toplevel Makefile for HQC
#--------------------------------

# includefile contains LOCALDIR-definition etc.
#include ../make.${OSTYPE}
# Hvis kildekoden ikke ligger i /metno/local/src
#include /metno/local/libsrc/make.${OSTYPE}
include ../make.${OSTYPE}
#include ../conf/${OSTYPE}.mk

# Definerer et par ekstra variable som ikke står i ../make.${OSTYPE}

ifndef HQCDIR
HQCDIR=$(HOME)/kvhqc
endif
ifndef KVDIR
KVDIR=$(HOME)/kvalobs/kvalobs-drift-1-0
endif

AUTHDIR= $(HQCDIR)/authentication
FAILDIR= $(HQCDIR)/FailInfo
WATCHDIR= $(HQCDIR)/WatchRR
WEATHERDIR= $(HQCDIR)/Weather

ifndef OMNIORB_HOME
OMNIORB_HOME=/usr/local
endif
OMNI_INCLUDE= $(OMNIORB_HOME)/include -I$(OMNIORB_HOME)/include/omniORB4 
OMNI_LIB= $(OMNIORB_HOME)/lib 

ifndef BOOST_HOME
BOOST_HOME=/usr/local/
endif
BOOST_INCLUDE= $(BOOST_HOME)/include/
BOOST_LIB= $(BOOST_HOME)/lib

SRCDIR=src
LIBDIR=lib$(PLT)
OBJDIR=obj$(PLT)
BINDIR=bin$(PLT)
INCDIR=../include
LOCALINC=$(LOCALDIR)
BITSDIR= $(LOCALINC)/bits

DEPENDSFILE=$(OBJDIR)/make.depends
MOCFILE=$(OBJDIR)/make.moc
DEFINES=-DQT_GENUINE_STR -DWITH_STD_BOOL -D_STANDARD_C_PLUS_PLUS

INCLUDE=-I$(INCDIR) \
	-I$(AUTHDIR) \
	-I$(FAILDIR)/src \
	-I$(WATCHDIR)/src \
	-I$(WEATHERDIR)/src \
	-I$(BOOST_INCLUDE) \
	-I$(OMNI_INCLUDE) \
	-I$(QTDIR)/include \
	-I$(KVDIR)/include \
	-I$(KVDIR)/include/kvalobs \
	-I$(KVDIR)/include/miutil \
	-I$(KVDIR)/include/dnmithread \
	-I$(KVDIR)/include/kvdb \
	-I$(KVDIR)/include/kvskel   \
	-I$(KVDIR)/include/corbahelper \
	-I$(KVDIR)/include/kvservice/kvcpp \
	-I$(HOME)/apps/include \
	-I$(KVDIR)/include/fileutil \
	-I$(LOCALINC)/puTools/include \
	-I$(LOCALINC)/qUtilities/include \
	-I$(LOCALINC)/pets2/include \
	-I$(LOCALINC)/tsData/include \
	-I$(LOCALINC)/parameter/include \
	-I$(LOCALINC)/puMet/include \
	-I$(LOCALINC)/puDatatypes/include \
	-I$(LOCALINC)/glp/include \
	-I$(LOCALINC)/glText/include \
	-I$(LOCALINC)/tsData/include \
	-I$(LOCALINC)/qTimeseries/include \
	-I$(LOCALINC)/pets.2.0/include

LINKS:= -L$(WATCHDIR) -lWatchRR -L$(WEATHERDIR) -lWeather -L$(FAILDIR) -lFailInfo -L$(AUTHDIR) -lauthentication -lldap -L$(KVDIR)/$(LIBDIR) -lkvcpp -ldecodeutility -lkvalobs -lkvdb -lfileutil -ldnmithread -L$(HOME)/apps/lib -lmiutil -lpuTools -lmilog -L$(BOOST_LIB) -lboost_thread -L$(LOCALDIR)/$(LIBDIR) -lqTimeseries -lqUtilities -lpets2 -ltsData -lpuMet -lglText -lGLP -lmiconfparser -lpuDatatypes -lparameter -lpuCtools -lpets2 -L$(KVDIR)/src/service-libs/qt -lcorbahelper -lcorba_skel -L$(OMNI_LIB) -lomniORB4 -lomnithread -L$(QTDIR)/lib $(QT_LIBS) $(XLIBDIR) -lXmu -lXext -lXt -lXrender -lSM -lICE -lX11 -lXxf86vm -lm `pkg-config --libs libxml++-1.0`


OPTIONS="CXX=${CXX}" "CCFLAGS=${CXXFLAGS} ${DEFINES}" "CC=${CC}" "CFLAGS=${CFLAGS} ${DEFINES}" "LDFLAGS=${CXXLDFLAGS}" "AR=${AR}" "ARFLAGS=${ARFLAGS}" "INCLUDE=${INCLUDE}" "LIBDIR=${LIBDIR}" "DEPENDSFILE=../${DEPENDSFILE}" "BINDIR=../${BINDIR}" "LOCALDIR=${LOCALDIR}" "INCDIR=${INCDIR}" "LINKS=${LINKS}" "MOC=${MOC}" "MOCFILE=../${MOCFILE}" "AUTHDIR=$(AUTHDIR)" "FAILDIR=$(FAILDIR)" "WATCHDIR=$(WATCHDIR)" "WEATHERDIR=$(WEATHERDIR)"


.PHONY:	all auth fail mocs depends directories hqc \
	install pretty clean veryclean mark bin/WatchRR bin/Weather

all: mark auth fail watch weather directories mocs mark hqc bin/WatchRR bin/Weather

bin/WatchRR:
	cd $(WATCHDIR); $(MAKE) WatchRR
	cp $(WATCHDIR)/WatchRR bin/

bin/Weather:
	cd $(WEATHERDIR); $(MAKE) Weather
	cp $(WEATHERDIR)/Weather bin/

auth:
	cd $(AUTHDIR); qmake; $(MAKE)

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
	install -d $(DESTDIR)$(LOCALDIR)/kvhqc
	install -d $(DESTDIR)$(LOCALDIR)/kvhqc/bin
	install hqc_stations $(DESTDIR)$(LOCALDIR)/kvhqc
	install paramorder $(DESTDIR)$(LOCALDIR)/kvhqc
	install hqc_stations $(DESTDIR)$(LOCALDIR)/kvhqc
	install	diana.setup $(DESTDIR)$(LOCALDIR)/kvhqc
	install bin/hqc $(DESTDIR)$(LOCALDIR)/kvhqc/bin
	install bin/hqcdiana.sh $(DESTDIR)$(LOCALDIR)/kvhqc/bin
	install	hqc.png $(DESTDIR)$(LOCALDIR)/kvhqc

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



