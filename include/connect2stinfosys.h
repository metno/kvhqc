// -*- c++ -*-

#ifndef CONNECT2STINFOSYS_H
#define CONNECT2STINFOSYS_H

#include "timeutil.hh"
#include <QtCore/QString>
#include <string>

struct countyInfo {
  int stnr;
  QString name;
  QString county;
  QString municip;
  QString web;
  QString pri;
  QString ki;
};
bool connect2stinfosys();

struct listStat_t {
    std::string name;    // listStatName
    int stationid;       // listStatNum
    float altitude;      // listStatHoh
    int environment;     // listStatType
    std::string fylke;   // listStatFylke
    std::string kommune; // listStatKommune
    std::string wmonr;   // listStatWeb
    std::string pri;     // listStatPri
    timeutil::ptime fromtime;
    timeutil::ptime totime;
    bool coast;
};
typedef std::list<listStat_t> listStat_l;

#endif
