
#include "ColumnFactory.hh"
#include "DataListModel.hh"
#include "KvHelpers.hh"

#include "FakeKvApp.hh"

#define MILOGGER_CATEGORY "kvhqc.test.DataListModelTest"
#include "util/HqcLogging.hh"

#define LOAD_DECL_ONLY
#include "load_18210_20130410.cc"

namespace /* anonymous */ {

std::shared_ptr<DataListModel> makeDLM(FakeKvApp& fa, const TimeSpan& time)
{
  load_18210_20130410(fa);
  const Sensor sensor(18210, kvalobs::PARAMID_TA, 0, 0, 514);
  EditAccess_p eda = std::make_shared<EditAccess>(fa.kda);

  std::shared_ptr<DataListModel> dlm(new DataListModel(eda, time));
  dlm->addColumn(ColumnFactory::columnForSensor(eda, sensor, time, ObsColumn::NEW_CORRECTED));
  return dlm;
}

} // namespace anonymous

TEST(DataListModelTest, RowCountRounded)
{
  FakeKvApp fa;
  const TimeSpan time(s2t("2013-04-02 01:00:00"), s2t("2013-04-03 01:00:00"));
  std::shared_ptr<DataListModel> dlm = makeDLM(fa, time);
  dlm->setFilterByTimestep(false);

  EXPECT_EQ(25, dlm->rowCount(QModelIndex()));

  // time range is from 01:00 to 01:00, so the rounded times are from
  // 2013-04-02T00:00 to 2013-04-03T02:00, 14 2-hour steps
  dlm->setTimeStep(2*60*60);
  EXPECT_EQ(14, dlm->rowCount(QModelIndex()));

  // time range is from 01:00 to 01:00, so the rounded times are from
  // 2013-04-01T06:00 to 2013-04-03T06:00, 3 days
  dlm->setTimeStep(24*60*60);
  EXPECT_EQ(3, dlm->rowCount(QModelIndex()));
}

TEST(DataListModelTest, RowCount6)
{
  FakeKvApp fa;
  const TimeSpan time(s2t("2013-04-02 06:00:00"), s2t("2013-04-03 06:00:00"));
  std::shared_ptr<DataListModel> dlm = makeDLM(fa, time);
  dlm->setFilterByTimestep(false);

  EXPECT_EQ(25, dlm->rowCount(QModelIndex()));

  // no rounding
  dlm->setTimeStep(2*60*60);
  EXPECT_EQ(13, dlm->rowCount(QModelIndex()));

  // no rounding
  dlm->setTimeStep(24*60*60);
  EXPECT_EQ(2, dlm->rowCount(QModelIndex()));
}

TEST(DataListModelTest, RowCountFiltered)
{
  FakeKvApp fa;
  const TimeSpan time(s2t("2013-04-09 18:00:00"), s2t("2013-04-09 23:00:00"));
  std::shared_ptr<DataListModel> dlm = makeDLM(fa, time);

  EXPECT_EQ(4, dlm->rowCount(QModelIndex()));

  dlm->setTimeStep(60*60);
  EXPECT_EQ(4, dlm->rowCount(QModelIndex()));

  dlm->setFilterByTimestep(false);
  EXPECT_EQ(6, dlm->rowCount(QModelIndex()));
}
