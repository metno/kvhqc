/*
  HQC - Free Software for Manual Quality Control of Meteorological Observations

  Copyright (C) 2018 met.no

  Contact information:
  Norwegian Meteorological Institute
  Box 43 Blindern
  0313 OSLO
  NORWAY
  email: kvalobs-dev@met.no

  This file is part of HQC

  HQC is free software; you can redistribute it and/or modify it under
  the terms of the GNU General Public License as published by the Free
  Software Foundation; either version 2 of the License, or (at your
  option) any later version.

  HQC is distributed in the hope that it will be useful, but WITHOUT ANY
  WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
  for more details.

  You should have received a copy of the GNU General Public License
  along with HQC; if not, write to the Free Software Foundation, Inc.,
  51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
*/

#ifndef DIANACLIENT_HH
#define DIANACLIENT_HH 1

#include "util/timeutil.hh"

#include <QObject>

#include <string>
#include <vector>

class DianaClient : public QObject
{
  Q_OBJECT

public:
  DianaClient(QObject* parent=0);
  virtual ~DianaClient();

  //! client has become active or inactive
  virtual void setActive(bool active) = 0;

  //! time selected in diana
  virtual void remoteTimeSelected(const timeutil::ptime& time) = 0;

  /*! Get time in hqc.
   * Should be the same as the time sent in clientTimeSelected.
   * \return current client time
   */
  virtual timeutil::ptime getClientTime() = 0;

  /*! Get time list in hqc.
   * Should be the same as the times sent in clientTimesChanged.
   * \return current client times
   */
  virtual std::vector<timeutil::ptime> getClientTimes() = 0;

Q_SIGNALS:
  //! time selected in hqc
  void clientTimeSelected(const timeutil::ptime& time);

  //! time list changed in hqc
  void clientTimesChanged(const std::vector<timeutil::ptime>& times);
};

#endif /* DIANACLIENT_HH */
