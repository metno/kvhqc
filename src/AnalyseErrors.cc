
#include "AnalyseErrors.hh"

#include "hqcdefs.h" // for struct currentType
#include "KvMetaDataBuffer.hh"

#include <kvalobs/kvDataOperations.h>

#include <boost/foreach.hpp>

#define MILOGGER_CATEGORY "kvhqc.AnalyseErrors"
#include "HqcLogging.hh"

namespace /* anonymous */ {

bool ErrorSpecialTimeFilter(int paramId, const timeutil::ptime& otime)
{
    const int otime_hour = otime.time_of_day().hours();
    if ( ((paramId == 214 || paramId == 216) && !(otime_hour == 6 || otime_hour == 18))
         || (paramId == 112 && otime_hour != 6) )
    {
        return false;
    }
    return true;
}

bool IsTypeInObsPgm(int stnr, int par, int typeId, const timeutil::ptime& otime)
{
    const timeutil::pdate otime_date = otime.date();

    // this is from HqcMainWindow::checkTypeId combined with from ErrorList::typeFilter
    const std::list<kvalobs::kvObsPgm>& obs_pgm = KvMetaDataBuffer::instance()->findObsPgm(stnr);
    BOOST_FOREACH(const kvalobs::kvObsPgm& op, obs_pgm) {
        const timeutil::pdate tfrom = timeutil::from_miTime(op.fromtime()).date(),
            tto = timeutil::from_miTime(op.totime()).date();
        if (abs(typeId) == op.typeID() and par == op.paramID() && otime_date >= tfrom && otime_date <= tto )
            return true;
    }
    return false;
}

/*!
 * \return  0 if only check is in controlNoControl,
 *         -1 if first check in controlNoControl but more checks done,
 *          1 if not checked or first check not in controlNoControl
 */
int ErrorControlFilter(const std::string& cfailed)
{
    const char* controlNoControl[] = {"QC1-2-100","QC1-2-123A","QC1-2-123B","QC1-2-123C","QC1-2-124A",
                                      "QC1-2-125A","QC1-2-126B","QC1-2-129A","QC1-2-129B","QC1-2-130A",
                                      "QC1-2-131","QC1-2-132","QC1-2-133","QC1-2-134B","QC1-2-139A",
                                      "QC1-2-139B","QC1-2-142","QC1-2-143","QC1-2-144","QC1-2-145",
                                      "QC1-2-146A","QC1-2-146B","QC1-2-146C","QC1-2-146D","QC1-2-147_148A",
                                      "QC1-2-147_148B","QC1-2-147_148C","QC1-2-147_148D","QC1-2-149A",
                                      "QC1-2-149B","QC1-2-149C","QC1-2-149D","QC1-2-150A","QC1-2-150B",
                                      "QC1-2-150C","QC1-2-150D","QC1-2-151A","QC1-2-151B","QC1-2-151C",
                                      "QC1-2-151D","QC1-2-152","QC1-2-153","QC1-2-154","QC1-2-155",
                                      "QC1-2-156A","QC1-2-156B","QC1-2-156C","QC1-2-156D","QC1-2-158A",
                                      "QC1-2-158B","QC1-2-158C","QC1-2-158D","QC1-2-159A","QC1-2-159B",
                                      "QC1-2-159C","QC1-2-159D","QC1-2-160A","QC1-2-160B", "QC1-2-160C",
                                      "QC1-2-160D","QC1-2-161A","QC1-2-161B","QC1-2-161C","QC1-2-161D",
                                      "QC1-2-162A","QC1-2-162B","QC1-2-162C","QC1-2-162D","QC1-2-163A",
                                      "QC1-2-163B","QC1-2-163C","QC1-2-163D","QC1-2-164","QC1-2-165",
                                      "QC1-2-166","QC1-2-167","QC1-2-168","QC1-2-169","QC1-2-170",
                                      "QC1-2-171","QC1-2-172","QC1-2-173","QC1-2-175"};

    if (cfailed.empty())
        return 1;
    const size_t first_comma = cfailed.find(",");
    const std::string first_check = (first_comma == std::string::npos) ? cfailed : cfailed.substr(0, first_comma);
    
    const char** f = std::find(controlNoControl, boost::end(controlNoControl), first_check);
    if (f == boost::end(controlNoControl))
        return 1;
    if (first_comma != std::string::npos)
        return 0;
    else
        return -1;
}

/*!
 * \param flTyp will contain flag number if return value > 1
 * \return -1 or -2 for no further manual checks,
           "first flag > 1" otherwise, flTyp is flag number
 */
int ErrorFilter(const kvalobs::kvControlInfo& cInfo, const std::string& cfailed, int& flTyp)
{
    if (cInfo.flag(kvalobs::flag::fpre) > 0 or cInfo.flag(kvalobs::flag::fhqc) > 0)
        return -1;

    const int pcf = ErrorControlFilter(cfailed);
    if (pcf == 0)
        return -1;
    int maxflg = -1;
    if (pcf == -1)
        maxflg = -2;

    int flg = 0;
    for (int i = 0; i < 16; i++) {
        flg = cInfo.flag(i);
        if( flg > 1 && maxflg > -2 ) {
            maxflg = flg;
            flTyp = i;
            break;
        } else if (flg > 1)
            maxflg = -1;
    }
    return maxflg;
}

bool ErrorFilter(const kvalobs::kvControlInfo& controlinfo, const std::string& cfailed, int& flg, int& flTyp)
{
    // from ErrorList::makeErrorList
    if (controlinfo.flag(kvalobs::flag::fr) == 6 && controlinfo.flag(kvalobs::flag::ftime) == 1)
        return false;

    flTyp = -1;
    flg = ErrorFilter(controlinfo, cfailed, flTyp);
    return not (flg <= 1 or flTyp < 0);
}

bool isErrorInMemstore1(const int flTyp, const int paramId, const kvalobs::kvControlInfo& controlinfo)
{
    const int fnum = controlinfo.flag(kvalobs::flag::fnum), fw  = controlinfo.flag(kvalobs::flag::fw);
    const int fs   = controlinfo.flag(kvalobs::flag::fs),   fcp = controlinfo.flag(kvalobs::flag::fcp);
    const int fr   = controlinfo.flag(kvalobs::flag::fr),   fcc = controlinfo.flag(kvalobs::flag::fcc);
    if (flTyp == kvalobs::flag::fr) {
        if (KvMetaDataBuffer::instance()->isModelParam(paramId)) {
            if ( fnum == 1 || (fnum > 1 && fw == 1) ) {
            } else if ( (fnum > 1 && fw > 1) || fw == 0 ) {
                return true;
            }
        } else {
            for( int k = 2; k < 16; k++ ) {
                //	  int iFlg = control.mid(k,1).toInt(0,16);
                int iFlg = controlinfo.flag(k);
                if ( iFlg > 1 )
                    return true;
            }
        }
    } else if (flTyp == kvalobs::flag::fs) {
        if ( fs == 2 && fcp > 1 )
            return true;
        else if ( fs == 2 && fcp <= 1 ) {
            if ( fr == 1 && fw == 1 ) {
            } else if ( fr > 1 || fw > 1 ) {
                return true;
            }
        } else if ( fs == 4 && fcp > 1 ) {
            return true;
        } else if ( fs == 4 && fcp <= 1 && fr <= 1 && fw <= 1 ) {
        } else if ( fs == 5 && ( fr > 1 || fw > 1 ) ) {
            return true;
        } else if ( fs == 5 && fr <= 1 &&  fw <= 1  ) {
        }
    }
    //TODO: Proper treatment of fcc=2 and fcp=2
    /*
      else if ( mo.flTyp == "fcc" ) {
      if ( fcc == 2 )
      // find the other parameter
      error.push_back(i);
      }
      else if ( mo.flTyp == "fcp" ) {
      if ( fcp == 2 )
      // find the other parameter
      error.push_back(i);
      }
    */
    else if (flTyp == kvalobs::flag::fnum) {
        if (paramId == 177 || paramId == 178) {
            return true;
        }
    } else if (flTyp == kvalobs::flag::fw) {
        if ( (fw == 2 || fw == 3) && ( fr > 1 || fcc > 1 || fs > 1 || fcp > 1) ) {
            return true;
        } else if ( (fw == 2 || fw == 3) && ( fr <= 1 && fcc <= 1 && fs <= 1 && fcp <= 1) ) {
        }
    } else {
    }
    return false;
}


int whichMemoryStore(const int flg, const int flTyp, bool errorsForSalen)
{
    // insert data into appropriate memory stores
    if (not errorsForSalen) {
        if( ((flg == 2 || flg == 3) && flTyp == kvalobs::flag::fr ) ||
            (flg == 2 && (flTyp == kvalobs::flag::fcc || flTyp == kvalobs::flag::fcp) ) ||
            ((flg == 2 || flg == 3 ||flg == 4 || flg == 5) && flTyp == kvalobs::flag::fnum) ||
            ((flg == 2 || flg == 4 || flg == 5 ) && flTyp == kvalobs::flag::fs ) )
        {
            return 1;
        } else if (((flg == 4 || flg == 5 || flg == 6) && flTyp == kvalobs::flag::fr ) ||
                   ((flg == 3 || flg == 4 || flg == 6 || flg == 7 || flg == 9 ||
                     flg == 0xA || flg == 0xB || flg == 0xD ) && flTyp == kvalobs::flag::fcc ) ||
                   ((flg == 3 || flg == 4 || flg == 6 || flg == 7 ||
                     flg == 0xA || flg == 0xB ) && flTyp == kvalobs::flag::fcp ) ||
                   ((flg == 3 || flg == 6 || flg == 8 || flg == 9)&& flTyp == kvalobs::flag::fs ) ||
                   (flg == 6 && flTyp == kvalobs::flag::fnum) ||
                   (( flg == 3 || flg == 4 || flg == 6) && flTyp == kvalobs::flag::fpos) ||
                   ((flg == 2 || flg == 3) && flTyp == kvalobs::flag::ftime) ||
                   ((flg == 2 || flg == 3 || flg == 0xA) && flTyp == kvalobs::flag::fw) ||
                   (flg > 0 && flTyp == kvalobs::flag::fmis ) ||
                   (flg == 7 && flTyp == kvalobs::flag::fd) )
        {
            return 2;
        }
    } else {
        if( ((flg == 4 || flg == 5 || flg == 6) && flTyp == kvalobs::flag::fr )
            || (flg == 2 && flTyp == kvalobs::flag::fs) )
        {
            return 2;
        }
    }
    return 0;
}

bool filterForMemStore(EditDataPtr obs, int& flg, int& flTyp)
{
    flg = flTyp = -1;
    if (not obs)
        return false;

    const SensorTime& st = obs->sensorTime();
    const Sensor& s = st.sensor;

    if (not ErrorSpecialTimeFilter(s.paramId, st.time))
        return false;
    if (not IsTypeInObsPgm(s.stationId, s.paramId, s.typeId, st.time))
        return false;

    if (not ErrorFilter(obs->controlinfo(), obs->cfailed(), flg, flTyp))
        return false;

    return true;
}

int whichMemStore(EditDataPtr obs, bool errorsForSalen)
{
  int flg = -1, flTyp = -1;
  if (not filterForMemStore(obs, flg, flTyp))
    return 0;
  int memStore = whichMemoryStore(flg, flTyp, errorsForSalen);
  if (memStore == 1 && isErrorInMemstore1(flTyp, obs->sensorTime().sensor.paramId, obs->controlinfo()))
    memStore = 2;
  return memStore;
}

//#define DUMPOBS 1
#ifdef DUMPOBS
void dumpObs(const EditDataPtr obs, float model, int flTyp, int flg, int memStore)
{
    const SensorTime& st = obs->sensorTime();
    METLIBS_LOG_DEBUG("ms " << memStore << ": "
                  << std::setw(7)  << st.sensor.stationId << ' '
                  << std::setw(21) << st.time
                  << std::setw(5)  << st.sensor.paramId
                  << std::setw(9)  << obs->original()
                  << std::setw(9)  << obs->corrected()
                  << std::setw(9)  << model << "  "
                  << std::setw(5)  << flTyp << "  " << flg << "  "
                  << obs->controlinfo().flagstring() << "  " << obs->cfailed());
}
#endif

bool checkError2013(const EditDataPtr obs)
{
  const int fhqc = obs->controlinfo().flag(kvalobs::flag::fhqc);
  if (fhqc != 0)
    return false;

  const int ui_2 = Helpers::extract_ui2(obs);
  if (ui_2 == 2 or ui_2 == 3 or ui_2 == 9)
    return true;

  const int fr = obs->controlinfo().flag(kvalobs::flag::fr);
  if (fr == 2 or fr == 3)
    return true;

  const int fs = obs->controlinfo().flag(kvalobs::flag::fs);
  if (fs == 2)
    return true;

  const int fw = obs->controlinfo().flag(kvalobs::flag::fr);
  if (fw == 2) {
    const int fcc = obs->controlinfo().flag(kvalobs::flag::fcc);
    const int fcp = obs->controlinfo().flag(kvalobs::flag::fcp);
    if (fcc == 2 or fcp == 2)
      return true;
  }

  return false;
}

} // namespace anonymous

// ************************************************************************

namespace Errors {

bool recheck(ErrorInfo& ei, bool errorsForSalen)
{
    const bool oldBad = ei.badInList;
    if (whichMemStore(ei.obs, errorsForSalen) == 2)
      ei.badInList |= ErrorInfo::BAD_IN_ERRORLIST2012;
    if (checkError2013(ei.obs))
      ei.badInList |= ErrorInfo::BAD_IN_ERRORLIST2013;
    return (ei.badInList != oldBad);
}

Errors_t fillMemoryStore2(EditAccessPtr eda, const Sensors_t& sensors, const TimeRange& limits, bool errorsForSalen)
{
    // FIXME add timeFilter from ClockDialog
    METLIBS_LOG_SCOPE();

    Errors_t memStore2;
    const ObsAccess::DataSet allData = eda->allData(sensors, limits);
    BOOST_FOREACH(const ObsDataPtr& obs, allData) {
      EditDataPtr ebs = boost::static_pointer_cast<EditData>(obs);
      ErrorInfo ei(ebs);
      recheck(ei, errorsForSalen);
      if (ei.badInList != 0)
        memStore2.push_back(ei);
    }

    return memStore2;
}

} // namespace Errors
