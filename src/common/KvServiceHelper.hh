
#ifndef KvServiceHelper_hh
#define KvServiceHelper_hh 1

#include <QObject>

class KvServiceHelper : public QObject
{
  Q_OBJECT;

public:
  KvServiceHelper();
  ~KvServiceHelper();

  /** Query last known availability of kvServiced. Does not re-check. */
  bool isKvalobsAvailable() const
    { return mKvalobsAvailable; }

  /** Query last known availability of kvServiced. Does not re-check. */
  bool checkKvalobsAvailability();

  int identifyOperator(const QString& username);

  static KvServiceHelper* instance()
    { return sInstance; }

Q_SIGNALS:
  void kvalobsAvailable(bool available);

private:
  bool updateKvalobsAvailability(bool available);

private:
  bool mKvalobsAvailable;

  static KvServiceHelper* sInstance;
};

#endif // KvServiceHelper_hh
