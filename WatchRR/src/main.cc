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
#include <kvcpp/corba/CorbaKvApp.h>
#include <qapplication.h>
#include <decodeutility/kvDataFormatter.h>
#include "identifyUser.h"
#include <qmessagebox.h>
#include <puTools/miString.h>

#include "MultiStationSelection.h"

using namespace std;

typedef kvservice::corba::CorbaKvApp KvApp;
typedef std::list<kvalobs::kvData> kvDataList;

namespace WatchRR {
  kvalobs::DataReinserter<kvservice::KvApp> * reinserter;
}

using WatchRR::reinserter;

int main( int argc, char ** argv )
{
  const char * kvdir = getenv( "KVALOBS" );
  const char * hist = getenv( "HIST" );
    std::string shist = hist ? std::string(hist) : "0";
  std::string myconf;
  if ( shist == "1" )
    myconf = std::string( kvdir ) + "/etc/kvhqc/kvhist.conf";
  else
    myconf = std::string( kvdir ) + "/etc/kvhqc/kvalobs.conf";

  miutil::conf::ConfSection *confSec = KvApp::readConf(myconf);
  if(!confSec) {
    std::clog << "Can't open configuration file: " << myconf << std::endl;
    return 1;
  }
  KvApp kvapp(argc, argv, confSec);

  QApplication a( argc, argv, true );


  QString userName;
  reinserter = Authentication::identifyUser( KvApp::kvApp,
  					     "ldap-oslo.met.no", userName);
  if ( reinserter == 0 ) {
    int res = QMessageBox::warning( 0, "Autentisering",
				    "Du er ikke registrert som operatør!\n"
				   "Du kan se på data, men ikke gjøre endringer i Kvalobsdatabasen!",
				    QMessageBox::Ok | QMessageBox::Default,
				    QMessageBox::Cancel | QMessageBox::Escape,
				    QMessageBox::NoButton );
    if ( res == QMessageBox::Cancel )
      return 1;
  }


  std::list<kvalobs::kvStation> slist;
  if (!KvApp::kvApp->getKvStations(slist)) {
    std::clog << "Can't connect to station table!" << std::endl;
  }

  WatchRR::MultiStationSelection d(QString::fromStdString( kvapp.kvpathInCorbaNameserver()), slist, 0, 0/*& dl.front()*/ );
  d.show();
  a.connect( &a, SIGNAL(lastWindowClosed()), &a, SLOT(quit()) );
  return a.exec();
}
