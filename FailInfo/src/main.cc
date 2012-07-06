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
//#include "FailList.h"
#include "FailDialog.h"

#include <iostream>

//#include <kvservice/qt/kvQtApp.h>
#include <kvcpp/KvApp.h>
#include <kvalobs/kvData.h>
#include <kvalobs/kvDataFlag.h>
#include <decodeutility/kvDataFormatter.h>

#include "hqc_paths.hh"
 
using namespace std;
using namespace FailInfo;
using namespace kvservice;
using namespace kvalobs;
using namespace miutil;
using namespace decodeutility;

int main( int argc, char ** argv ) {

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

  miutil::conf::ConfSection *confSec = KvApp::readConf(myconf.toStdString());
  if(!confSec) {
    clog << "Can't open configuration file: " << myconf << endl;
    return 1;
  }
  KvApp kvapp(argc, argv, confSec);

  QApplication a( argc, argv, true );

  FailDialog *f = new FailDialog();
  f->show();

  const int bufSiz = 1024;
  char buffer[bufSiz];
  while ( not cin.eof() ) {
    cin.getline( buffer, bufSiz -1 );
    miString data = buffer;
    data.trim();
    if ( data.empty() )
      continue;
    list<kvData> dataList;
    try {
      dataList = decodeutility::kvdataformatter::getKvData( data );
      cerr << data << endl;
    }
    catch(...) {
      cerr << "Ugyldig data: " << data << endl;
      continue;
    }

    f->newData( &*dataList.begin() );
  }
 
  a.connect( &a, SIGNAL(lastWindowClosed()), &a, SLOT(quit()) );
 
  return a.exec();
}
