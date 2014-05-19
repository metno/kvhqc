
#include "priority_list.hh"

#include <gtest/gtest.h>

TEST(PriorityListTest, Basic)
{
  priority_list<int> pl;
  EXPECT_TRUE(pl.empty());

  pl.push(2);
  EXPECT_FALSE(pl.empty());
  EXPECT_EQ(2, pl.top());

  pl.push(1);
  EXPECT_EQ(2, pl.top());

  pl.push(3);
  EXPECT_EQ(3, pl.top());

  pl.pop();
  EXPECT_EQ(2, pl.top());

  pl.pop();
  EXPECT_EQ(1, pl.top());

  pl.pop();
  EXPECT_TRUE(pl.empty());
}

namespace /*anonymous*/ {
struct task {
  int priority;
  int value;
  bool operator==(const task& other) const
    { return priority == other.priority and value == other.value; }
  task(int p, int v) : priority(p), value(v) { }
};

struct task_by_priority {
  bool operator()(const task& a, const task& b) const
    { return a.priority < b.priority; }
};

} // anonymous namespace

TEST(PriorityListTest, Drop)
{
  priority_list<task, task_by_priority> pl;

  pl.push(task(2,1));
  pl.push(task(1,1));
  pl.push(task(3,2));
  pl.push(task(3,1));

  EXPECT_FALSE(pl.drop(task(0,0)));

  EXPECT_FALSE(pl.drop(task(1,0)));
  EXPECT_TRUE(pl.drop(task(1,1)));
  EXPECT_FALSE(pl.drop(task(1,1)));

  EXPECT_EQ(task(3,2), pl.top());
  pl.pop();
  EXPECT_EQ(task(3,1), pl.top());
  pl.pop();
  EXPECT_EQ(task(2,1), pl.top());
  pl.pop();
  EXPECT_TRUE(pl.empty());
}
