/*
HQC - Free Software for Manual Quality Control of Meteorological Observations

Copyright (C) 2013 met.no

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
#include "identifyUser.h"
#include "KvMetaDataBuffer.hh"
#include "MultiStationSelection.h"
#include "hqc_paths.hh"

#include <kvcpp/corba/CorbaKvApp.h>
#include <decodeutility/kvDataFormatter.h>

#include <QtGui/qapplication.h>
#include <QtGui/qmessagebox.h>
#include <QtCore/QDebug>

using namespace std;

namespace Weather {
  kvalobs::DataReinserter<kvservice::KvApp> * reinserter;
}

using Weather::reinserter;

int main( int argc, char* argv[] )
{
  QApplication a( argc, argv, true );
  QStringList args = a.arguments();
  QString myconf = hqc::getPath(hqc::CONFDIR) + "/kvalobs.conf";
  for (int i = 1; i < args.size(); ++i) {
      if( args.at(i) == "--config" ) {
          if( i+1 >= args.size() ) {
              qDebug() << "invalid --config without filename";
              exit(1);
          }
          i += 1;
          myconf = args.at(i);
          qDebug() << "--config '" << myconf << "'";
      }
  }

  miutil::conf::ConfSection *confSec = kvservice::corba::CorbaKvApp::readConf(myconf.toStdString());
  if(!confSec) {
    std::clog << "Can't open configuration file: " << myconf.toStdString() << std::endl;
    return 1;
  }
  kvservice::corba::CorbaKvApp kvapp(argc, argv, confSec);


  QString userName;
  reinserter = Authentication::identifyUser(0, kvservice::corba::CorbaKvApp::kvApp,
                                            "ldap-oslo.met.no", userName);
  if ( reinserter == 0 ) {
    int res = QMessageBox::warning( 0, "Autentisering",
				    "Du er ikke registrert som operat�r!\n"
				   "Du kan se p� data, men ikke gj�re endringer i Kvalobsdatabasen!",
				    "Fortsett",//QMessageBox::Ok | QMessageBox::Default,
				    "Avslutt",//QMessageBox::Cancel | QMessageBox::Escape,
				    ""//QMessageBox::NoButton
				    );
    if ( res == QMessageBox::Cancel )
      return 1;
  }

  KvMetaDataBuffer kvmdb;
  Weather::MultiStationSelection d(0, 0/*& dl.front()*/ );
  d.show();

  a.connect( &a, SIGNAL(lastWindowClosed()), &a, SLOT(quit()) );
  return a.exec();
}
