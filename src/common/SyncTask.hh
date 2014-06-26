
#ifndef ACCESS_SYNCTASK_HH
#define ACCESS_SYNCTASK_HH 1

#include "QueryTaskHandler.hh"

QueryTask* syncTask(QueryTask* task, QueryTaskHandler* handler);

inline QueryTask* syncTask(QueryTask* task, QueryTaskHandler_p handler)
{ return syncTask(task, handler.get()); }

#endif // ACCESS_SYNCTASK_HH
