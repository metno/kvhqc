#--------------------------------
# Toplevel Makefile for HQC
#--------------------------------

# includefile contains LOCALDIR-definition etc.
OSTYPE=linux-gnu
include etc/make.${OSTYPE}

# Definerer et par ekstra variable som ikke står i ../make.${OSTYPE}

ifndef HQCDIR
HQCDIR:=$(PWD)
endif
ifndef KVDIR
KVDIR=$(HQC)/kvalobs
endif
QTINC= /usr/include/qt4
AUTHDIR= $(HQCDIR)/authentication
FAILDIR= $(HQCDIR)/FailInfo
WATCHDIR= $(HQCDIR)/WatchRR
WEATHERDIR= $(HQCDIR)/Weather

#ifndef OMNIORB_HOME
OMNIORB_HOME=/usr
#endif
#OMNI_INCLUDE= $(OMNIORB_HOME)/include -I$(OMNIORB_HOME)/include/omniORB4 
OMNI_INCLUDE= $(OMNIORB_HOME)/include 
OMNI_LIB= $(OMNIORB_HOME)/lib 

#ifndef BOOST_HOME
BOOST_HOME=/usr
#endif
BOOST_INCLUDE= $(BOOST_HOME)/include
BOOST_LIB= $(BOOST_HOME)/lib
#BOOST_LIB= ../../lib

SRCDIR=src
LIBDIR=lib$(PLT)
OBJDIR=obj$(PLT)
BINDIR=bin$(PLT)
INCDIR=../include
#LOCALINC=/disk1/QT4/local/include
LOCALINC=$(HOME)/local/include
LOCLIB=$(HQC)/../lib
#QUTILLIB=/disk1/metlibs/metlibs/lib
BITSDIR= $(LOCALDIR)/bits

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
	-I$(QTINC) \
	-I$(QTINC)/Qt3Support \
	-I$(QTINC)/Qt \
	-I$(QTINC)/QtCore \
	-I$(QTINC)/QtGui \
	-I$(QTINC)/QtOpenGL \
	-I$(QTINC)/QtNetwork \
	`pkg-config --cflags kvcpp qutilities qtimeseries gltext putools`

LINKS:= -L$(WATCHDIR) -lWatchRR4 \
	-L$(WEATHERDIR) -lWeather4 \
	-L$(FAILDIR) -lFailInfo \
	-L$(AUTHDIR) -lauthentication -lldap \
	-L$(BOOST_LIB) -lboost_thread \
	`pkg-config --libs kvcpp qutilities qtimeseries gltext putools tsdata pets2` \
	-L$(OMNI_LIB) -lomniORB4 -lomnithread \
	$(QTLIBDIR) -lQt3Support -lQtCore -lQtGui -lQtOpenGL $(QT_LIBS) $(XLIBDIR) -lXmu -lXext -lXt -lXrender -lSM -lICE -lX11 -lXxf86vm -lm `pkg-config --libs libxml++-2.6`

OPTIONS="CXX=${CXX}" "CCFLAGS=${CXXFLAGS} ${DEFINES}" "CC=${CC}" "CFLAGS=${CFLAGS} ${DEFINES}" "LDFLAGS=${CXXLDFLAGS}" "AR=${AR}" "ARFLAGS=${ARFLAGS}" "INCLUDE=${INCLUDE}" "LIBDIR=${LIBDIR}" "DEPENDSFILE=../${DEPENDSFILE}" "BINDIR=../${BINDIR}" "LOCALDIR=${LOCALDIR}" "INCDIR=${INCDIR}" "LINKS=${LINKS}" "MOC=${MOC}" "MOCFILE=../${MOCFILE}" "AUTHDIR=$(AUTHDIR)" "FAILDIR=$(FAILDIR)" "WATCHDIR=$(WATCHDIR)" "WEATHERDIR=$(WEATHERDIR)"


.PHONY:	all auth fail mocs depends directories hqc4 \
	install pretty clean veryclean mark bin/WatchRR4 bin/Weather4

all: mark auth fail watch weather directories mocs mark hqc4 bin/WatchRR4 bin/Weather4

bin/WatchRR4:
	cd $(WATCHDIR); $(MAKE) WatchRR4
	cp $(WATCHDIR)/WatchRR4 bin/

bin/Weather4:
	cd $(WEATHERDIR); $(MAKE) Weather4
	cp $(WEATHERDIR)/Weather4 bin/

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

hqc4:	
	cd $(OBJDIR); make $(OPTIONS) all

install:
	install -d $(DESTDIR)$(LOCALDIR)/kvhqc
	install -d $(DESTDIR)$(LOCALDIR)/kvhqc/bin
	install hqc_stations $(DESTDIR)$(LOCALDIR)/kvhqc
	install paramorder $(DESTDIR)$(LOCALDIR)/kvhqc
	install hqc_stations $(DESTDIR)$(LOCALDIR)/kvhqc
	install	diana.setup $(DESTDIR)$(LOCALDIR)/kvhqc
	install bin/hqc4 $(DESTDIR)$(LOCALDIR)/kvhqc/bin
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



