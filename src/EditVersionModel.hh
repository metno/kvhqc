
#ifndef EditVersionModel_hh
#define EditVersionModel_hh 1

#include "EditAccess.hh"

class EditVersionModel {
public:
    EditVersionModel(EditAccessPtr eda);
    ~EditVersionModel();

private:
    void onCurrentVersionChanged(int current, int highest);
    void onDataChanged(ObsAccess::ObsDataChange what, ObsDataPtr obs);
    void dump();

private:
    EditAccessPtr mDA;
};

#endif // EditVersionModel_hh
