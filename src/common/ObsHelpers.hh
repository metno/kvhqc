
#ifndef OBSACCESS_OBSHELPERS_HH
#define OBSACCESS_OBSHELPERS_HH 1

#include "EditDataEditor.hh"
#include "ObsData.hh"

namespace Helpers {

int is_accumulation(ObsDataPtr obs);
int is_accumulation(EditDataEditorPtr editor);
int is_endpoint(ObsDataPtr obs);
int is_endpoint(EditDataEditorPtr editor);

bool is_rejected(ObsDataPtr obs);
bool is_rejected(EditDataEditorPtr editor);
bool is_missing(ObsDataPtr obs);
bool is_missing(EditDataEditorPtr editor);
bool is_orig_missing(ObsDataPtr obs);
bool is_orig_missing(EditDataEditorPtr editor);

void reject(EditDataEditorPtr editor);
void correct(EditDataEditorPtr editor, float newC);
void auto_correct(EditDataEditorPtr editor, float newC);

void set_flag(EditDataEditorPtr editor, int flag, int value);
void set_fhqc(EditDataEditorPtr editor, int fhqc);

int extract_ui2(ObsDataPtr obs);

} // namespace Helpers

#endif // OBSACCESS_OBSHELPERS_HH
