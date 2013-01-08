
#ifndef QtKvalobsAccess_hh
#define QtKvalobsAccess_hh 1

#include "KvalobsAccess.hh"

class QtKvalobsAccess : public QObject, public KvalobsAccess
{ Q_OBJECT;
public:
    QtKvalobsAccess();
    ~QtKvalobsAccess();

private Q_SLOTS:
    void onKvData(kvservice::KvObsDataListPtr data);

private:
    std::string mKvServiceSubsription;
};

#endif // KvalobsAccess_hh
