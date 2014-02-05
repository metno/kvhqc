
#include "AbstractUpdateListener.hh"

static AbstractUpdateListener* ul;

AbstractUpdateListener* updateListener()
{
  return ul;
}

void setUpdateListener(AbstractUpdateListener* u)
{
  ul = u;
}
