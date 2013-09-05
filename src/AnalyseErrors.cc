
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

/*! \return true if the parameter should be checked */
bool ErrorParameterFilter(int parNo)
{
    // from ErrorList::priorityParameterFilter
    const int  parNoControl[] = {
        2,    3,  4,  5,  6,  9, 10, 11, 12, 13,
        17,  20, 21, 22, 23, 24, 25, 26, 27, 28,
        44,  45, 46, 47, 48, 49, 50, 51, 52, 53,
        54,  55, 56, 57,101,102,103,115,116,124,
        138,191,192,193,194,195,196,197,198,199,
        202,226,227,229,230,231,232,233,234,235,
        236,237,238,239,240,241,247,261,271,272,
        274,275,276,277,278,279,280,281,282,283,
        284,285,286,287,288,289,290,291,292,293,
        294,295,296,297,298,299,300,301,302,303,
        304,305,306,307,308
    };

    if (std::binary_search(parNoControl, boost::end(parNoControl), parNo))
        return false;
    return (parNo < 1000);
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

    // IMPORTANT: if this should be removed from fillMemoryStore2, it must also be removed here
    if (not ErrorParameterFilter(s.paramId))
        return false;

    if (not ErrorSpecialTimeFilter(s.paramId, st.time))
        return false;
    if (not IsTypeInObsPgm(s.stationId, s.paramId, s.typeId, st.time))
        return false;

    if (not ErrorFilter(obs->controlinfo(), obs->cfailed(), flg, flTyp))
        return false;

    return true;
}

int whichMemStore(EditDataPtr obs, bool errorsForSalen, int& flg, int& flTyp)
{
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

} // namespace anonymous

// ************************************************************************

namespace Errors {

bool recheck(ErrorInfo& ei, bool errorsForSalen)
{
    const int oldFlg = ei.flg, oldTyp = ei.flTyp;
    const bool oldFixed = ei.fixed;
    ei.fixed = (whichMemStore(ei.obs, errorsForSalen, ei.flg, ei.flTyp) != 2);
    return (ei.flg != oldFlg or ei.flTyp != oldTyp or ei.fixed != oldFixed);
}

Errors_t fillMemoryStore2(EditAccessPtr eda, const Sensors_t& sensors, const TimeRange& limits, bool errorsForSalen)
{
    // FIXME add timeFilter from ClockDialog
    METLIBS_LOG_SCOPE();

    Errors_t memStore2;
    BOOST_FOREACH(const Sensor& s, sensors) {
        METLIBS_LOG_DEBUG(LOGVAL(s));
        // IMPORTANT: if this should be removed from isMemoryStore2, it must also be removed here
        if (not ErrorParameterFilter(s.paramId))
            continue;

        const ObsAccess::DataSet allData = eda->allData(s, limits);
#ifndef NDEBUG
        METLIBS_LOG_DEBUG(LOGVAL(allData.size()));
        BOOST_FOREACH(const ObsDataPtr& obs, allData)
            METLIBS_LOG_DEBUG(obs->sensorTime());
#endif
        BOOST_FOREACH(const ObsDataPtr& obs, allData) {
          EditDataPtr ebs = boost::static_pointer_cast<EditData>(obs);
          ErrorInfo ei(ebs);
          int memstore = whichMemStore(ei.obs, errorsForSalen, ei.flg, ei.flTyp);
          if (memstore == 0)
            continue;
          if (memstore == 2)
            memStore2.push_back(ei);
#ifdef DUMPOBS
          dumpObs(ei.obs, -99999, ei.flTyp, ei.flg, memstore);
#endif
        }
    }

    return memStore2;
}

} // namespace Errors
