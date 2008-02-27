/*
HQC - Free Software for Manual Quality Control of Meteorological Observations

$Id$

Copyright (C) 2007 met.no

Contact information:
Norwegian Meteorological Institute
Box 43 Blindern
0313 OSLO
NORWAY
email: kvalobs-dev@met.no

This file is part of HQC

HQC is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

HQC is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License along
with HQC; if not, write to the Free Software Foundation Inc.,
51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/
/****************************************************************************
**
*****************************************************************************/

//#include <KvApp.h>
#include <corba/CorbaKvApp.h>
#include <qapplication.h>
#include "hqcmain.h"
#include <iostream>

using namespace std;

using kvservice::corba::CorbaKvApp;


int main( int argc, char ** argv ) {
  const char * kvdir = getenv( "KVALOBS" );
  const char * hist = getenv( "HIST" );
    string shist = hist ? string(hist) : "0";
  string myconf;
  if ( shist == "1" )
    myconf = string( kvdir ) + "/etc/kvhist.conf";
  else if ( shist == "2" )
    myconf = string( kvdir ) + "/etc/kvtest.conf";
  else 
    myconf = string( kvdir ) + "/etc/kvalobs.conf";
  
  
  //  miutil::conf::ConfSection *confSec = kvservice::KvApp::readConf(myconf);
  miutil::conf::ConfSection *confSec = CorbaKvApp::readConf(myconf);
  if(!confSec) {
    clog << "Can't open configuration file: " << myconf << endl;
    return 1;
  }

  //  kvservice::KvApp kvapp(argc, argv, confSec);
  CorbaKvApp kvapp(argc, argv, confSec);
  
  QApplication a( argc, argv, true );
  
  HqcMainWindow * mw;
  
  try {
    mw = new HqcMainWindow();
  }
  catch ( std::exception &e ) {
    std::cerr << "Error: " << e.what() << std::endl;
    std::cerr << "Exiting application" << std::endl;
    return 1;
  }
  
  cerr << "C\n";

  QString captionSuffix = kvapp.kvpathInCorbaNameserver();
  QString caption = "HQC " + captionSuffix;  
  mw->setCaption( caption );
  mw->showMaximized();
  //  mw->setGeometry(10,10,1268,942);
  a.setMainWidget(mw);
  
  mw->show();
  a.connect( &a, SIGNAL(lastWindowClosed()), &a, SLOT(quit()) );

  int res = a.exec();
  return res;
}
