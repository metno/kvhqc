
#include "CachingAccess.hh"
#include "CountingBuffer.hh"
#include "EditAccess.hh"
#include "SqliteAccess.hh"

#include "Functors.hh"
#include "util/make_set.hh"

#include <boost/make_shared.hpp>

#include <gtest/gtest.h>

namespace /*anonymous*/ {

inline timeutil::ptime s2t(const std::string& t)
{ return timeutil::from_iso_extended_string(t); }

class Updater {
public:
  Updater(ObsAccess_p ea, TimeBuffer_p buffer)
    : mOA(ea), mBuffer(buffer) { }

  ObsUpdate_p createUpdate(const SensorTime& st);
  void setCorrected(const SensorTime& st, float newC);

  void send();

private:
  ObsAccess_p mOA;
  TimeBuffer_p mBuffer;
  ObsUpdate_pv mUpdates;
};

void Updater::setCorrected(const SensorTime& st, float newC)
{
  ObsUpdate_p up = createUpdate(st);
  up->setCorrected(newC);
  send();
}

ObsUpdate_p Updater::createUpdate(const SensorTime& st)
{
  if (ObsData_p obs = mBuffer->get(st))
    mUpdates.push_back(mOA->createUpdate(obs));
  else
    mUpdates.push_back(mOA->createUpdate(st));
  return mUpdates.back();
}

void Updater::send()
{
  mOA->storeUpdates(mUpdates);
  mUpdates.clear();
}

const float NO_OBS = -32765;

inline float corrected(TimeBuffer_p buffer, const SensorTime& st)
{
  if (ObsData_p obs = buffer->get(st))
    return obs->corrected();
  else
    return NO_OBS;
}

} // namespace anonymous

// ========================================================================

TEST(EditAccessTest, UndoRedoStore)
{
  SqliteAccess_p sqla(new SqliteAccess);
  sqla->insertDataFromFile(std::string(TEST_SOURCE_DIR)+"/data_18210_20130410.txt");
  CachingAccess_p ca(new CachingAccess(sqla));
  EditAccess_p ea(new EditAccess(ca));

  const Sensor sensor1(18210, 211, 0, 0, 514);
  //const Sensor_s sensors = (SetMaker<Sensor_s>() << sensor1 << sensor2).set();
  const TimeSpan time(s2t("2013-04-01 00:00:00"), s2t("2013-04-01 06:00:00"));

  const Time t1 = s2t("2013-04-01 01:00:00"), t2 = s2t("2013-04-01 02:00:00");
  const SensorTime st1(sensor1, t1), st2(sensor1, t2);

  const float ORIG_C1 = -4.5f, ORIG_C2 = -5.0f;
  const float CHNG_C1 =  4.0f, CHNG_C2 =  3.0f;

  CountingBuffer_p counter(new CountingBuffer(sensor1, time));
  counter->postRequest(ea);
  EXPECT_EQ(7, counter->size());
  EXPECT_EQ(1, counter->countComplete);
  EXPECT_EQ(0, ea->currentVersion());
  EXPECT_EQ(0, ea->highestVersion());

  EXPECT_FLOAT_EQ(ORIG_C1, corrected(counter, st1));
  EXPECT_FLOAT_EQ(ORIG_C2, corrected(counter, st2));

  ea->newVersion();
  EXPECT_EQ(1, ea->currentVersion());
  EXPECT_EQ(1, ea->highestVersion());

  { Updater updater(ea, counter);
    ObsUpdate_p up = updater.createUpdate(st1);
    up->setCorrected(CHNG_C1);
    
    up = updater.createUpdate(st2);
    up->setCorrected(CHNG_C2);
    
    updater.send();
  }

  EXPECT_EQ(7, counter->size());
  EXPECT_EQ(1, counter->countUpdate);

  EXPECT_FLOAT_EQ(CHNG_C1, corrected(counter, st1));
  EXPECT_FLOAT_EQ(CHNG_C2, corrected(counter, st2));
  EXPECT_EQ(2, ea->countU());

  ea->undoVersion();
  EXPECT_EQ(0, ea->currentVersion());
  EXPECT_EQ(1, ea->highestVersion());
  EXPECT_EQ(2, counter->countUpdate);

  EXPECT_FLOAT_EQ(ORIG_C1, corrected(counter, st1));
  EXPECT_FLOAT_EQ(ORIG_C2, corrected(counter, st2));
  EXPECT_EQ(0, ea->countU());

  ea->redoVersion();
  EXPECT_EQ(1, ea->currentVersion());
  EXPECT_EQ(1, ea->highestVersion());
  EXPECT_EQ(3, counter->countUpdate);

  EXPECT_FLOAT_EQ(CHNG_C1, corrected(counter, st1));
  EXPECT_FLOAT_EQ(CHNG_C2, corrected(counter, st2));
  EXPECT_EQ(2, ea->countU());

  EXPECT_TRUE(ea->storeToBackend());

  EXPECT_EQ(0, ea->currentVersion());
  EXPECT_EQ(0, ea->highestVersion());
  EXPECT_EQ(4, counter->countUpdate);

  EXPECT_FLOAT_EQ(CHNG_C1, corrected(counter, st1));
  EXPECT_FLOAT_EQ(CHNG_C2, corrected(counter, st2));
  EXPECT_EQ(0, ea->countU());
}

TEST(EditAccessTest, UndoRedoTwice)
{
  SqliteAccess_p sqla(new SqliteAccess);
  sqla->insertDataFromFile(std::string(TEST_SOURCE_DIR)+"/data_18210_20130410.txt");
  CachingAccess_p ca(new CachingAccess(sqla));
  EditAccess_p ea(new EditAccess(ca));

  const Sensor sensor1(18210, 211, 0, 0, 514);
  //const Sensor_s sensors = (SetMaker<Sensor_s>() << sensor1 << sensor2).set();
  const TimeSpan time(s2t("2013-04-01 00:00:00"), s2t("2013-04-01 06:00:00"));

  const Time t1 = s2t("2013-04-01 01:00:00"), t2 = s2t("2013-04-01 02:00:00");
  const SensorTime st1(sensor1, t1), st2(sensor1, t2);

  const float ORIG_C1 = -4.5f, ORIG_C2 = -5.0f;
  const float CHNG_C1 =  4.0f, CHNG_C2 =  3.0f;

  CountingBuffer_p counter(new CountingBuffer(sensor1, time));
  counter->postRequest(ea);
  EXPECT_EQ(7, counter->size());
  EXPECT_EQ(1, counter->countComplete);
  EXPECT_EQ(0, ea->currentVersion());
  EXPECT_EQ(0, ea->highestVersion());

  EXPECT_FLOAT_EQ(ORIG_C1, corrected(counter, st1));
  EXPECT_FLOAT_EQ(ORIG_C2, corrected(counter, st2));

  Updater updater(ea, counter);

  ea->newVersion();
  updater.setCorrected(st1, CHNG_C1);
  EXPECT_EQ(1, ea->currentVersion());
  EXPECT_EQ(1, ea->highestVersion());
  EXPECT_FLOAT_EQ(CHNG_C1, corrected(counter, st1));
  EXPECT_FLOAT_EQ(ORIG_C2, corrected(counter, st2));
  EXPECT_EQ(1, ea->countU());

  ea->newVersion();
  updater.setCorrected(st2, CHNG_C2);
  EXPECT_EQ(2, ea->currentVersion());
  EXPECT_EQ(2, ea->highestVersion());
  EXPECT_FLOAT_EQ(CHNG_C1, corrected(counter, st1));
  EXPECT_FLOAT_EQ(CHNG_C2, corrected(counter, st2));
  EXPECT_EQ(2, ea->countU());

  ea->undoVersion();
  EXPECT_EQ(1, ea->currentVersion());
  EXPECT_EQ(2, ea->highestVersion());
  EXPECT_EQ(3, counter->countUpdate);
  EXPECT_FLOAT_EQ(CHNG_C1, corrected(counter, st1));
  EXPECT_FLOAT_EQ(ORIG_C2, corrected(counter, st2));
  EXPECT_EQ(1, ea->countU());

  ea->undoVersion();
  EXPECT_EQ(0, ea->currentVersion());
  EXPECT_EQ(2, ea->highestVersion());
  EXPECT_EQ(4, counter->countUpdate);
  EXPECT_FLOAT_EQ(ORIG_C1, corrected(counter, st1));
  EXPECT_FLOAT_EQ(ORIG_C2, corrected(counter, st2));
  EXPECT_EQ(0, ea->countU());

  ea->redoVersion();
  EXPECT_EQ(1, ea->currentVersion());
  EXPECT_EQ(2, ea->highestVersion());
  EXPECT_EQ(5, counter->countUpdate);
  EXPECT_FLOAT_EQ(CHNG_C1, corrected(counter, st1));
  EXPECT_FLOAT_EQ(ORIG_C2, corrected(counter, st2));
  EXPECT_EQ(1, ea->countU());

  ea->redoVersion();
  EXPECT_EQ(2, ea->currentVersion());
  EXPECT_EQ(2, ea->highestVersion());
  EXPECT_EQ(6, counter->countUpdate);
  EXPECT_FLOAT_EQ(CHNG_C1, corrected(counter, st1));
  EXPECT_FLOAT_EQ(CHNG_C2, corrected(counter, st2));
  EXPECT_EQ(2, ea->countU());
}

TEST(EditAccessTest, UndoRedoNew1)
{
  SqliteAccess_p sqla(new SqliteAccess);
  sqla->insertDataFromFile(std::string(TEST_SOURCE_DIR)+"/data_18210_20130410.txt");
  CachingAccess_p ca(new CachingAccess(sqla));
  EditAccess_p ea(new EditAccess(ca));

  const Sensor sensor1(18210, 211, 0, 0, 514);
  //const Sensor_s sensors = (SetMaker<Sensor_s>() << sensor1 << sensor2).set();
  const TimeSpan time(s2t("2013-04-01 00:00:00"), s2t("2013-04-01 06:00:00"));

  const Time t1 = s2t("2013-04-01 01:00:00"), t2 = s2t("2013-04-01 02:00:00");
  const SensorTime st1(sensor1, t1), st2(sensor1, t2);

  const float ORIG_C1 = -4.5f, ORIG_C2 = -5.0f;
  const float CHNG_C1a = 4.0f, CHNG_C1b = 4.1f, CHNG_C1c = 4.2f, CHNG_C2 =  3.0f;

  CountingBuffer_p counter(new CountingBuffer(sensor1, time));
  counter->postRequest(ea);
  EXPECT_EQ(0, ea->currentVersion());
  EXPECT_EQ(0, ea->highestVersion());
  EXPECT_FLOAT_EQ(ORIG_C1, corrected(counter, st1));
  EXPECT_FLOAT_EQ(ORIG_C2, corrected(counter, st2));

  Updater updater(ea, counter);

  ea->newVersion();
  updater.setCorrected(st1, CHNG_C1a);
  EXPECT_EQ(1, ea->currentVersion());
  EXPECT_EQ(1, ea->highestVersion());
  EXPECT_FLOAT_EQ(CHNG_C1a, corrected(counter, st1));
  EXPECT_FLOAT_EQ(ORIG_C2, corrected(counter, st2));
  EXPECT_EQ(1, ea->countU());

  ea->newVersion();
  updater.setCorrected(st2, CHNG_C2);
  EXPECT_EQ(2, ea->currentVersion());
  EXPECT_EQ(2, ea->highestVersion());
  EXPECT_FLOAT_EQ(CHNG_C1a, corrected(counter, st1));
  EXPECT_FLOAT_EQ(CHNG_C2, corrected(counter, st2));
  EXPECT_EQ(2, ea->countU());

  ea->newVersion();
  updater.setCorrected(st1, CHNG_C1b);
  EXPECT_EQ(3, ea->currentVersion());
  EXPECT_EQ(3, ea->highestVersion());
  EXPECT_FLOAT_EQ(CHNG_C1b, corrected(counter, st1));
  EXPECT_FLOAT_EQ(CHNG_C2, corrected(counter, st2));
  EXPECT_EQ(2, ea->countU());

  ea->undoVersion();
  EXPECT_EQ(2, ea->currentVersion());
  EXPECT_EQ(3, ea->highestVersion());
  EXPECT_FLOAT_EQ(CHNG_C1a, corrected(counter, st1));
  EXPECT_FLOAT_EQ(CHNG_C2, corrected(counter, st2));
  EXPECT_EQ(2, ea->countU());

  ea->undoVersion();
  EXPECT_EQ(1, ea->currentVersion());
  EXPECT_EQ(3, ea->highestVersion());
  EXPECT_FLOAT_EQ(CHNG_C1a, corrected(counter, st1));
  EXPECT_FLOAT_EQ(ORIG_C2, corrected(counter, st2));
  EXPECT_EQ(1, ea->countU());

  ea->newVersion();
  updater.setCorrected(st1, CHNG_C1c);
  EXPECT_EQ(2, ea->currentVersion());
  EXPECT_EQ(2, ea->highestVersion());
  EXPECT_FLOAT_EQ(CHNG_C1c, corrected(counter, st1));
  EXPECT_FLOAT_EQ(ORIG_C2, corrected(counter, st2));
  EXPECT_EQ(1, ea->countU());
}

TEST(EditAccessTest, UndoRedoNew2)
{
  SqliteAccess_p sqla(new SqliteAccess);
  sqla->insertDataFromFile(std::string(TEST_SOURCE_DIR)+"/data_18210_20130410.txt");
  CachingAccess_p ca(new CachingAccess(sqla));
  EditAccess_p ea(new EditAccess(ca));

  const Sensor sensor1(18210, 211, 0, 0, 514);
  //const Sensor_s sensors = (SetMaker<Sensor_s>() << sensor1 << sensor2).set();
  const TimeSpan time(s2t("2013-04-01 00:00:00"), s2t("2013-04-01 06:00:00"));

  const Time t1 = s2t("2013-04-01 01:00:00"), t2 = s2t("2013-04-01 02:00:00");
  const SensorTime st1(sensor1, t1), st2(sensor1, t2);
  const float CHNG_C1a = 4.0f, CHNG_C1b = 4.1f, CHNG_C2 =  3.0f;

  CountingBuffer_p counter(new CountingBuffer(sensor1, time));
  counter->postRequest(ea);

  Updater updater(ea, counter);

  ea->newVersion();
  updater.setCorrected(st1, CHNG_C1a);
  EXPECT_EQ(1, ea->currentVersion());
  EXPECT_EQ(1, ea->highestVersion());
  EXPECT_EQ(1, ea->countU());

  ea->newVersion();
  updater.setCorrected(st2, CHNG_C2);
  EXPECT_EQ(2, ea->currentVersion());
  EXPECT_EQ(2, ea->highestVersion());
  EXPECT_EQ(2, ea->countU());

  ea->undoVersion();
  EXPECT_EQ(1, ea->countU());
  ea->undoVersion();
  EXPECT_EQ(0, ea->countU());

  ea->newVersion();
  updater.setCorrected(st1, CHNG_C1b);
  EXPECT_EQ(1, ea->currentVersion());
  EXPECT_EQ(1, ea->highestVersion());
  EXPECT_EQ(1, ea->countU());
}

TEST(EditAccessTest, Reset)
{
  SqliteAccess_p sqla(new SqliteAccess);
  sqla->insertDataFromFile(std::string(TEST_SOURCE_DIR)+"/data_18210_20130410.txt");
  CachingAccess_p ca(new CachingAccess(sqla));
  EditAccess_p ea(new EditAccess(ca));

  const Sensor sensor1(18210, 211, 0, 0, 514);
  const TimeSpan time(s2t("2013-04-01 00:00:00"), s2t("2013-04-01 06:00:00"));

  const Time t1 = s2t("2013-04-01 01:00:00"), t2 = s2t("2013-04-01 02:00:00");
  const SensorTime st1(sensor1, t1), st2(sensor1, t2);

  const float ORIG_C1 = -4.5f, ORIG_C2 = -5.0f;
  const float CHNG_C1 =  4.0f, CHNG_C2 =  3.0f;

  CountingBuffer_p counter(new CountingBuffer(sensor1, time));
  counter->postRequest(ea);
  EXPECT_EQ(7, counter->size());
  EXPECT_EQ(1, counter->countComplete);
  EXPECT_EQ(0, ea->currentVersion());
  EXPECT_EQ(0, ea->highestVersion());

  EXPECT_FLOAT_EQ(ORIG_C1, corrected(counter, st1));
  EXPECT_FLOAT_EQ(ORIG_C2, corrected(counter, st2));

  ea->newVersion();
  EXPECT_EQ(1, ea->currentVersion());
  EXPECT_EQ(1, ea->highestVersion());

  { Updater updater(ea, counter);
    ObsUpdate_p up = updater.createUpdate(st1);
    up->setCorrected(CHNG_C1);
    
    up = updater.createUpdate(st2);
    up->setCorrected(CHNG_C2);
    
    updater.send();
  }

  EXPECT_EQ(7, counter->size());
  EXPECT_EQ(1, counter->countUpdate);

  EXPECT_FLOAT_EQ(CHNG_C1, corrected(counter, st1));
  EXPECT_FLOAT_EQ(CHNG_C2, corrected(counter, st2));

  ea->reset();
  EXPECT_EQ(0, ea->currentVersion());
  EXPECT_EQ(0, ea->highestVersion());
  EXPECT_EQ(2, counter->countUpdate);

  EXPECT_FLOAT_EQ(ORIG_C1, corrected(counter, st1));
  EXPECT_FLOAT_EQ(ORIG_C2, corrected(counter, st2));
}

TEST(EditAccessTest, CreateReset)
{
  SqliteAccess_p sqla(new SqliteAccess);
  sqla->insertDataFromFile(std::string(TEST_SOURCE_DIR)+"/data_18210_20130410.txt");
  CachingAccess_p ca(new CachingAccess(sqla));
  EditAccess_p ea(new EditAccess(ca));

  const Sensor sensor1(18210, 211, 0, 0, 514);
  const TimeSpan time(s2t("2013-04-01 00:00:00"), s2t("2013-04-01 02:00:00"));

  const Time t1 = s2t("2013-04-01 01:30:00");
  const SensorTime st1(sensor1, t1);

  const float CHNG_C1 = 5.0f;

  CountingBuffer_p counter(new CountingBuffer(sensor1, time));
  counter->postRequest(ea);
  EXPECT_EQ(3, counter->size());
  EXPECT_EQ(1, counter->countComplete);
  EXPECT_EQ(1, counter->countNew);
  EXPECT_EQ(0, ea->currentVersion());
  EXPECT_EQ(0, ea->highestVersion());

  EXPECT_FLOAT_EQ(NO_OBS,  corrected(counter, st1));

  ea->newVersion();
  EXPECT_EQ(1, ea->currentVersion());
  EXPECT_EQ(1, ea->highestVersion());

  { Updater updater(ea, counter);
    ObsUpdate_p up = updater.createUpdate(st1);
    up->setCorrected(CHNG_C1);
    updater.send();
  }

  EXPECT_EQ(4, counter->size());
  EXPECT_EQ(0, counter->countUpdate);
  EXPECT_EQ(2, counter->countNew);

  EXPECT_FLOAT_EQ(CHNG_C1, corrected(counter, st1));

  ea->reset();
  EXPECT_EQ(0, ea->currentVersion());
  EXPECT_EQ(0, ea->highestVersion());
  EXPECT_EQ(0, counter->countUpdate);
  EXPECT_EQ(2, counter->countNew);
  EXPECT_EQ(1, counter->countDrop);

  EXPECT_FLOAT_EQ(NO_OBS, corrected(counter, st1));
}

TEST(EditAccessTest, UpdateInParent)
{
  SqliteAccess_p sqla(new SqliteAccess);
  sqla->insertDataFromFile(std::string(TEST_SOURCE_DIR)+"/data_18210_20130410.txt");
  CachingAccess_p ca(new CachingAccess(sqla));
  EditAccess_p ea(new EditAccess(ca));

  const Sensor sensor1(18210, 211, 0, 0, 514);
  const TimeSpan time(s2t("2013-04-01 00:00:00"), s2t("2013-04-01 02:00:00"));

  const Time t1 = s2t("2013-04-01 01:00:00"), t2 = s2t("2013-04-01 01:30:00");
  const SensorTime st1(sensor1, t1), st2(sensor1, t2);

  const float ORIG_C1 = -4.5f;
  const float CHNG_C1 =  4.0f, CHNG_C2 = 3.0f;

  // get data from EditAccess
  CountingBuffer_p counterE(new CountingBuffer(sensor1, time));
  counterE->postRequest(ea);
  EXPECT_EQ(3, counterE->size());
  EXPECT_EQ(1, counterE->countComplete);
  EXPECT_EQ(1, counterE->countNew);

  EXPECT_FLOAT_EQ(ORIG_C1, corrected(counterE, st1));
  EXPECT_FLOAT_EQ(NO_OBS,  corrected(counterE, st2));

  // get same data from CachingAccess
  CountingBuffer_p counterC(new CountingBuffer(sensor1, time));
  counterC->postRequest(ca);
  EXPECT_EQ(3, counterC->size());
  EXPECT_EQ(1, counterC->countComplete);

  EXPECT_FLOAT_EQ(ORIG_C1, corrected(counterC, st1));
  EXPECT_FLOAT_EQ(NO_OBS,  corrected(counterC, st2));

  // change data in CachingAccess
  ea->newVersion();
  EXPECT_EQ(1, ea->currentVersion());
  EXPECT_EQ(1, ea->highestVersion());

  { Updater updater(ca, counterC);
    ObsUpdate_p up1 = updater.createUpdate(st1);
    up1->setCorrected(CHNG_C1);
    ObsUpdate_p up2 = updater.createUpdate(st2);
    up2->setCorrected(CHNG_C2);
    updater.send();
  }

  EXPECT_EQ(4, counterE->size());
  EXPECT_EQ(1, counterE->countUpdate);
  EXPECT_EQ(2, counterE->countNew);

  EXPECT_FLOAT_EQ(CHNG_C1, corrected(counterE, st1));
  EXPECT_FLOAT_EQ(CHNG_C2, corrected(counterE, st2));
}

TEST(EditAccessTest, DropInParent)
{
  SqliteAccess_p sqla(new SqliteAccess);
  sqla->insertDataFromFile(std::string(TEST_SOURCE_DIR)+"/data_18210_20130410.txt");
  CachingAccess_p ca(new CachingAccess(sqla));
  EditAccess_p ea(new EditAccess(ca));

  const Sensor sensor1(18210, 211, 0, 0, 514);
  const TimeSpan time(s2t("2013-04-01 00:00:00"), s2t("2013-04-01 02:00:00"));

  const Time t1 = s2t("2013-04-01 01:00:00");
  const SensorTime st1(sensor1, t1);

  const float ORIG_C1 = -4.5f;

  // get data from EditAccess
  CountingBuffer_p counter(new CountingBuffer(sensor1, time));
  counter->postRequest(ea);
  EXPECT_EQ(3, counter->size());
  EXPECT_EQ(1, counter->countComplete);
  EXPECT_EQ(1, counter->countNew);

  EXPECT_FLOAT_EQ(ORIG_C1, corrected(counter, st1));

  // drop data in SqliteAccess
  sqla->dropData(SensorTime_v(1, st1));

  EXPECT_EQ(2, counter->size());
  EXPECT_EQ(1, counter->countDrop);

  EXPECT_FLOAT_EQ(NO_OBS, corrected(counter, st1));
}

TEST(EditAccessTest, DropInParentAfterChange)
{
  SqliteAccess_p sqla(new SqliteAccess);
  sqla->insertDataFromFile(std::string(TEST_SOURCE_DIR)+"/data_18210_20130410.txt");
  CachingAccess_p ca(new CachingAccess(sqla));
  EditAccess_p ea(new EditAccess(ca));

  const Sensor sensor1(18210, 211, 0, 0, 514);
  const TimeSpan time(s2t("2013-04-01 00:00:00"), s2t("2013-04-01 02:00:00"));

  const Time t1 = s2t("2013-04-01 01:00:00");
  const SensorTime st1(sensor1, t1);

  const float ORIG_C1 = -4.5f;
  const float CHNG_C1 =  4.0f;

  // get data from EditAccess
  CountingBuffer_p counter(new CountingBuffer(sensor1, time));
  counter->postRequest(ea);
  EXPECT_EQ(3, counter->size());
  EXPECT_EQ(1, counter->countComplete);
  EXPECT_EQ(1, counter->countNew);

  EXPECT_FLOAT_EQ(ORIG_C1, corrected(counter, st1));

  ea->newVersion();
  EXPECT_EQ(1, ea->currentVersion());
  EXPECT_EQ(1, ea->highestVersion());

  { Updater updater(ea, counter);
    ObsUpdate_p up = updater.createUpdate(st1);
    up->setCorrected(CHNG_C1);
    updater.send();
  }

  EXPECT_EQ(3, counter->size());
  EXPECT_EQ(1, counter->countUpdate);

  EXPECT_FLOAT_EQ(CHNG_C1, corrected(counter, st1));

  // drop data in SqliteAccess
  sqla->dropData(SensorTime_v(1, st1));

  EXPECT_EQ(3, counter->size());
  EXPECT_EQ(1, counter->countUpdate);
  EXPECT_EQ(0, counter->countDrop);
  EXPECT_FLOAT_EQ(CHNG_C1, corrected(counter, st1));

  ea->reset();

  EXPECT_EQ(2, counter->size());
  EXPECT_EQ(1, counter->countDrop);

  EXPECT_FLOAT_EQ(NO_OBS, corrected(counter, st1));
}

TEST(EditAccessTest, RequestAfterEdit)
{
  SqliteAccess_p sqla(new SqliteAccess);
  sqla->insertDataFromFile(std::string(TEST_SOURCE_DIR)+"/data_18210_20130410.txt");
  CachingAccess_p ca(new CachingAccess(sqla));
  EditAccess_p ea(new EditAccess(ca));

  const Sensor sensor1(18210, 211, 0, 0, 514);
  //const Sensor_s sensors = (SetMaker<Sensor_s>() << sensor1 << sensor2).set();
  const TimeSpan time(s2t("2013-04-01 00:00:00"), s2t("2013-04-01 06:00:00"));

  const Time t1 = s2t("2013-04-01 01:00:00"), t2 = s2t("2013-04-01 02:00:00");
  const SensorTime st1(sensor1, t1), st2(sensor1, t2);

  const float ORIG_C1 = -4.5f, ORIG_C2 = -5.0f;
  const float CHNG_C1 =  4.0f, CHNG_C2 =  3.0f;

  CountingBuffer_p counter(new CountingBuffer(sensor1, time));
  counter->postRequest(ea);
  EXPECT_EQ(7, counter->size());
  EXPECT_EQ(1, counter->countComplete);

  EXPECT_FLOAT_EQ(ORIG_C1, corrected(counter, st1));
  EXPECT_FLOAT_EQ(ORIG_C2, corrected(counter, st2));

  ea->newVersion();

  { Updater updater(ea, counter);
    ObsUpdate_p up = updater.createUpdate(st1);
    up->setCorrected(CHNG_C1);
    
    up = updater.createUpdate(st2);
    up->setCorrected(CHNG_C2);
    
    updater.send();
  }

  EXPECT_EQ(7, counter->size());
  EXPECT_EQ(1, counter->countUpdate);

  CountingBuffer_p counter2(new CountingBuffer(sensor1, time));
  counter2->postRequest(ea);
  EXPECT_EQ(7, counter2->size());
  EXPECT_EQ(1, counter2->countComplete);
  EXPECT_FLOAT_EQ(CHNG_C1, corrected(counter2, st1));
  EXPECT_FLOAT_EQ(CHNG_C2, corrected(counter2, st2));
}

TEST(EditAccessTest, History)
{
  SqliteAccess_p sqla(new SqliteAccess);
  sqla->insertDataFromFile(std::string(TEST_SOURCE_DIR)+"/data_18210_20130410.txt");
  CachingAccess_p ca(new CachingAccess(sqla));
  EditAccess_p ea(new EditAccess(ca));

  const Sensor sensor1(18210, 211, 0, 0, 514);
  //const Sensor_s sensors = (SetMaker<Sensor_s>() << sensor1 << sensor2).set();
  const TimeSpan time(s2t("2013-04-01 00:00:00"), s2t("2013-04-01 06:00:00"));

  const Time t1 = s2t("2013-04-01 01:00:00"), t2 = s2t("2013-04-01 02:00:00");
  const SensorTime st1(sensor1, t1), st2(sensor1, t2);
  const float CHNG_C1a = 4.0f, CHNG_C1b = 4.1f, CHNG_C2 =  3.0f;

  CountingBuffer_p counter(new CountingBuffer(sensor1, time));
  counter->postRequest(ea);

  Updater updater(ea, counter);

  ea->newVersion();
  updater.setCorrected(st1, CHNG_C1a);
  EXPECT_EQ(1, ea->currentVersion());
  EXPECT_EQ(1, ea->highestVersion());
  EXPECT_EQ(1, ea->countU());

  { const ObsData_pv changes1 = ea->versionChanges(1);
    ASSERT_EQ(1, changes1.size());
    EXPECT_TRUE(eq_SensorTime()(st1, changes1[0]->sensorTime()));
  }

  ea->newVersion();
  updater.setCorrected(st2, CHNG_C2);
  EXPECT_EQ(2, ea->currentVersion());
  EXPECT_EQ(2, ea->highestVersion());
  EXPECT_EQ(2, ea->countU());

  { const ObsData_pv changes1 = ea->versionChanges(1);
    ASSERT_EQ(1, changes1.size());
    EXPECT_TRUE(eq_SensorTime()(st1, changes1[0]->sensorTime()));

    const ObsData_pv changes2 = ea->versionChanges(2);
    ASSERT_EQ(1, changes2.size());
    EXPECT_TRUE(eq_SensorTime()(st2, changes2[0]->sensorTime()));
  }

  ea->undoVersion();
  EXPECT_EQ(1, ea->countU());
  ea->undoVersion();
  EXPECT_EQ(0, ea->countU());

  ea->newVersion();
  updater.setCorrected(st1, CHNG_C1b);
  EXPECT_EQ(1, ea->currentVersion());
  EXPECT_EQ(1, ea->highestVersion());
  EXPECT_EQ(1, ea->countU());
}

TEST(EditAccessTest, InsertData)
{
  const Sensor s0(45420, 110, 0, 0, 302);
  const timeutil::ptime t0 = s2t("2012-10-13 06:00:00");
  const SensorTime st0(s0, t0);
  const TimeSpan time0(s2t("2012-10-12 06:00:00"), s2t("2012-10-14 06:00:00"));

  SqliteAccess_p sqla(new SqliteAccess);
  sqla->insertDataFromText("45420	110	302	2012-10-12 06:00:00	-32767.0	2.0	0000001000007000	");
  //sqla->insertDataFromText("45420	110	302	2012-10-13 06:00:00	-32767.0	2.0	0000001000007000	");
  sqla->insertDataFromText("45420	110	302	2012-10-14 06:00:00	-32767.0	2.0	0000001000007000	");

  const float newC = 4.0;

  EditAccess_p ea = boost::make_shared<EditAccess>(sqla);

  CountingBuffer_p counter(new CountingBuffer(s0, time0));
  counter->syncRequest(ea);
  EXPECT_EQ(2, counter->size());
  EXPECT_EQ(1, counter->countComplete);
  counter->zero();
  EXPECT_EQ(0, ea->currentVersion());
  EXPECT_EQ(0, ea->highestVersion());

  ASSERT_FALSE(counter->get(st0));

  { ea->newVersion();
    ObsUpdate_pv updates;
    
    ObsUpdate_p up = ea->createUpdate(st0);
    ASSERT_TRUE((bool)up);
    
    up->setCorrected(newC);
    updates.push_back(up);
    ASSERT_TRUE(ea->storeUpdates(updates));
  }
    
  EXPECT_EQ(3, counter->size());
  EXPECT_EQ(1, counter->countNew);
  ASSERT_TRUE((bool)counter->get(st0));
  EXPECT_FLOAT_EQ(newC, corrected(counter, st0));
  EXPECT_EQ(1, ea->countU());

  ea->undoVersion();

  EXPECT_EQ(0, ea->currentVersion());
  EXPECT_EQ(1, ea->highestVersion());
  EXPECT_EQ(1, counter->countDrop);
  ASSERT_FALSE(counter->get(st0));
  EXPECT_EQ(0, ea->countU());

  ea->redoVersion();
  EXPECT_EQ(3, counter->size());
  EXPECT_EQ(2, counter->countNew);
  ASSERT_TRUE((bool)counter->get(st0));
  EXPECT_FLOAT_EQ(newC, corrected(counter, st0));
  EXPECT_EQ(1, ea->countU());
}
