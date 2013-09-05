
#ifndef SimpleCorrections_hh
#define SimpleCorrections_hh 1

#include "EditAccess.hh"

#include <QtGui/QWidget>
#include <memory>

namespace Ui {
class SimpleCorrections;
}

class SimpleCorrections : public QWidget
{ Q_OBJECT
public:
    SimpleCorrections(QWidget* parent=0);
    ~SimpleCorrections();
                        
    void setDataAccess(EditAccessPtr eda);

public Q_SLOTS:
    void navigateTo(const SensorTime&);

private:
    void onDataChanged(ObsAccess::ObsDataChange, ObsDataPtr);

private:
    std::auto_ptr<Ui::SimpleCorrections> ui;
    EditAccessPtr mDA;
    SensorTime mSensorTime;
};

#endif // SimpleCorrections_hh
