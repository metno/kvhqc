#ifndef CONNECT2STINFOSYS_H
#define CONNECT2STINFOSYS_H


using namespace std;

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

#endif
