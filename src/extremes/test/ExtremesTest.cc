
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

static inline const Sensor& s(ObsData_p obs) { return obs->sensorTime().sensor; }
static inline const std::string t(ObsData_p obs) { return timeutil::to_iso_extended_string(obs->sensorTime().time); }
static inline float cv(ObsData_p obs) { return obs->corrected(); }

TEST(ExtremesTest, Filter)
{
  std::shared_ptr<FakeKvApp> fa(std::make_shared<FakeKvApp>(false)); // no threading
  KvServiceHelper kvsh(fa);
  KvMetaDataBuffer kvmdbuf;
  kvmdbuf.setHandler(fa->obsAccess()->handler());

  load_17000_20141002(*fa);
  KvMetaDataBuffer::instance()->reload();

  ExtremesFilter_p ef(new ExtremesFilter(kvalobs::PARAMID_TAX, 5));
  SortedBuffer::Ordering_p ordering = std::make_shared<ExtremesTableModel::CorrectedOrdering>(not ef->isMaximumSearch());
  SortedBuffer_p b = std::make_shared<SortedBuffer>(ordering, Sensor_s(), t_17000_20141002(), ef);
  b->syncRequest(fa->obsAccess());

  // grep '\<21[15]\>' src/extremes/test/data_17000_20141002.txt | sort -rn -k 7 | head -n 20
  // omit 216 in grep as it is aggregated / not in obs_pgm

  ASSERT_EQ(7, b->size()); // two have TA=TAX

  const ObsData_pv& d = b->data();
  const eq_Sensor eq;

  EXPECT_TRUE(eq(Sensor(17380, 215, 0, 0, 502), s(d[0])));
  EXPECT_TRUE(eq(Sensor(17050, 215, 0, 0, 502), s(d[1])));
  EXPECT_TRUE(eq(Sensor(17050, 215, 0, 0, 502), s(d[2])));
  EXPECT_TRUE(eq(Sensor(17150, 215, 0, 0, 342), s(d[3])));
  EXPECT_TRUE(eq(Sensor(17000, 211, 0, 0, 330), s(d[4])));
  EXPECT_TRUE(eq(Sensor(17000, 215, 0, 0, 330), s(d[5])));
  EXPECT_TRUE(eq(Sensor(18420, 215, 0, 0, 514), s(d[6])));

  EXPECT_EQ("2014-10-01 12:00:00", t(d[0]));
  EXPECT_EQ("2014-10-01 11:00:00", t(d[1]));
  EXPECT_EQ("2014-10-01 12:00:00", t(d[2]));
  EXPECT_EQ("2014-10-01 12:00:00", t(d[3]));
  EXPECT_EQ("2014-10-02 05:00:00", t(d[4]));
  EXPECT_EQ("2014-10-02 05:00:00", t(d[5]));
  EXPECT_EQ("2014-10-01 11:00:00", t(d[6]));

  EXPECT_FLOAT_EQ(16.8, cv(d[0]));
  EXPECT_FLOAT_EQ(15.7, cv(d[1]));
  EXPECT_FLOAT_EQ(15.7, cv(d[2]));
  EXPECT_FLOAT_EQ(15.4, cv(d[3]));
  EXPECT_FLOAT_EQ(14.7, cv(d[4]));
  EXPECT_FLOAT_EQ(14.7, cv(d[5]));
  EXPECT_FLOAT_EQ(14.6, cv(d[6]));

  // (s:17380, p:215, l:0, s:0, t:502)@2014-10-01 12:00:00 c= ci=0111100000000010
  // (s:17050, p:215, l:0, s:0, t:502)@2014-10-01 11:00:00 c= ci=0111100000000010
  // (s:17050, p:215, l:0, s:0, t:502)@2014-10-01 12:00:00 c= ci=0111100000000010
  // (s:17150, p:215, l:0, s:0, t:342)@2014-10-01 12:00:00 c= ci=0111100000000010
  // (s:17000, p:211, l:0, s:0, t:330)@2014-10-02 05:00:00 c= ci=0111100000100010
  // (s:17000, p:215, l:0, s:0, t:330)@2014-10-02 05:00:00 c= ci=0111100000000010
  // (s:18420, p:215, l:0, s:0, t:514)@2014-10-01 11:00:00 c= ci=0111100000000010
  // for (ObsData_pv::const_iterator it = d.begin(); it != d.end(); ++it)
  //   EXPECT_FALSE(*it) << (*it)->sensorTime() << " c=" << (*it)->corrected() << " ci=" << (*it)->controlinfo().flagstring();
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

  ExtremesFilter_p ef(new ExtremesFilter(kvalobs::PARAMID_TAX, 5));

  Sensor_s invalid;
  invalid.insert(Sensor());

  TimeBuffer_p b = std::make_shared<TimeBuffer>(invalid, t_17000_20141002(), ef);
  b->syncRequest(cache);

  ASSERT_EQ(7, b->size()); // two have TA=TAX
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

  EXPECT_EQ(34, tm.rowCount(QModelIndex()));
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

  ASSERT_EQ(34, tm.rowCount(QModelIndex()));

  const SensorTime st0(Sensor(17380, 215, 0, 0, 502), s2t("2014-10-01 12:00:00"));
  const SensorTime st1(Sensor(17050, 215, 0, 0, 502), s2t("2014-10-01 11:00:00"));
  const float newC = 20.0;

  { ObsData_p obs0 = tm.getObs(0);
    ASSERT_TRUE((bool)obs0);
    EXPECT_TRUE(eq_SensorTime()(st0, obs0->sensorTime()));
    EXPECT_FLOAT_EQ(16.8, obs0->corrected());
  }

  { ObsData_p obs1 = tm.getObs(1);
    ASSERT_TRUE((bool)obs1);
    EXPECT_TRUE(eq_SensorTime()(st1, obs1->sensorTime()));
    EXPECT_FLOAT_EQ(15.7, obs1->corrected());

    edit->newVersion();
    ObsUpdate_pv updates;
    
    ObsUpdate_p up = edit->createUpdate(obs1);
    ASSERT_TRUE((bool)up);
    
    up->setCorrected(newC);
    updates.push_back(up);
    EXPECT_TRUE(edit->storeUpdates(updates));

    // TODO check modelReset signal
  }

  { ObsData_p obs0 = tm.getObs(0);
    ASSERT_TRUE((bool)obs0);
    EXPECT_TRUE(eq_SensorTime()(st1, obs0->sensorTime())) << obs0->sensorTime();
    EXPECT_FLOAT_EQ(newC, obs0->corrected());
  }

  edit->undoVersion();

  { ObsData_p obs0 = tm.getObs(0);
    ASSERT_TRUE((bool)obs0);
    EXPECT_TRUE(eq_SensorTime()(st0, obs0->sensorTime()));
    EXPECT_FLOAT_EQ(16.8, obs0->corrected());
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
