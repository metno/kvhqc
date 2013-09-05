
#include "ColumnFactory.hh"
#include "Helpers.hh"
#include "KvalobsAccess.hh"
#include <boost/bind.hpp>

#define LOAD_DECL_ONLY
#include "load_31850_20121130.cc"

#define NDEBUG
#include "w2debug.hh"

TEST(VxColumnTest, Basic)
{
    FakeKvApp fa;
    load_31850_20121130(fa);

    const Sensor sensor1(31850, kvalobs::PARAMID_V6,   0, 0, 302);
    const Sensor sensor2(31850, kvalobs::PARAMID_V6+1, 0, 0, 302);
    EditAccessPtr eda = boost::make_shared<EditAccess>(fa.kda);
    DataColumnPtr dc = ColumnFactory::columnForSensor(eda, sensor1, t_31850_20121130(), ColumnFactory::NEW_CORRECTED);
    const boost::posix_time::time_duration offset = boost::posix_time::hours(-18);
    dc->setTimeOffset(offset);

    //boost::signals::scoped_connection sc = eda->obsDataChanged.connect(boost::bind(&DataColumn::onDataChanged, dc.get(), _1, _2));

    const timeutil::ptime t = s2t("2012-11-07 06:00:00");
    const SensorTime st1(sensor1, t + offset), st2(sensor2, t + offset);

    EditDataPtr obs1 = eda->findE(st1);
    EditDataPtr obs2 = eda->findE(st2);
    ASSERT_FALSE(obs1);
    ASSERT_FALSE(obs2);

    QVariant v = dc->data(t, Qt::DisplayRole);
    ASSERT_FALSE(v.isValid());

    ASSERT_TRUE(dc->setData(t, "SL2", Qt::EditRole));

    obs1 = eda->findE(st1);
    obs2 = eda->findE(st2);
    ASSERT_TRUE(obs1);
    ASSERT_TRUE(obs2);
    ASSERT_EQ(1, obs1->corrected());
    ASSERT_EQ(2, obs2->corrected());

    v = dc->data(t, Qt::DisplayRole);
    ASSERT_TRUE(v.isValid());
    const QString expect = QString("SL") + QChar( 0xB2 );
    ASSERT_EQ(expect.toStdString(), v.toString().toStdString());

    eda->undoVersion();

    v = dc->data(t, Qt::DisplayRole);
    ASSERT_FALSE(v.isValid());
}
