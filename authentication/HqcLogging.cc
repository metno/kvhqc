
#include "HqcLogging.hh"

#include "Sensor.hh"

#include <log4cpp/Category.hh>
#include <log4cpp/CategoryStream.hh>
#include <log4cpp/PropertyConfigurator.hh>
#include <log4cpp/OstreamAppender.hh>
#include <log4cpp/Layout.hh>
#include <log4cpp/PatternLayout.hh>

#include <QtCore/QString>

#include <boost/foreach.hpp>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

namespace /* anonymous */ {

class FlushingOstreamAppender : public log4cpp::OstreamAppender {
public:
    FlushingOstreamAppender(const std::string& name, std::ostream* stream)
        : OstreamAppender(name, stream) { }

protected:
    virtual void _append(const log4cpp::LoggingEvent& event)
        { log4cpp::OstreamAppender::_append(event); if (_stream) (*_stream) << std::flush; }

};

log4cpp::Appender* makeConsoleAppender()
{
    log4cpp::Appender *a = new FlushingOstreamAppender("console", &std::cout);
    log4cpp::PatternLayout* layout = new log4cpp::PatternLayout();
    layout->setConversionPattern("%d %p %c: %m%n");
    a->setLayout(layout);
    return a;
}

} // anonymous namespace

Log4CppConfig::Log4CppConfig(const std::string& l4c_p)
{
    struct stat sb;
    if (stat(l4c_p.c_str(), &sb) == 0) {
        log4cpp::PropertyConfigurator::configure(l4c_p);
        std::vector<log4cpp::Category*>* allCat = log4cpp::Category::getCurrentCategories();
        if (allCat) {
            BOOST_FOREACH(log4cpp::Category* cat, *allCat) {
                log4cpp::AppenderSet allApp = cat->getAllAppenders();
                BOOST_FOREACH(log4cpp::Appender* app, allApp) {
                    log4cpp::OstreamAppender* oApp = dynamic_cast<log4cpp::OstreamAppender*>(app);
                    if (!oApp)
                        continue;
                    cat->removeAppender(oApp);
                    cat->addAppender(makeConsoleAppender());
                }
            }
        }
    } else {
        log4cpp::Appender *a = makeConsoleAppender();

        log4cpp::Category& root = log4cpp::Category::getRoot();
        root.setPriority(log4cpp::Priority::DEBUG);
        root.addAppender(a);
    }
}

Log4CppConfig::~Log4CppConfig()
{
    log4cpp::Category::shutdown();
}

log4cpp::CategoryStream& operator<<(log4cpp::CategoryStream& out, const QString& qs)
{
    out << qs.toStdString();
    return out;
}

log4cpp::CategoryStream& operator<<(log4cpp::CategoryStream& out, const Sensor& s)
{
    out << "(s:" << s.stationId
        << ", p:" << s.paramId
        << ", l:" << s.level
        << ", s:" << s.sensor
        << ", t:" << s.typeId << ')';
    return out;
}

log4cpp::CategoryStream& operator<<(log4cpp::CategoryStream& out, const timeutil::ptime& t)
{
    out << timeutil::to_iso_extended_string(t);
    return out;
}

log4cpp::CategoryStream& operator<<(log4cpp::CategoryStream& out, const SensorTime& st)
{
    out << st.sensor << '@' << st.time;
    return out;
}
