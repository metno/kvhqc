
#ifndef OBSACCESS_OBSHELPERS_HH
#define OBSACCESS_OBSHELPERS_HH 1

#include "access/ObsData.hh"
#include "access/ObsUpdate.hh"
#include "FlagChange.hh"

namespace Helpers {

int is_accumulation(ObsData_p obs);
int is_accumulation(ObsUpdate_p update);
int is_endpoint(ObsData_p obs);
int is_endpoint(ObsUpdate_p update);

bool is_rejected(ObsData_p obs);
bool is_rejected(ObsUpdate_p update);
bool is_missing(ObsData_p obs);
bool is_missing(ObsUpdate_p update);
bool is_orig_missing(ObsData_p obs);
bool is_orig_missing(ObsUpdate_p update, ObsData_p obs);

void reject(ObsUpdate_p update);
void correct(ObsUpdate_p update, float newC);
void auto_correct(ObsUpdate_p update, ObsData_p obs, float newC);

void set_flag(ObsUpdate_p update, int flag, int value);
void set_fhqc(ObsUpdate_p update, int fhqc);

inline void changeControlinfo(ObsUpdate_p obs, const FlagChange& fc)
{ obs->setControlinfo(fc.apply(obs->controlinfo())); }

int extract_ui2(ObsData_p obs);

} // namespace Helpers

#endif // OBSACCESS_OBSHELPERS_HH
