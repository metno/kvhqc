
#ifndef Tasks_hh
#define Tasks_hh 1

#include <QString>

namespace tasks {

enum { TASK_NONE,
       TASK_MISSING_OBS,
       TASK_HQC_BEFORE_REDIST,
       TASK_HQC_AUTOMATIC,
       TASK_NO_ACCUMULATION_DAYS,
       TASK_NO_ENDPOINT,
       TASK_MIXED_REDISTRIBUTION,
       TASK_MAYBE_ACCUMULATED,
       TASK_PREVIOUSLY_ACCUMULATION,
       TASK_FCC_ERROR,
       TASK_LAST
};

extern const int REQUIRED_TASK_MASK;

QString asText(int tasks);

} // namespace tasks

#endif
