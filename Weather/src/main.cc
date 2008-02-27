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
#include <corba/CorbaKvApp.h>
#include <qapplication.h>
#include <decodeutility/kvDataFormatter.h>
#include "identifyUser.h"
#include <qmessagebox.h>
#include <puTools/miString.h>

#include "MultiStationSelection.h"

typedef kvservice::corba::CorbaKvApp KvApp;
typedef std::list<kvalobs::kvData> kvDataList;


namespace Weather {
  kvalobs::DataReinserter<kvservice::KvApp> * reinserter;
}

using Weather::reinserter;

int main( int argc, char ** argv ) 
{
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

  miutil::conf::ConfSection *confSec = KvApp::readConf(myconf);
  if(!confSec) {
    clog << "Can't open configuration file: " << myconf << endl;
    return 1;
  }
  KvApp kvapp(argc, argv, confSec);

  QApplication a( argc, argv, true );
  //  KvApp app( argc, argv, true );


//  kvDataList dl = 
//    decodeutility::kvdataformatter::getKvData( station );

  QString userName;
  reinserter = Authentication::identifyUser( KvApp::kvApp,
  					     "ldap.oslo.dnmi.no", userName);
  if ( reinserter == 0 ) {
    int res = QMessageBox::warning( 0, "Autentisering", 
				    "Du er ikke registrert som operatør!\n"
				   "Du kan se på data, men ikke gjøre endringer i Kvalobsdatabasen!",
				    "Fortsett",//				    QMessageBox::Ok | QMessageBox::Default,
				    "Avslutt",//				    QMessageBox::Cancel | QMessageBox::Escape,
				    ""//QMessageBox::NoButton 
				    );
    if ( res == QMessageBox::Cancel )
      return 1;
  }

  Weather::MultiStationSelection d( 0, 0/*& dl.front()*/ );
  d.show();

  a.connect( &a, SIGNAL(lastWindowClosed()), &a, SLOT(quit()) );
  return a.exec();
}
