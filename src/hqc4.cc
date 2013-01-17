/*
HQC - Free Software for Manual Quality Control of Meteorological Observations

$Id: hqc.cc 388 2008-04-28 10:18:55Z knutj $

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

#include "hqcmain.h"
#include "hqc_paths.hh"

#include <kvcpp/corba/CorbaKvApp.h>

#include <qapplication.h>
#include <qdebug.h>

#include <iostream>

using namespace std;

using kvservice::corba::CorbaKvApp;


int main( int argc, char* argv[] )
{
  QApplication a( argc, argv, true );

  QStringList args = a.arguments();

  bool haveUnknownOptions = false;
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
      } else {
          haveUnknownOptions = true;
          qDebug() << "Unknown option: " << args.at(i);
      }
  }
  if( haveUnknownOptions ) {
      qDebug() << "have unknown options, stop";
      exit(1);
  }

  miutil::conf::ConfSection *confSec = CorbaKvApp::readConf(myconf.toStdString());
  if(!confSec) {
    clog << "Can't open configuration file: " << myconf.toStdString() << endl;
    return 1;
  }
  CorbaKvApp kvapp(argc, argv, confSec);
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

  QString captionSuffix = QString::fromStdString(kvapp.kvpathInCorbaNameserver());
  QString caption = "HQC " + captionSuffix;
  mw->setCaption( caption );
  mw->setIcon( QPixmap( hqc::getPath(hqc::IMAGEDIR) + "/hqc.png") );
  a.setMainWidget(mw);

  mw->startup();

  a.connect( &a, SIGNAL(lastWindowClosed()), &a, SLOT(quit()) );
  int res = a.exec();

  return res;
}
