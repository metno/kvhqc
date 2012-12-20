
#include "VxColumn.hh"
#include "Helpers.hh"
#include <boost/bind.hpp>

#define LOAD_DECL_ONLY
#include "load_31850_20121130.cc"

#define NDEBUG
#include "w2debug.hh"

TEST(VxColumnTest, Basic)
{
    FakeDataAccessPtr fda = boost::make_shared<FakeDataAccess>();
    load_31850_20121130(fda);

    const Sensor sensor1(31850, kvalobs::PARAMID_V6,   0, 0, 302);
    const Sensor sensor2(31850, kvalobs::PARAMID_V6+1, 0, 0, 302);
    EditAccessPtr eda = boost::make_shared<EditAccess>(fda);
    DataColumnPtr dc = boost::make_shared<VxColumn>(eda, sensor1, DataColumn::NEW_CORRECTED);
    dc->setTimeOffset(boost::posix_time::hours(-18));

    //boost::signals::scoped_connection sc = eda->obsDataChanged.connect(boost::bind(&DataColumn::onDataChanged, dc.get(), _1, _2));

    const timeutil::ptime t = s2t("2012-11-07 06:00:00");

    EditDataPtr obs1 = eda->findE(SensorTime(sensor1, t + dc->timeOffset()));
    EditDataPtr obs2 = eda->findE(SensorTime(sensor2, t + dc->timeOffset()));
    ASSERT_FALSE(obs1);
    ASSERT_FALSE(obs2);

    QVariant v = dc->data(t, Qt::DisplayRole);
    ASSERT_FALSE(v.isValid());

    ASSERT_TRUE(dc->setData(t, "SL2", Qt::EditRole));

    obs1 = eda->findE(SensorTime(sensor1, t + dc->timeOffset()));
    obs2 = eda->findE(SensorTime(sensor2, t + dc->timeOffset()));
    ASSERT_TRUE(obs1);
    ASSERT_TRUE(obs2);

    v = dc->data(t, Qt::DisplayRole);
    ASSERT_TRUE(v.isValid());
    const QString expect = QString("SL") + QChar( 0xB2 );
    ASSERT_EQ(expect, v.toString());

    eda->popUpdate();

    v = dc->data(t, Qt::DisplayRole);
    ASSERT_FALSE(v.isValid());
}
