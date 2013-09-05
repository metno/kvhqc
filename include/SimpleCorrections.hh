
#ifndef SimpleCorrections_hh
#define SimpleCorrections_hh 1

#include "EditAccess.hh"
#include "ModelAccess.hh"

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
                        
    void setDataAccess(EditAccessPtr eda, ModelAccessPtr mda);

public Q_SLOTS:
    void navigateTo(const SensorTime&);

private:
    void onDataChanged(ObsAccess::ObsDataChange, ObsDataPtr);
    void enableEditing();

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
    EditAccessPtr mDA;
    ModelAccessPtr mMA;
    SensorTime mSensorTime;
};

#endif // SimpleCorrections_hh
