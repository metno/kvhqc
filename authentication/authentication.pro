######################################################################
# Automatically generated by qmake (1.05a) Tue Dec 7 16:31:12 2004
######################################################################

CONFIG += thread
CONFIG += qt warn_on release
CONFIG	+= create_prl
CONFIG += link_prl
QT += qt3support

# Input
HEADERS += Authenticator.h
SOURCES += Authenticator.cc 
#SOURCES += main.cc

#The following line was changed from FORMS to FORMS3 by qt3to4
FORMS3	= authenticationdialog.ui
TEMPLATE	=lib
CONFIG +=  staticlib
INCLUDEPATH	+= .
LDAP_DEPRECATED=1
LIBS	+= -lldap
LANGUAGE	= C++
#The following line was inserted by qt3to4
QT +=  
#The following line was inserted by qt3to4
CONFIG += uic3

