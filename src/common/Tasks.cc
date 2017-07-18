
#include "Tasks.hh"

#include "util/stringutil.hh"

#include <QCoreApplication>

namespace /*anonymous*/ {
void append(QString& text, const QString& taskText)
{
  Helpers::appendText(text, taskText, "; ");
}
} // namespace anonymous

namespace tasks {

extern const int REQUIRED_TASK_MASK = (1<<tasks::TASK_PREVIOUSLY_ACCUMULATION);


QString asText(int tasks)
{
  QString text;
  if (tasks & (1<<TASK_MISSING_OBS))
    append(text, qApp->translate("Tasks", "missing observation"));
  if (tasks & (1<<TASK_HQC_BEFORE_REDIST))
    append(text, qApp->translate("Tasks", "HQC flags set before redistribution"));
  if (tasks & (1<<TASK_HQC_AUTOMATIC))
    append(text, qApp->translate("Tasks", "HQC flags set for automatic redistribution"));
  if (tasks & (1<<TASK_NO_ACCUMULATION_DAYS))
    append(text, qApp->translate("Tasks", "endpoint without any days of accumulation"));
  if (tasks & (1<<TASK_NO_ENDPOINT))
    append(text, qApp->translate("Tasks", "no endpoint for accumulation found"));
  if (tasks & (1<<TASK_MIXED_REDISTRIBUTION))
    append(text, qApp->translate("Tasks", "redistribution partially not done or done by QC2 or HQC"));
  if (tasks & (1<<TASK_MAYBE_ACCUMULATED))
    append(text, qApp->translate("Tasks", "unusual observation, might be an accumulated value"));
  if (tasks & (1<<TASK_PREVIOUSLY_ACCUMULATION))
    append(text, qApp->translate("Tasks", "previously part of an accumulation"));
  if (tasks & (1<<TASK_FCC_ERROR))
    append(text, qApp->translate("Tasks", "consistency error"));
  return text;
};

} // namespace tasks
