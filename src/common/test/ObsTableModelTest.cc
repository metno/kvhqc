
#include "ObsTableModel.hh"
#include "KvHelpers.hh"

#include "FakeKvApp.hh"
#include "TestHelpers.hh"

#define MILOGGER_CATEGORY "kvhqc.test.ObsTableModelTest"
#include "util/HqcLogging.hh"

TEST(ObsTableModelTest, RowCount)
{
  FakeKvApp fa;
  const TimeRange time(s2t("2013-04-02 00:00:00"), s2t("2013-04-03 00:00:00"));
  EditAccessPtr eda = std::make_shared<EditAccess>(fa.kda);
  std::shared_ptr<ObsTableModel> otm(new ObsTableModel(eda, time));
  EXPECT_EQ(3, otm->rowCount(QModelIndex()));
  otm->setTimeStep(1*60*60);
  EXPECT_EQ(25, otm->rowCount(QModelIndex()));
}
