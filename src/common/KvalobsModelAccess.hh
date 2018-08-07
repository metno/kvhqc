/*
  HQC - Free Software for Manual Quality Control of Meteorological Observations

  Copyright (C) 2013-2018 met.no

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


#ifndef KvalobsModelAccess_hh
#define KvalobsModelAccess_hh 1

#include "ModelAccess.hh"
#include "ModelData.hh"
#include "QueryTaskHandler.hh"

#include <QObject>

class ModelRequest;
HQC_TYPEDEF_P(ModelRequest);
HQC_TYPEDEF_PV(ModelRequest);

class KvalobsModelAccess : public QObject, public ModelAccess
{ Q_OBJECT;
public:
  KvalobsModelAccess(QueryTaskHandler_p handler);
  ~KvalobsModelAccess();

  void postRequest(ModelRequest_p request) override;
  void dropRequest(ModelRequest_p request) override;

  void cleanCache();

private Q_SLOTS:
  void modelData(const ModelData_pv&);

private:
  QueryTaskHandler_p mHandler;
  ModelRequest_pv mRequests;

  typedef std::map<SensorTime, ModelData_p, lt_ModelSensorTime> ModelDataCache_t;
  ModelDataCache_t mCache;
};

#endif // KvalobsModelAccess_hh
