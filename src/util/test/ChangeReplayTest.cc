
#include "ChangeReplay.hh"

#include <gtest/gtest.h>

namespace /* anonymous */ {

struct Col {
  int station, param;
  Col(int s, int p) : station(s), param(p) { }
};

bool operator<(const Col& a, const Col& b)
{
  if (a.station != b.station)
    return a.station < b.station;
  return a.param < b.param;
}

bool operator==(const Col& a, const Col& b)
{
  return (a.station == b.station)
      and (a.param == b.param);
}

std::ostream& operator<<(std::ostream& out, const Col& c)
{
  return out << "[s=" << c.station << ",p=" << c.param << "]";
}

typedef std::vector<Col> Columns;
Columns& operator<<(Columns& cs, const Col& c)
{ cs.push_back(c); return cs; }

} // namespace anonymous

TEST(ChangeReplayTest, ChangedOrig)
{
  ChangeReplay<Col> cr;

  Columns original;
  original << Col(18210, 211)
           << Col(18210, 213)
           << Col(18210, 215)
           << Col(18700, 211)
           << Col(18020, 211);

  Columns actual;
  actual << Col(18210, 211)
         << Col(18700, 211)
         << Col(17980, 211)
         << Col(18210, 215);

  Columns expect_removed;
  expect_removed << Col(18020, 211)
                 << Col(18210, 213);
  const Columns removed = cr.removals(original, actual);
  EXPECT_EQ(expect_removed, removed);

  Columns original2;
  original2 << Col(18210, 211)
            << Col(18210, 215)
            << Col(18700, 211)
            << Col(18900, 211)
            << Col(17020, 211);

  Columns expect_replayed;
  expect_replayed << Col(18210, 211)
                  << Col(18700, 211)
                  << Col(17980, 211)
                  << Col(18210, 215)
                  << Col(18900, 211)
                  << Col(17020, 211);
  const Columns replayed = cr.replay(original2, actual, removed);
  EXPECT_EQ(expect_replayed, replayed);
}
