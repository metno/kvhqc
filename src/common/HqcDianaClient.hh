/* -*- c++ -*-
   HQC - Free Software for Manual Quality Control of Meteorological Observations

   Copyright (C) 2014-2018 met.no

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

#ifndef HQC_HQCDIANACLIENT_H
#define HQC_HQCDIANACLIENT_H

#include "common/DianaClient.hh"
#include "common/Sensor.hh"
#include "common/TimeSpan.hh"

QT_BEGIN_NAMESPACE
class QAction;
QT_END_NAMESPACE

class DianaHelper;
class HqcAppWindow;

class HqcDianaClient : public DianaClient {
  Q_OBJECT

public:
  HqcDianaClient(DianaHelper* dh, QObject* parent=0);
  ~HqcDianaClient();

  QAction* getToolButtonAction()
    { return mActionForToolButton; }

  void setActive(bool active) override;
  void remoteTimeSelected(const timeutil::ptime& time) override;
  timeutil::ptime getClientTime() override;
  std::vector<timeutil::ptime> getClientTimes() override;

protected:
  virtual bool doNavigateTo(const SensorTime& st);

public Q_SLOTS:
  void navigateTo(const SensorTime& st);

private Q_SLOTS:
  void makeActive(bool);

Q_SIGNALS:
  void signalNavigateTo(const SensorTime&);
  void requestClientActive(DianaClient* client, bool active);

protected:
  SensorTime st_;
  TimeSpan ts_;
  std::vector<timeutil::ptime> times_;

private
  :DianaHelper* mDianaHelper;
  QAction* mActionForToolButton;
};

#endif // HQC_HQCDIANACLIENT_H
