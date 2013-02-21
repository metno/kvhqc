
#ifndef HQC_LOGGING_HH
#define HQC_LOGGING_HH 1

#include <string>

class QString;
namespace log4cpp {
class CategoryStream;
}

class Log4CppConfig {
public:
    Log4CppConfig(const std::string& l4c_p);
    ~Log4CppConfig();
};

log4cpp::CategoryStream& operator<<(log4cpp::CategoryStream& out, const QString& qs);

#ifndef NO_LOG4CPP
#include <log4cpp/Category.hh>
#include <log4cpp/Priority.hh>

#define LOG4HQC(logger, level, message)                                 \
    do {                                                                \
        if (logger.isPriorityEnabled(level)) {                          \
            logger << level << __FILE__ << ':'                          \
                   << __LINE__ << '[' << __FUNCTION__ << "]"            \
                   << log4cpp::eol << "  " << message;                  \
        }                                                               \
    } while(false)

#else // NO_LOG4CPP
#define LOG4HQC(logger, level, message)         \
    do { /* nothing */ } while(false)
#endif // NO_LOG4CPP

#define LOGHQC_FATAL(logger, message)                  \
    LOG4HQC(logger, log4cpp::Priority::FATAL, message)
#define LOGHQC_ERROR(logger, message)                  \
    LOG4HQC(logger, log4cpp::Priority::ERROR, message)
#define LOGHQC_WARN(logger, message)                   \
    LOG4HQC(logger, log4cpp::Priority::WARN, message)
#define LOGHQC_INFO(logger, message)                   \
    LOG4HQC(logger, log4cpp::Priority::INFO, message)
#define LOGHQC_DEBUG(logger, message)                  \
    LOG4HQC(logger, log4cpp::Priority::DEBUG, message)

#define LOGHQC_CATEGORY(category, level, message)                          \
    LOG4HQC(log4cpp::Category::getInstance(category), level, message)
#define LOG4HQC_FATAL(category, message)                            \
    LOGHQC_CATEGORY(category, log4cpp::Priority::FATAL, message)
#define LOG4HQC_ERROR(category, message)                            \
    LOGHQC_CATEGORY(category, log4cpp::Priority::ERROR, message)
#define LOG4HQC_WARN(category, message)                             \
    LOGHQC_CATEGORY(category, log4cpp::Priority::WARN, message)
#define LOG4HQC_INFO(category, message)                             \
    LOGHQC_CATEGORY(category, log4cpp::Priority::INFO, message)
#define LOG4HQC_DEBUG(category, message)                            \
    LOGHQC_CATEGORY(category, log4cpp::Priority::DEBUG, message)

#endif // HQC_LOGGING_HH
