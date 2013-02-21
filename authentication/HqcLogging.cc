
#include "HqcLogging.hh"

#include <log4cpp/Category.hh>
#include <log4cpp/CategoryStream.hh>
#include <log4cpp/PropertyConfigurator.hh>
#include <log4cpp/OstreamAppender.hh>
#include <log4cpp/Layout.hh>
#include <log4cpp/PatternLayout.hh>

#include <QtCore/QString>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

Log4CppConfig::Log4CppConfig(const std::string& l4c_p)
{
    struct stat sb;
    if (stat(l4c_p.c_str(), &sb) == 0) {
        log4cpp::PropertyConfigurator::configure(l4c_p);
    } else {
        log4cpp::Appender *a = new log4cpp::OstreamAppender("console", &std::cout);
        log4cpp::PatternLayout* layout = new log4cpp::PatternLayout();
        layout->setConversionPattern("%d %p %c: %m%n");
        a->setLayout(layout);

        log4cpp::Category& root = log4cpp::Category::getRoot();
        root.setPriority(log4cpp::Priority::WARN);
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
