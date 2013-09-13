
#include "AnalyseFCC.hh"
#include "AnalyseRR24.hh"
#include "FlagChange.hh"
#include "EditAccess.hh"

#include "FakeKvApp.hh"
#include "CountDataChanged.hh"
#include "TestHelpers.hh"

#define LOAD_DECL_ONLY
#include "load_31850_20121130.cc"
#include "load_44160_20121207.cc"
#include "load_54420_20121130.cc"

TEST(EditAccessTest, SendToParent)
{
    FakeKvApp fa;
    const TimeRange time(s2t("2012-11-15 06:00:00"), s2t("2012-11-30 06:00:00"));
    load_31850_20121130(fa);

    EditAccessPtr eda = boost::make_shared<EditAccess>(fa.kda);

    const FlagChange fc_ok("fhqc=1");
    const Sensor s(31850, 18, 0, 0, 302);

    CountDataChanged cdcF, cdcE;
    cdcF.filterParam = cdcE.filterParam = s.paramId;
    fa.kda->obsDataChanged.connect(boost::ref(cdcF));
    eda->obsDataChanged.connect(boost::ref(cdcE));

    eda->newVersion();
    EditDataPtr ebs1 = eda->findE(SensorTime(s, s2t("2012-11-09 06:00:00")));
    ASSERT_TRUE(ebs1);
    eda->editor(ebs1)->setCorrected(4).changeControlinfo(fc_ok).commit();

    EditDataPtr ebs2 = eda->findE(SensorTime(s, s2t("2012-11-10 06:00:00")));
    ASSERT_TRUE(ebs2);
    eda->editor(ebs2)->setCorrected(5).changeControlinfo(fc_ok).commit();

    EditDataPtr ebs3 = eda->findE(SensorTime(s, s2t("2012-11-11 06:00:00")));
    ASSERT_TRUE(ebs3);
    eda->editor(ebs3)->setCorrected(3).changeControlinfo(fc_ok).commit();

    EXPECT_EQ(0, cdcF.count);
    EXPECT_EQ(3, cdcE.count);
    EXPECT_TRUE(ebs1->modified());
    EXPECT_TRUE(ebs2->modified());
    EXPECT_TRUE(ebs3->modified());

    EXPECT_TRUE(eda->sendChangesToParent());

    EXPECT_EQ(3, cdcF.count);
    EXPECT_EQ(6, cdcE.count); // EditData are no longer modified, therefore obsDataChanged is emitted
    EXPECT_FALSE(ebs1->modified());
    EXPECT_FALSE(ebs2->modified());
    EXPECT_FALSE(ebs3->modified());

    EXPECT_CORR_CONTROL(4, "0000000000000001", ebs1);
    EXPECT_CORR_CONTROL(5, "0000000000000001", ebs2);
    EXPECT_CORR_CONTROL(3, "0000000000000001", ebs3);
}

TEST(EditAccessTest, Reset)
{
    FakeKvApp fa;
    const TimeRange time(s2t("2012-11-15 06:00:00"), s2t("2012-11-30 06:00:00"));
    load_31850_20121130(fa);

    EditAccessPtr eda = boost::make_shared<EditAccess>(fa.kda);

    const FlagChange fc_ok("fhqc=1");
    const Sensor s(31850, 18, 0, 0, 302);

    CountDataChanged cdcF, cdcE;
    cdcF.filterParam = cdcE.filterParam = s.paramId;
    fa.kda->obsDataChanged.connect(boost::ref(cdcF));
    eda->obsDataChanged.connect(boost::ref(cdcE));

    eda->newVersion();

    EditDataPtr ebs1 = eda->findE(SensorTime(s, s2t("2012-11-09 06:00:00")));
    ASSERT_TRUE(ebs1);
    eda->editor(ebs1)->setCorrected(4).changeControlinfo(fc_ok).commit();

    EditDataPtr ebs2 = eda->findE(SensorTime(s, s2t("2012-11-10 06:00:00")));
    ASSERT_TRUE(ebs2);
    eda->editor(ebs2)->setCorrected(5).changeControlinfo(fc_ok).commit();

    EXPECT_EQ(0, cdcF.count);
    EXPECT_EQ(2, cdcE.count);

    eda->reset();

    EXPECT_EQ(0, cdcF.count);
    EXPECT_EQ(4, cdcE.count);

    EXPECT_CORR_CONTROL(2, "0000000000000000", ebs1);
    EXPECT_CORR_CONTROL(1, "0000000000000000", ebs2);
}

TEST(EditAccessTest, ResetCreate)
{
    FakeKvApp fa;
    const TimeRange time(s2t("2012-11-15 06:00:00"), s2t("2012-11-30 06:00:00"));
    load_31850_20121130(fa);

    EditAccessPtr eda = boost::make_shared<EditAccess>(fa.kda);

    const FlagChange fc_ok("fhqc=1");
    const Sensor s(31850, 18, 0, 0, 302);

    CountDataChanged cdcF, cdcE, cdcEdestroy;
    cdcF.filterParam = cdcE.filterParam = cdcEdestroy.filterParam = s.paramId;
    cdcEdestroy.filterWhat = ObsAccess::DESTROYED;
    fa.kda->obsDataChanged.connect(boost::ref(cdcF));
    eda->obsDataChanged.connect(boost::ref(cdcE));
    eda->obsDataChanged.connect(boost::ref(cdcEdestroy));

    eda->newVersion();

    EditDataPtr ebs1 = eda->findE(SensorTime(s, s2t("2012-11-09 06:00:00")));
    ASSERT_TRUE(ebs1);
    eda->editor(ebs1)->setCorrected(4).changeControlinfo(fc_ok).commit();

    EditDataPtr ebs2 = eda->createE(SensorTime(s, s2t("2012-11-09 18:00:00")));
    ASSERT_TRUE(ebs2);
    eda->editor(ebs2)->setCorrected(5).changeControlinfo(fc_ok).commit();

    EXPECT_EQ(0, cdcF.count);
    EXPECT_EQ(2, cdcE.count);

    eda->reset();

    EXPECT_EQ(0, cdcF.count);
    EXPECT_EQ(3, cdcE.count);
    EXPECT_EQ(1, cdcEdestroy.count);

    EXPECT_CORR_CONTROL(2, "0000000000000000", ebs1);
    // next would fail because KvBufferedAccess is not told to forget the object it created
    // EXPECT_FALSE(eda->findE(SensorTime(s, s2t("2012-11-09 18:00:00"))));
}

TEST(EditAccessTest, OnlyTasks)
{
    const Sensor sensor(44160, 110, 0, 0, 302);
    const TimeRange time(t_44160_20121207());
    FakeKvApp fa;
    load_44160_20121207(fa);
    EditAccessPtr eda = boost::make_shared<EditAccess>(fa.kda);

    CountDataChanged cdcF, cdcE;
    fa.kda->obsDataChanged.connect(boost::ref(cdcF));
    eda->obsDataChanged.connect(boost::ref(cdcE));

    eda->newVersion();
    FCC::analyse(eda, sensor, time);

    ASSERT_EQ(0, cdcF.count);
    ASSERT_EQ(2, cdcE.count);

    eda->sendChangesToParent();

    ASSERT_EQ(0, cdcF.count); // analyse only sets tasks, which are not kept in fda
    ASSERT_EQ(2, cdcE.count);
}

TEST(EditAccessTest, PopChange)
{
    const Sensor sensor(54420, 110, 0, 0, 302);
    const TimeRange time(s2t("2012-10-01 06:00:00"), s2t("2012-11-20 06:00:00"));
    FakeKvApp fa;
    load_54420_20121130(fa);
    EditAccessPtr eda = boost::make_shared<EditAccess>(fa.kda);

    CountDataChanged cdcF, cdcE, cdcE2;
    fa.kda->obsDataChanged.connect(boost::ref(cdcF));
    eda->obsDataChanged.connect(boost::ref(cdcE));

    eda->newVersion();
    TimeRange editableTime(time);
    RR24::analyse(eda, sensor, editableTime);

    ASSERT_EQ( 0, cdcF.count);
    ASSERT_EQ(33, cdcE.count);
    cdcE.count = 0;
    
    const TimeRange timeAcc(s2t("2012-10-05 06:00:00"), s2t("2012-10-11 06:00:00"));
    const std::vector<float> newCorrected(timeAcc.days()+1, 123.4);

    EditAccessPtr eda2 = boost::make_shared<EditAccess>(eda);
    eda2->obsDataChanged.connect(boost::ref(cdcE2));
    RR24::redistribute(eda2, sensor, timeAcc.t0(), editableTime, newCorrected);

    ASSERT_EQ(7, cdcE2.count);
    ASSERT_EQ(0, cdcE .count);

    eda->newVersion();
    eda2->sendChangesToParent();
    ASSERT_EQ(7, cdcE.count);
    cdcE.count = 0;

    eda->undoVersion();
    ASSERT_EQ(7, cdcE.count); // pop 7 days with changes

    eda->sendChangesToParent();
    ASSERT_EQ(0, cdcF.count); // only tasks from RR24::analyse remain, but fda does not keep tasks
}

TEST(EditAccessTest, UndoRedoNewVersions)
{
    using boost::gregorian::days;

    const Sensor sensor(32780, 211, 0, 0, 302);
    const TimeRange time(s2t("2012-12-01 06:00:00"), s2t("2012-12-06 06:00:00"));
    FakeKvApp fa;
    fa.insertStation = sensor.stationId;
    fa.insertParam   = sensor.paramId;
    fa.insertType    = sensor.typeId;
    fa.insertData("2012-12-01 06:00:00", 6.0, 6.0, "0110000000001000", "");
    fa.insertData("2012-12-02 06:00:00", 1.0, 1.0, "0110000000001000", "");
    fa.insertData("2012-12-03 06:00:00", 1.0, 1.0, "0110000000001000", "");
    fa.insertData("2012-12-04 06:00:00", 1.0, 1.0, "0110000000001000", "");
    fa.insertData("2012-12-05 06:00:00", 4.0, 4.0, "0110000000001000", "");
    fa.insertData("2012-12-06 06:00:00", 8.0, 8.0, "0110000000001000", "");

    const TimeRange timeR(s2t("2012-12-02 06:00:00"), s2t("2012-12-05 06:00:00"));
    fa.kda->addSubscription(ObsSubscription(sensor.stationId, time));
    EditAccessPtr eda = boost::make_shared<EditAccess>(fa.kda);

    EditDataPtr obs1 = eda->findE(SensorTime(sensor, s2t("2012-12-03 06:00:00")));
    EditDataPtr obs2 = eda->findE(SensorTime(sensor, s2t("2012-12-04 06:00:00")));
    EditDataPtr obs3 = eda->findE(SensorTime(sensor, s2t("2012-12-05 06:00:00")));
    ASSERT_TRUE(obs1);
    ASSERT_TRUE(obs2);
    ASSERT_TRUE(obs3);

    eda->newVersion();
    eda->editor(obs1)->setCorrected(2);
    EXPECT_TRUE(obs1->hasVersion(1));
    EXPECT_EQ(2, obs1->corrected());

    eda->newVersion();
    eda->editor(obs1)->setCorrected(7);
    EXPECT_TRUE(obs1->hasVersion(1));
    EXPECT_TRUE(obs1->hasVersion(2));
    EXPECT_EQ(2, obs1->corrected(1));
    EXPECT_EQ(7, obs1->corrected());
    eda->editor(obs2)->setCorrected(3);
    EXPECT_TRUE(obs2->hasVersion(2));
    EXPECT_EQ(3, obs2->corrected());

    eda->undoVersion();
    EXPECT_EQ(2, obs1->corrected());
    EXPECT_EQ(1, obs2->corrected());
    EXPECT_TRUE(eda->canRedo());
    EXPECT_TRUE(eda->canUndo());

    eda->redoVersion();
    EXPECT_EQ(7, obs1->corrected());
    EXPECT_EQ(3, obs2->corrected());
    EXPECT_FALSE(eda->canRedo());
    EXPECT_TRUE(eda->canUndo());

    eda->undoVersion();
    EXPECT_EQ(1, eda->currentVersion());
    eda->newVersion();
    EXPECT_EQ(2, eda->currentVersion());
    EXPECT_EQ(2, obs1->corrected());
    EXPECT_EQ(1, obs2->corrected());
    EXPECT_FALSE(eda->canRedo());
    EXPECT_TRUE(eda->canUndo());

    const std::vector<EditDataPtr> changed1 = eda->versionChanges(1);
    ASSERT_EQ(1, changed1.size());
    EXPECT_EQ(2, changed1.at(0)->corrected(1));

    const std::vector<EditDataPtr> changed2 = eda->versionChanges(2);
    ASSERT_TRUE(changed2.empty());
}

TEST(EditAccessTest, ChangeInParent)
{
    FakeKvApp fa;

    const Sensor s(31850, 18, 0, 0, 302);
    const timeutil::ptime t0 = s2t("2012-11-09 06:00:00");
    const SensorTime st(s, t0);

    const float OCORR = 2, NCORR = 4;

    fa.kda->addSubscription(ObsSubscription(s.stationId, TimeRange(t0, t0)));
    fa.insertData(s.stationId, s.paramId, s.typeId, "2012-11-09 06:00:00", 2, OCORR);

    EditAccessPtr parent = boost::make_shared<EditAccess>(fa.kda);
    EditAccessPtr child  = boost::make_shared<EditAccess>(parent);

    const FlagChange fc_ok("fhqc=1");

    CountDataChanged cdcP, cdcC;
    cdcP.filterParam = cdcC.filterParam = s.paramId;
    parent->obsDataChanged.connect(boost::ref(cdcP));
    child ->obsDataChanged.connect(boost::ref(cdcC));

    parent->newVersion();
    EditDataPtr ebsP = parent->findE(st);
    ASSERT_TRUE(ebsP);
    EXPECT_EQ(OCORR, ebsP->corrected());

    EditDataPtr ebsC = child ->findE(st);
    ASSERT_TRUE(ebsC);
    EXPECT_EQ(OCORR, ebsC->corrected());
  
    parent->editor(ebsP)->setCorrected(NCORR).changeControlinfo(fc_ok).commit();

    EXPECT_EQ(1, cdcP.count);
    EXPECT_EQ(1, cdcC.count);
    EXPECT_EQ(NCORR, ebsP->corrected());
    EXPECT_EQ(NCORR, ebsC->corrected());
}
