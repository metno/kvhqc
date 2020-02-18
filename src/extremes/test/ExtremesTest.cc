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


#include "extremes/ExtremesFilter.hh"
#include "extremes/ExtremesTableModel.hh"

#include "common/CachingAccess.hh"
#include "common/EditAccess.hh"
#include "common/KvHelpers.hh"
#include "common/KvMetaDataBuffer.hh"
#include "common/KvServiceHelper.hh"
#include "common/SingleObsBuffer.hh"
#include "common/TimeBuffer.hh"

#include "util/Synchronizer.hh"

#define LOAD_DECL_ONLY
#include "load_17000_20141002.cc"

#define MILOGGER_CATEGORY "kvhqc.test.ExtremesTest"
#include "common/ObsLogging.hh"

namespace {
const int N_REQUEST = 5;
const int N_FILTER_RESULT = 12; // some repetition
const int N_TABLE_RESULT = 52; // table asks for more

std::string message(ObsData_p obs)
{
  std::ostringstream msg;
  msg << obs->sensorTime() << " c=" << obs->corrected() << " ci=" << obs->controlinfo().flagstring();
  return msg.str();
}
}

TEST(ExtremesTest, Filter)
{
  std::shared_ptr<FakeKvApp> fa(std::make_shared<FakeKvApp>(false)); // no threading
  KvServiceHelper kvsh(fa);
  KvMetaDataBuffer kvmdbuf;
  kvmdbuf.setHandler(fa->obsAccess()->handler());

  load_17000_20141002(*fa);
  KvMetaDataBuffer::instance()->reload();

  ExtremesFilter_p ef(new ExtremesFilter(kvalobs::PARAMID_TAX, N_REQUEST));
  SortedBuffer::Ordering_p ordering = std::make_shared<ExtremesTableModel::CorrectedOrdering>(not ef->isMaximumSearch());
  SortedBuffer_p b = std::make_shared<SortedBuffer>(ordering, Sensor_s(), t_17000_20141002(), ef);
  b->syncRequest(fa->obsAccess());

  // grep '\<21[156]\>' src/extremes/test/data_17000_20141002.txt | sort -rn -k 7 | head -n 30

  EXPECT_EQ(size_t(N_FILTER_RESULT), b->size()); // two have TA=TAX
  std::set<float> corrected;
  for (auto obs : b->data()) {
    EXPECT_LE(14.6, obs->corrected()) << message(obs);
    corrected.insert(obs->corrected());
  }
  EXPECT_EQ(size_t(N_REQUEST), corrected.size());
}

TEST(ExtremesTest, FilterCached)
{
  std::shared_ptr<FakeKvApp> fa(std::make_shared<FakeKvApp>(false)); // no threading
  KvServiceHelper kvsh(fa);
  KvMetaDataBuffer kvmdbuf;
  kvmdbuf.setHandler(fa->obsAccess()->handler());

  load_17000_20141002(*fa);
  KvMetaDataBuffer::instance()->reload();

  CachingAccess_p cache(new CachingAccess(fa->obsAccess()));

  ExtremesFilter_p ef(new ExtremesFilter(kvalobs::PARAMID_TAX, N_REQUEST));

  Sensor_s invalid;
  invalid.insert(Sensor());

  TimeBuffer_p b = std::make_shared<TimeBuffer>(invalid, t_17000_20141002(), ef);
  b->syncRequest(cache);

  ASSERT_EQ(size_t(N_FILTER_RESULT), b->size());
}

TEST(ExtremesTest, TableModel)
{
  std::shared_ptr<FakeKvApp> fa(std::make_shared<FakeKvApp>(true)); // with threading
  KvServiceHelper kvsh(fa);
  KvMetaDataBuffer kvmdbuf;
  kvmdbuf.setHandler(fa->obsAccess()->handler());

  load_17000_20141002(*fa);
  KvMetaDataBuffer::instance()->reload();

#if 1
  CachingAccess_p cache(new CachingAccess(fa->obsAccess()));
  EditAccess_p edit(new EditAccess(cache));
#else
  EditAccess_p edit(new EditAccess(fa.obsAccess()));
#endif

  Synchronizer sync;
  ExtremesTableModel tm(edit);
  QObject::connect(&tm, SIGNAL(modelReset()), &sync, SLOT(taskDone()));
  tm.search(kvalobs::PARAMID_TAX, t_17000_20141002());
  sync.waitForSignal();

  EXPECT_EQ(N_TABLE_RESULT, tm.rowCount(QModelIndex()));
}

TEST(ExtremesTest, TableModelUpdateTAX)
{
  std::shared_ptr<FakeKvApp> fa(std::make_shared<FakeKvApp>(true)); // with threading
  KvServiceHelper kvsh(fa);
  KvMetaDataBuffer kvmdbuf;
  kvmdbuf.setHandler(fa->obsAccess()->handler());

  load_17000_20141002(*fa);
  KvMetaDataBuffer::instance()->reload();

  CachingAccess_p cache(new CachingAccess(fa->obsAccess()));
  EditAccess_p edit(new EditAccess(cache));

  Synchronizer sync;
  ExtremesTableModel tm(edit);
  QObject::connect(&tm, SIGNAL(modelReset()), &sync, SLOT(taskDone()));
  tm.search(kvalobs::PARAMID_TAX, t_17000_20141002());
  sync.waitForSignal();

  ASSERT_EQ(N_TABLE_RESULT, tm.rowCount(QModelIndex()));

  const SensorTime st0(Sensor(17380, 215, 0, 0, 502), s2t("2014-10-01 12:00:00"));
  const SensorTime st1(Sensor(17050, 215, 0, 0, 502), s2t("2014-10-01 11:00:00"));
  const float newC = 20.0;

#if 0
  for (int i=0; i<tm.rowCount(QModelIndex()); ++i)
    EXPECT_FALSE(true) << message(tm.getObs(i));
#endif

  { ObsData_p obs = tm.getObs(0);
    ASSERT_TRUE((bool)obs);
    EXPECT_TRUE(eq_SensorTime()(st0, obs->sensorTime())) << obs;
    EXPECT_FLOAT_EQ(16.8, obs->corrected()) << message(obs);
  }

  { ObsData_p obs = tm.getObs(2);
    ASSERT_TRUE((bool)obs);
    EXPECT_TRUE(eq_SensorTime()(st1, obs->sensorTime())) << message(obs);
    EXPECT_FLOAT_EQ(15.7, obs->corrected()) << message(obs);

    edit->newVersion();
    ObsUpdate_pv updates;
    
    ObsUpdate_p up = edit->createUpdate(obs);
    ASSERT_TRUE((bool)up);

    up->setCorrected(newC);
    updates.push_back(up);
    EXPECT_TRUE(edit->storeUpdates(updates));

    // TODO check modelReset signal
  }

  { ObsData_p obs = tm.getObs(0);
    ASSERT_TRUE((bool)obs);
    EXPECT_TRUE(eq_SensorTime()(st1, obs->sensorTime())) << message(obs);
    EXPECT_FLOAT_EQ(newC, obs->corrected()) << message(obs);
  }

  edit->undoVersion();

  { ObsData_p obs = tm.getObs(0);
    ASSERT_TRUE((bool)obs);
    EXPECT_TRUE(eq_SensorTime()(st0, obs->sensorTime())) << message(obs);
    EXPECT_FLOAT_EQ(16.8, obs->corrected()) << message(obs);
  }
}

#if 0
// these tests are more like a wish-list right now
TEST(ExtremesTest, TableModelUpdateTAN)
{
  FakeKvApp fa(true); // with threading
  KvServiceHelper kvsh;
  KvMetaDataBuffer kvmdbuf;
  kvmdbuf.setHandler(fa.obsAccess()->handler());

  load_17000_20141002(fa);
  KvMetaDataBuffer::instance()->reload();

  EditAccess_p edit(new EditAccess(fa.obsAccess()));

  Synchronizer sync;
  ExtremesTableModel tm(edit);
  QObject::connect(&tm, SIGNAL(modelReset()), &sync, SLOT(taskDone()));
  tm.search(kvalobs::PARAMID_TAN, t_17000_20141002());
  sync.waitForSignal();

  const int NROWS = 23;
  ASSERT_EQ(NROWS, tm.rowCount(QModelIndex()));

  const SensorTime st0(Sensor(17380, 213, 0, 0, 502), s2t("2014-10-01 06:00:00"));
  const SensorTime st1(Sensor(18500, 214, 0, 0, 342), s2t("2014-10-01 06:00:00"));
  const SensorTime st2(Sensor(17280, 211, 0, 0, 330), s2t("2014-10-01 06:00:00"));
  const float newC = 0.0;

  { ObsData_p obs0 = tm.getObs(0);
    ASSERT_TRUE(obs0);
    EXPECT_TRUE(eq_SensorTime()(st0, obs0->sensorTime()));
    EXPECT_FLOAT_EQ(1.2, obs0->corrected());
  }

  // check that st2 does not appear in model
  for (int i=0; i<tm.rowCount(QModelIndex()); ++i)
    EXPECT_FALSE(eq_SensorTime()(st2, tm.getObs(i)->sensorTime())) << "found at row " << i;
  
  // fetch st2 into singleobsbuffer
  SingleObsBuffer_p buf2(new SingleObsBuffer(st2));
  buf2->syncRequest(edit);
  ObsData_p obs2 = buf2->get();
  ASSERT_TRUE(obs2);

  // update st2 to corr=newC
  { edit->newVersion();
    ObsUpdate_pv updates;
    
    ObsUpdate_p up = edit->createUpdate(obs2);
    ASSERT_TRUE(up);
    
    up->setCorrected(newC);
    updates.push_back(up);
    EXPECT_TRUE(edit->storeUpdates(updates));
  }

  // check that st2 comes first in extremes value model
  ASSERT_LE(1, tm.rowCount(QModelIndex()));
  EXPECT_TRUE(eq_SensorTime()(st2, tm.getObs(0)->sensorTime()));

  // undo change of corrected value
  edit->undoVersion();

  // check that st2 does not appear in model any more
  for (int i=0; i<tm.rowCount(QModelIndex()); ++i)
    EXPECT_FALSE(eq_SensorTime()(st2, tm.getObs(i)->sensorTime())) << "found at row " << i;
}

TEST(ExtremesTest, TableModelAfterEditLow)
{
  FakeKvApp fa(true); // with threading
  KvServiceHelper kvsh;
  KvMetaDataBuffer kvmdbuf;
  kvmdbuf.setHandler(fa.obsAccess()->handler());

  load_17000_20141002(fa);
  KvMetaDataBuffer::instance()->reload();

  EditAccess_p edit(new EditAccess(fa.obsAccess()));

  const SensorTime st0(Sensor(17380, 213, 0, 0, 502), s2t("2014-10-01 06:00:00"));
  const SensorTime st1(Sensor(18500, 214, 0, 0, 342), s2t("2014-10-01 06:00:00"));
  const SensorTime st2(Sensor(17280, 211, 0, 0, 330), s2t("2014-10-01 06:00:00"));
  const float newC = -25.0; // lower than TAN

  // fetch st2 into singleobsbuffer
  SingleObsBuffer_p buf2(new SingleObsBuffer(st2));
  buf2->syncRequest(edit);
  ObsData_p obs2 = buf2->get();
  ASSERT_TRUE(obs2);

  // update st2 to corr=newC
  { edit->newVersion();
    ObsUpdate_pv updates;
    
    ObsUpdate_p up = edit->createUpdate(obs2);
    ASSERT_TRUE(up);
    
    up->setCorrected(newC);
    updates.push_back(up);
    EXPECT_TRUE(edit->storeUpdates(updates));
  }

  Synchronizer sync;
  ExtremesTableModel tm(edit);
  QObject::connect(&tm, SIGNAL(modelReset()), &sync, SLOT(taskDone()));
  tm.search(kvalobs::PARAMID_TAN, t_17000_20141002());
  sync.waitForSignal();

  const int NROWS = 23;
  ASSERT_EQ(NROWS, tm.rowCount(QModelIndex()));

  { ObsData_p obs0 = tm.getObs(0);
    ASSERT_TRUE(obs0);
    EXPECT_TRUE(eq_SensorTime()(st2, obs0->sensorTime())) << "expected st2, got " << obs0->sensorTime();
    EXPECT_FLOAT_EQ(newC, obs0->corrected());
  }

  // undo change of corrected value
  edit->undoVersion();

  // check that st2 does not appear in model any more
  for (int i=0; i<tm.rowCount(QModelIndex()); ++i)
    EXPECT_FALSE(eq_SensorTime()(st2, tm.getObs(i)->sensorTime())) << "found at row " << i;
}

TEST(ExtremesTest, TableModelAfterEditHigh)
{
  FakeKvApp fa(true); // with threading
  KvServiceHelper kvsh;
  KvMetaDataBuffer kvmdbuf;
  kvmdbuf.setHandler(fa.obsAccess()->handler());

  load_17000_20141002(fa);
  KvMetaDataBuffer::instance()->reload();

  EditAccess_p edit(new EditAccess(fa.obsAccess()));

  const SensorTime st0(Sensor(17380, 213, 0, 0, 502), s2t("2014-10-01 06:00:00"));
  const SensorTime st1(Sensor(18500, 214, 0, 0, 342), s2t("2014-10-01 06:00:00"));
  const SensorTime st2(Sensor(17280, 211, 0, 0, 330), s2t("2014-10-01 06:00:00"));
  const float newC = 25.0; // higher than TAX, see above

  // fetch st2 into singleobsbuffer
  SingleObsBuffer_p buf2(new SingleObsBuffer(st2));
  buf2->syncRequest(edit);
  ObsData_p obs2 = buf2->get();
  ASSERT_TRUE(obs2);

  // update st2 to corr=newC
  { edit->newVersion();
    ObsUpdate_pv updates;
    
    ObsUpdate_p up = edit->createUpdate(obs2);
    ASSERT_TRUE(up);
    
    up->setCorrected(newC);
    updates.push_back(up);
    EXPECT_TRUE(edit->storeUpdates(updates));
  }

  Synchronizer sync;
  ExtremesTableModel tm(edit);
  QObject::connect(&tm, SIGNAL(modelReset()), &sync, SLOT(taskDone()));
  tm.search(kvalobs::PARAMID_TAN, t_17000_20141002());
  sync.waitForSignal();

  const int NROWS = 23;
  ASSERT_EQ(NROWS, tm.rowCount(QModelIndex()));

  { ObsData_p obs0 = tm.getObs(0);
    ASSERT_TRUE(obs0);
    EXPECT_TRUE(eq_SensorTime()(st0, obs0->sensorTime())) << "expected st0, got " << obs0->sensorTime();
    EXPECT_FLOAT_EQ(1.2, obs0->corrected());
  }

  // check that st2 does not appear in model
  for (int i=0; i<tm.rowCount(QModelIndex()); ++i)
    //EXPECT_FALSE(eq_SensorTime()(st2, tm.getObs(i)->sensorTime())) << "found at row " << i;
    EXPECT_FALSE(1) << " st=" << tm.getObs(i)->sensorTime() << " c=" << tm.getObs(i)->corrected();
}
#endif
