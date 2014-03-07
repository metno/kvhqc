
#ifndef UTIL_BLOCKER_HH
#define UTIL_BLOCKER_HH 1

class Blocker {
public:
  Blocker(int& counter)
    : mCounter(counter)
    { mCounter += 1; }

  bool open() const
    { return mCounter <= 1; }

  ~Blocker()
    { mCounter -= 1; }

private:
  int& mCounter;
};

#endif // UTIL_BLOCKER_HH
