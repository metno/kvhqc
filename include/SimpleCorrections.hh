
#ifndef SimpleCorrections_hh
#define SimpleCorrections_hh 1

#include "DataView.hh"

#include <QtGui/QWidget>
#include <memory>

namespace Ui {
class SimpleCorrections;
}

class SimpleCorrections : public QWidget, public DataView
{   Q_OBJECT;
public:
    SimpleCorrections(QWidget* parent=0);
    ~SimpleCorrections();
                        
    virtual void setDataAccess(EditAccessPtr eda, ModelAccessPtr mda);
    virtual void navigateTo(const SensorTime&);

protected:
    virtual void onDataChanged(ObsAccess::ObsDataChange, ObsDataPtr);

private:
    void enableEditing();
    void update();

private Q_SLOTS:
    void onAcceptOriginal();
    void onAcceptOriginalQC2();
    void onAcceptCorrected();
    void onAcceptCorrectedQC2();
    void onReject();
    void onRejectQC2();

    void onInterpolated();
    void onCorrected();

private:
    std::auto_ptr<Ui::SimpleCorrections> ui;
    SensorTime mSensorTime;
};

#endif // SimpleCorrections_hh
