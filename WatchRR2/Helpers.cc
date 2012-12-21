
#include "Helpers.hh"

#include "EditAccess.hh"
#include "EditData.hh"
#include "EditDataEditor.hh"
#include "FlagChange.hh"
#include "timeutil.hh"
#include <kvalobs/kvDataOperations.h>
#include <kvalobs/kvModelData.h>

namespace {
// from WatchRR/src/ControlFlagCell.cc
static const char* flagnames[16] = {
    "fqclevel", "fr", "fcc", "fs", "fnum", 
    "fpos", "fmis", "ftime", "fw", "fstat", 
    "fcp", "fclim", "fd", "fpre", "", "fhqc" 
};
}

namespace Helpers {

bool float_eq::operator()(float a, float b) const
{
    return std::fabs(a - b) < 0.01f;
}

int kvSensorNumber(const kvalobs::kvData& d)
{
    const int s = d.sensor();
    return (s>='0') ? (s-'0') : s;
}

Sensor sensorFromKvData(const kvalobs::kvData& d)
{
    return Sensor(d.stationID(), d.paramID(), d.level(), kvSensorNumber(d), d.typeID());
}

SensorTime sensorTimeFromKvData(const kvalobs::kvData& d)
{
    return SensorTime(sensorFromKvData(d), timeutil::from_miTime(d.obstime()));
}

Sensor sensorFromKvModelData(const kvalobs::kvModelData& d)
{
    return Sensor(d.stationID(), d.paramID(), d.level(), 0, 0);
}

SensorTime sensorTimeFromKvModelData(const kvalobs::kvModelData& d)
{
    return SensorTime(sensorFromKvModelData(d), timeutil::from_miTime(d.obstime()));
}

Sensor modelSensor(const Sensor& s)
{
    return Sensor(s.stationId, s.paramId, s.level, 0, 0);
}

int is_accumulation(ObsDataPtr obs)
{
    using namespace kvalobs::flag;
    const int f_fd = obs->controlinfo().flag(fd);
    if( f_fd == 2 or f_fd == 4 )
        return BEFORE_REDIST;
    if( f_fd == 7 or f_fd == 8 )
        return QC2_REDIST;
    if( f_fd == 9 or f_fd == 0xA )
        return HQC_REDIST;
    return NO;
}
int is_endpoint(ObsDataPtr obs)
{
    using namespace kvalobs::flag;
    const int f_fd = obs->controlinfo().flag(fd);
    if( f_fd == 4 )
        return BEFORE_REDIST;
    if( f_fd == 8 )
        return QC2_REDIST;
    if( f_fd == 0xA )
        return HQC_REDIST;
    return NO;
}

bool is_rejected(ObsDataPtr obs)
{
    return (obs->controlinfo().flag(kvalobs::flag::fmis) == 2) // same as kvDataOperations.cc
        or (obs->corrected() == kvalobs::REJECTED);
}

bool is_missing(ObsDataPtr obs)
{
    return (obs->controlinfo().flag(kvalobs::flag::fmis) == 3) // same as kvDataOperations.cc
        or (obs->corrected() == kvalobs::MISSING);
}

bool is_orig_missing(ObsDataPtr obs)
{
    return (obs->controlinfo().flag(kvalobs::flag::fmis) & 1) // same as kvDataOperations.cc
        or (obs->original() == kvalobs::MISSING);
}

char int2char(int i)
{
    if( i<10 )
        return ('0' + i);
    else
        return ('A' + (i-10));
}

bool is_valid(ObsDataPtr obs) // same as kvDataOperations.cc
{
    return not is_missing(obs) and not is_rejected(obs);
}

void reject(EditDataEditorPtr editor) // same as kvDataOperations.cc
{
    if (not is_valid(editor->obs()))
        return;
    
    const FlagChange fc_reject("fmis=[04]->fmis=2;fmis=1->fmis=3;fhqc=A");
    editor->changeControlinfo(fc_reject);
    if (is_orig_missing(editor->obs()))
        editor->setCorrected(kvalobs::MISSING);
    else
        editor->setCorrected(kvalobs::REJECTED);
}

void correct(EditDataEditorPtr editor, float newC) // same as kvDataOperations.cc, except that it sets fmis=0
{
    const FlagChange fc_diff("fmis=3->fmis=1;fmis=[02]->fmis=4;fhqc=7");
    editor->changeControlinfo(fc_diff);
    editor->setCorrected(newC);
}

static const char* flagExplanations[16][16] = {
    { // fagg
        "Kvalitetsinformasjon er ikke gitt",
        "Grunnlagsdata er funnet i orden",
        "Grunnlagsdata er litt mistenkelig",
        "Grunnlagsdata er svært mistenkelig",
        "Grunnlagsdata er manuelt korrigert",
        "Grunnlagsdata er manuelt interpolert",
        "Grunnlagsdata er automatisk korrigert",
        "Grunnlagsdata er automatisk interpolert",
        "Grunnlagsdata er manuelt tilfordelt fra akkumulert verdi",
        "Grunnlagsdata er automatisk tilfordelt fra akkumulert verdi"
        "",
        "Grunnlagsdata er forkastet" },
    { // frange
 	"Ikke kontrollert",
        "Kontrollert. Funnet i orden",
        "Kontrollert. Observert verdi større enn høy testverdi",
        "Kontrollert. Observert verdi mindre enn lav testverdi",
        "Kontrollert. Observert verdi større enn høyeste testverdi",
        "Kontrollert. Observert verdi mindre enn minste testverdi",
        "Forkastet. Observert verdi utenfor fysikalske grenser",
        "Kontrollert. Funnet å svare til spesialverdi som betyr manglende",
        0,
        0,
        "Kontrollert. Observert verdi utenfor fysikalske grenser. Korrigert automatisk" },
    { // fcc
        "Ikke kontrollert.",
        "Kontrollert. Funnet i orden.",
        "Kontrollert. Formell inkonsistens, men neppe feil i aktuell parameter.",
        "Kontrollert. Formell inkonsistens ved observasjonsterminen, men ikke mulig å avgjøre i hvilken parameter.",
        "Kontrollert. Formell inkonsistens i forhold til tidligere/senere observasjonstermin, men ikke mulig å avgjøre i hvilken parameter.",
        0,
        "Kontrollert. Formell inkonsistens ved observasjonsterminen, antagelig feil i aktuell parameter.",
        "Kontrollert. Formell inkonsistens i forhold til tidligere/senere observasjonstermin, antagelig feil i aktuell parameter.",
        "Kontrollert. Formell inkonsistens ved observasjonsterminen. En av parametrene mangler.",
        "Original verdi mangler. Interpolert fra andre parametere.",
        "Kontrollert. Formell inkonsistens ved observasjonsterminen. Korrigert automatisk.",
        "Kontrollert. Formell inkonsistens i forhold til tidligere/senere observasjonstermin. Korrigert automatisk.",
        0,
        "Forkastet. Formell inkonsistens." },
    { // fs
        "Ikke kontrollert",
        "Kontrollert. Funnet i orden",
        "Kontrollert. Observert endring høyere enn (minste) testverdi",
        "Kontrollert. Ingen endring i måleverdi over x tidsskritt",
        "Kontrollert. Mistanke om feil detektert i QC1-3, mistanken opphevet av dipptest i QC2d-1",
        0,
        0,
        "Kontrollert. Observert drift i instrumentet",
        "Observert endring høyere enn høyeste testverdi. Forkastet",
        "Observert endring høyere enn testverdi. Korrigert automatisk",
        "Ingen endring i måleverdi over x tidsskritt. Korrigert automatisk" },
    { // fnum
        "Ikke kontrollert",
        "Kontrollert. Funnet i orden",
        "Kontrollert. Observert verdis avvik fra modellverdien større enn høy testverdi",
        "Kontrollert. Observert verdis avvik fra modellverdien mindre enn lav testverdi",
        "Kontrollert. Observert verdis avvik fra modellverdien større enn høyeste testverdi",
        "Kontrollert. Observert verdis avvik fra modellverdien mindre enn minste testverdi",
        "Original verdi mangler eller er forkastet. Interpolert/korrigert med modellverdi." },
    { // fpos
        "Ikke kontrollert",
        "Kontrollert funnet i orden",
        0,
        "Kontrollert. Mistenkelig melding. Ingen korreksjon.",
        "Kontrollert. Mistenkelig melding. Korrigert automatisk.",
        0,
        "Meldingsverdi forkastet." },
    { //  fmis
        "Original verdi eksisterer, det er ikke grunnlag for å si at den er sikkert feilaktig (corrected=original)",
        "Original verdi mangler. Korrigert verdi eksisterer",
        "Korrigert verdi mangler. Original verdi eksisterer (original verdi er forkastet)",
        "Original verdi og korrigert verdi mangler",
        "Original og korrigert verdi eksisterer, men original er sikkert feilaktig" },
    { // ftime
        "Ikke kontrollert",
        "Interpolert/korrigert med godt resultat",
        "Interpolert/korrigert med usikkert resultat",
        "Forsøkt interpolert/korrigert. Metode uegnet." },
    { // fw
        "Ikke kontrollert",
        "Kontrollert. Funnet i orden",
        "Kontrollert. Observert verdis avvik fra beregnet verdi er større enn høy testverdi",
        "Kontrollert. Observert verdis avvik fra beregnet verdi er mindre enn lav testverdi",
        "Kontrollert. Observert verdis avvik fra beregnet verdi er større enn høyeste testverdi",
        "Kontrollert. Observert verdis avvik fra beregnet verdi er mindre enn laveste testverdi",
        "Original verdi mangler eller er forkastet. Interpolert/korrigert med beregnet verdi" },
    { // fstat
        "Ikke kontrollert",
        "Kontrollert. Funnet i orden",
        "Kontrollert. Mistenkelig verdi. Ingen korreksjon." },
    { // fcp
        "Ikke kontrollert",
        "Kontrollert. Funnet i orden",
        "Kontrollert. Klimatologisk tvilsom, men neppe feil i aktuell parameter",
        "Kontrollert. Klimatologisk tvilsom ved observasjonsterminen, men ikke mulig å avgjøre i hvilken parameter",
        "Kontrollert. Klimatologisk tvilsom i forhold til tidligere/senere observasjonstermin, men ikke mulig å avgjøre i hvilken parameter",
        0,
        "Kontrollert. Klimatologisk tvilsom ved observasjonsterminen, antagelig feil i aktuell parameter",
        "Kontrollert. Klimatologisk tvilsom i forhold til tidligere/senere observasjonstermin, antagelig feil i aktuell parameter",
        0,
        0,
        "Kontrollert. Klimatologisk inkonsistens ved observasjonsterminen. Korrigert automatisk.",
        "Kontrollert. Klimatologisk inkonsistens i forhold til tidligere/senere observasjonstermin. Korrigert automatisk." },
    { // fclim
        "Ikke kontrollert",
        "Kontrollert. Funnet i orden",
        "Kontrollert. Mistenkelig verdi. Ikke korrigert",
        "Kontrollert. Mistenkelig verdi. Korrigert automatisk" },
    { // fd
        "Ikke vurdert som samleverdi",
        "Normal observasjon, ikke oppsamlet verdi",
        "Oppsamling pågår",
        "Unormal observasjon. Original verdi kan være oppsamlet verdi",
        "Oppsamlet verdi før tilfordeling",
        0,
        0,
        "Korrigert verdi er et resultat av automatisk tilfordeling. Original verdi mangler eller er forkastet",
        "Original verdi er oppsamling, korrigert verdi er tilfordelt automatisk",
        "Korrigert verdi er et resultat av manuell tilfordeling. Original verdi mangler eller er forkastet",
        "Original verdi er oppsamling, korrigert verdi er tilfordelt manuellt" },
    { // fpre
        "Ikke vurdert",
        "Vurdert. Ikke forkastet",
        0,
        0,
        "Korrigert automatisk",
        0,
        "Forkastet. Original verdi er kjent å være feil",
        "Forkastet. Parameter utelatt fra ny innsendt melding" },
    { // fcombi
        "Ikke tolket",
        "Tolket. Funnet iorden.",
        "Utenfor høyeste grenseverdi, ikke påvist sprang, innenfor tolerert avvik fra modellverdi",
        0,
        0,
        0,
        0,
        0,
        0,
        "Forkastet. Utenfor høyeste grenseverdi, ikke påvist sprang, utenfor tolerert avvik fra modellverdi",
        "Forkastet. Utenfor høyeste grenseverdi, påvist sprang, innenfor tolerert avvik fra modellverdi",
        "Forkastet. Utenfor høyeste grenseverdi, påvist sprang, utenfor tolerert avvik fra modellverdi" },
    { // fhqc
        "Ikke kontrollert i HQC",
        "Kontrollert i HQC. Funnet i orden.",
        "Ikke kvalifisert for feilliste. Sannsynligvis i orden.",
        "Ikke ferdig HQC kontrollert. Mulig kvalifisert for feilliste. Ikke aktuell for kontroll i QC1 eller QC2.",
        "Ikke ferdig HQC kontrollert. Mulig kvalifisert for feilliste. Aktuell for interpolering i QC2.",
        "Interpolert manuelt",
        "Tilfordelt manuelt",
        "Korrigert manuelt",
        0,
        0,
        "Forkastet manuelt" }
};

static QString formatFlag(const kvalobs::kvControlInfo & cInfo, bool explain)
{
    std::ostringstream ss;
    bool first = true;
    for(int f=1; f<16; f++) {
	const int flag = cInfo.flag(f);
        using namespace kvalobs::flag;
	if( (f != fmis and f != ftime and flag > 1) or ((f == fmis or f == ftime) and flag > 0) ) {
            if( not first )
                ss << (explain ? '\n' : ' ');
            ss << flagnames[f] << '=' << int2char(flag);
            if( explain ) {
                const char* e = flagExplanations[f][flag];
                if (not e)
                    e = "Ugyldig flagg-verdi";
                ss << ": " << e;
            }
            first = false;
	}
    }
    return QString::fromStdString(ss.str());
}


QString getFlagText(const kvalobs::kvControlInfo & cInfo)
{
    return formatFlag(cInfo, false);
}

QString getFlagExplanation(const kvalobs::kvControlInfo & cInfo)
{
    return formatFlag(cInfo, true);
}

QString parameterName(int paramId)
{
    using namespace kvalobs;

    switch(paramId) {
    case PARAMID_RR: return "RR_24";
    case PARAMID_V4: return "V4";
    case PARAMID_V5: return "V5";
    case PARAMID_V6: return "V6";
    case PARAMID_SA: return "SA";
    case PARAMID_SD: return "SD";
    default:
        return QString("{%1}").arg(paramId);
    }
}

void updateUseInfo(kvalobs::kvData& data)
{
    kvalobs::kvUseInfo ui = data.useinfo();
    ui.setUseFlags( data.controlinfo() );
    data.useinfo( ui );
}

void updateCfailed(kvalobs::kvData& data, const std::string& add)
{
    std::string new_cfailed = data.cfailed();
    if( new_cfailed.length() > 0 )
        new_cfailed += ",";
    new_cfailed += add;
    data.cfailed(new_cfailed);
}

QString appendText(QString& text, const QString& append, const QString& separator)
{
    if (append.isEmpty())
        return text;
    if (not text.isEmpty())
        text += separator;
    text += append;
    return text;
}

QString appendedText(const QString& text, const QString& append, const QString& separator)
{
    QString t(text);
    return appendText(t, append, separator);
}

} // namespace Helpers
