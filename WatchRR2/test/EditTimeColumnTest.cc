
#include "ColumnFactory.hh"
#include "EditTimeColumn.hh"
#include "FakeDataAccess.hh"
#include "Helpers.hh"
#include <boost/bind.hpp>

#define NDEBUG
#include "w2debug.hh"

struct CountColumnChanged : private boost::noncopyable
{
    int count;
    CountColumnChanged() : count(0) { }
    void operator()(const timeutil::ptime& /*time*/, ObsColumn* /*column*/)
        { count += 1; }
};

TEST(EditTimeColumnTest, UpdateSignal)
{
    FakeDataAccessPtr fda = boost::make_shared<FakeDataAccess>();
    fda->insertStation = 31850;
    fda->insertParam = 110;
    fda->insertType = 302;
    fda->insert("2012-11-20 06:00:00",  -32767.0,       2.8, "0000001000007000", "QC2-...");
    fda->insert("2012-11-21 06:00:00",       7.4,       4.6, "0000004000008000", "");
    fda->insert("2012-11-22 06:00:00",       3.7,       3.7, "0110000000004004", "watchRR");
    fda->insert("2012-11-23 06:00:00",       1.2,       1.2, "0110000000004004", "watchRR");
    fda->insert("2012-11-24 06:00:00",  -32767.0,       6.0, "0000001000009006", "watchRR,watchRR");
    fda->insert("2012-11-25 06:00:00",  -32767.0,      -1.0, "0000001000009006", "watchRR,watchRR");
    fda->insert("2012-11-26 06:00:00",       8.9,       2.9, "0110004000002006", "QC1-7-110,hqc");
    fda->insert("2012-11-27 06:00:00",       2.8,       2.8, "0110000000001000", "");

    const Sensor sensor(31850, kvalobs::PARAMID_RR, 0, 0, 302);
    EditAccessPtr eda = boost::make_shared<EditAccess>(fda);
    const TimeRange time(s2t("2012-11-20 06:00:00"), s2t("2012-11-27 06:00:00"));
    DataColumnPtr dc = ColumnFactory::columnForSensor(eda, sensor, time, ColumnFactory::NEW_CORRECTED);
    boost::shared_ptr<EditTimeColumn> etc = boost::make_shared<EditTimeColumn>(dc);
    etc->setEditableTime(TimeRange(s2t("2012-11-21 06:00:00"), s2t("2012-11-27 06:00:00")));

    CountColumnChanged cccD, cccET;
    boost::signals::scoped_connection scD  = dc ->columnChanged.connect(boost::ref(cccD ));
    boost::signals::scoped_connection scET = etc->columnChanged.connect(boost::ref(cccET));

    const timeutil::ptime t = s2t("2012-11-24 06:00:00");
    QVariant v = etc->data(t, Qt::DisplayRole);
    ASSERT_TRUE(v.isValid());
    ASSERT_EQ("6.0", v.toString().toStdString());

    EditDataPtr obs = eda->findE(SensorTime(sensor, t));
    ASSERT_TRUE(obs);

    eda->pushUpdate();
    eda->editor(obs)->setCorrected(5.5f).commit();
    ASSERT_EQ(1, cccD .count);
    ASSERT_EQ(1, cccET.count);

    v = etc->data(t, Qt::DisplayRole);
    ASSERT_TRUE(v.isValid());
    ASSERT_EQ("5.5", v.toString().toStdString());

    eda->popUpdate();
    ASSERT_EQ(2, cccD .count);
    ASSERT_EQ(2, cccET.count);

    v = etc->data(t, Qt::DisplayRole);
    ASSERT_TRUE(v.isValid());
    ASSERT_EQ("6.0", v.toString().toStdString());
}
