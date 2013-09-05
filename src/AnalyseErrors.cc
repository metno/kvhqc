
#include "AnalyseErrors.hh"

#include "hqcdefs.h" // for struct currentType
#include "KvMetaDataBuffer.hh"

#include <kvalobs/kvDataOperations.h>

#include <boost/foreach.hpp>

//#define NDEBUG
#include "debug.hh"

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
    // this is from HqcMainWindow::checkTypeId
    const std::list<kvalobs::kvObsPgm>& obs_pgm = KvMetaDataBuffer::instance()->findObsPgm(stnr);
    std::vector<currentType> currentTypeList;
    BOOST_FOREACH(const kvalobs::kvObsPgm& op, obs_pgm) {
        currentType crT;
        crT.stnr    = stnr;
        crT.par     = op.paramID();
        crT.fDate   = timeutil::from_miTime(op.fromtime()).date();
        crT.tDate   = timeutil::from_miTime(op.totime()).date();;
        crT.cLevel  = op.level();
        crT.cSensor = 0;
        crT.cTypeId = op.typeID();
        currentTypeList.push_back(crT);
    }

    // this is from ErrorList::typeFilter
    const timeutil::pdate otime_date = otime.date();
    BOOST_FOREACH(const currentType& ct, currentTypeList) {
        if (stnr == ct.stnr && abs(typeId) == ct.cTypeId && par == ct.par && otime_date >= ct.fDate && otime_date <= ct.tDate )
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

bool isErrorInMemstore1(const ErrorList::mem& mo)
{
    const int fnum = mo.controlinfo.flag(kvalobs::flag::fnum), fw  = mo.controlinfo.flag(kvalobs::flag::fw);
    const int fs   = mo.controlinfo.flag(kvalobs::flag::fs),   fcp = mo.controlinfo.flag(kvalobs::flag::fcp);
    const int fr   = mo.controlinfo.flag(kvalobs::flag::fr),   fcc = mo.controlinfo.flag(kvalobs::flag::fcc);
    if (mo.flTyp == kvalobs::flag::fr) {
        if (KvMetaDataBuffer::instance()->isModelParam(mo.parNo)) {
            if ( fnum == 1 || (fnum > 1 && fw == 1) ) {
            } else if ( (fnum > 1 && fw > 1) || fw == 0 ) {
                return true;
            }
        } else {
            for( int k = 2; k < 16; k++ ) {
                //	  int iFlg = control.mid(k,1).toInt(0,16);
                int iFlg = mo.controlinfo.flag(k);
                if ( iFlg > 1 )
                    return true;
            }
        }
    } else if (mo.flTyp == kvalobs::flag::fs) {
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
    else if (mo.flTyp == kvalobs::flag::fnum) {
        if ( mo.parNo == 177 || mo.parNo == 178 ) {
            return true;
        }
    } else if (mo.flTyp == kvalobs::flag::fw) {
        if ( (fw == 2 || fw == 3) && ( fr > 1 || fcc > 1 || fs > 1 || fcp > 1) ) {
            return true;
        } else if ( (fw == 2 || fw == 3) && ( fr <= 1 && fcc <= 1 && fs <= 1 && fcp <= 1) ) {
        }
    } else {
    }
    return false;
}

void dumpMemstore(const std::vector<ErrorList::mem>& DBGE(memstore), const char* DBGE(label))
{
#ifndef NDEBUG
    LOG_SCOPE("AnalyseErrors");
    LOG4SCOPE_DEBUG("memory store " << label << " size = " << memstore.size());
    int i = -1;
    BOOST_FOREACH(const ErrorList::mem& mo, memstore) {
        LOG4SCOPE_DEBUG(std::setw(7) << (++i)
                        << std::setw(7) << mo.stnr << ' '
                        << std::setw(21) << mo.obstime
                        << std::setw(5) << mo.parNo
                        << std::setw(9) << mo.orig
                        << std::setw(9) << mo.corr
                        << std::setw(9) << mo.morig << "  "
                        << std::setw(5) << mo.flTyp << "  " << mo.flg << "  "
                        << mo.controlinfo.flagstring() << "  " << mo.cfailed);
    }
#endif
}

} // namespace anonymous

// ************************************************************************

namespace Errors {

std::vector<ErrorList::mem> fillMemoryStore2(const std::vector<int>& selectedParameters,
                                             const TimeRange& timerange,
                                             bool errorsForSalen,
                                             model::KvalobsDataListPtr dtl,
                                             const std::vector<modDatl>& mdtl)
{
    LOG_SCOPE("AnalyseErrors");
    std::vector<ErrorList::mem> memStore1, memStore2;
    BOOST_FOREACH(const model::KvalobsData& data, *dtl) {
        if( data.stnr() > 99999 )
            continue;
#if 0
#warning Is showTypeId correct here? (It was a bug before checking if a pointer was less than zero)
        if( data.showTypeId() < 0 )
            continue;
#endif
        if( data.otime() < timerange.t0() || data.otime() > timerange.t1() )
            continue;
        ErrorList::mem memObs;
        memObs.obstime = data.otime();
        memObs.tbtime = data.tbtime();
        memObs.name = data.name();
        memObs.stnr = data.stnr();

        BOOST_FOREACH(const int parameterID, selectedParameters) {
            // LOG4SCOPE_DEBUG(DBG1(memObs.obstime) << DBG1(memObs.stnr) << DBG1(parameterID));
            if (not ErrorSpecialTimeFilter(parameterID, data.otime()))
                continue;
            if (not IsTypeInObsPgm(data.stnr(), parameterID, data.typeId(parameterID), data.otime()))
                continue;
            if (not ErrorParameterFilter(parameterID))
                continue;

            memObs.controlinfo = data.controlinfo(parameterID);
            memObs.cfailed     = data.cfailed(parameterID);

            // from ErrorList::makeErrorList
            if (memObs.controlinfo.flag(kvalobs::flag::fr) == 6 && memObs.controlinfo.flag(kvalobs::flag::ftime) == 1)
                continue;

            memObs.flTyp = -1;
            memObs.flg = ErrorFilter(memObs.controlinfo,
                                     memObs.cfailed,
                                     memObs.flTyp);
            if (memObs.flg <= 1 or memObs.flTyp < 0)
                continue;

            memObs.typeId      = data.typeId(parameterID);
            memObs.orig        = data.orig(parameterID);
            memObs.corr        = data.corr(parameterID);
            memObs.sen         = data.sensor(parameterID);
            memObs.lev         = data.level(parameterID);
            memObs.useinfo     = data.useinfo(parameterID);
            memObs.parNo       = parameterID;

            memObs.morig = -32767.0;
            if (KvMetaDataBuffer::instance()->isModelParam(memObs.parNo)) {
                BOOST_FOREACH(const modDatl& md, mdtl) {
                    if( md.stnr == memObs.stnr && md.otime == memObs.obstime ) {
                        memObs.morig = md.orig[memObs.parNo];
                    }
                }
            }

            // insert data into appropriate memory stores
            if (not errorsForSalen) {
                if( ((memObs.flg == 2 || memObs.flg == 3) && memObs.flTyp == kvalobs::flag::fr ) ||
                    (memObs.flg == 2 && (memObs.flTyp == kvalobs::flag::fcc || memObs.flTyp == kvalobs::flag::fcp) ) ||
                    ((memObs.flg == 2 || memObs.flg == 3 ||memObs.flg == 4 || memObs.flg == 5) && memObs.flTyp == kvalobs::flag::fnum) ||
                    ((memObs.flg == 2 || memObs.flg == 4 || memObs.flg == 5 ) && memObs.flTyp == kvalobs::flag::fs ) )
                {
                    if( isErrorInMemstore1(memObs) ) {
                        memStore2.push_back(memObs);
                    } else {
                        memStore1.push_back(memObs);
                    }
                } else if (((memObs.flg == 4 || memObs.flg == 5 || memObs.flg == 6) && memObs.flTyp == kvalobs::flag::fr ) ||
                           ((memObs.flg == 3 || memObs.flg == 4 || memObs.flg == 6 || memObs.flg == 7 || memObs.flg == 9 ||
                             memObs.flg == 0xA || memObs.flg == 0xB || memObs.flg == 0xD ) && memObs.flTyp == kvalobs::flag::fcc ) ||
                           ((memObs.flg == 3 || memObs.flg == 4 || memObs.flg == 6 || memObs.flg == 7 ||
                             memObs.flg == 0xA || memObs.flg == 0xB ) && memObs.flTyp == kvalobs::flag::fcp ) ||
                           ((memObs.flg == 3 || memObs.flg == 6 || memObs.flg == 8 || memObs.flg == 9)&& memObs.flTyp == kvalobs::flag::fs ) ||
                           (memObs.flg == 6 && memObs.flTyp == kvalobs::flag::fnum) ||
                           (( memObs.flg == 3 || memObs.flg == 4 || memObs.flg == 6) && memObs.flTyp == kvalobs::flag::fpos) ||
                           ((memObs.flg == 2 || memObs.flg == 3) && memObs.flTyp == kvalobs::flag::ftime) ||
                           ((memObs.flg == 2 || memObs.flg == 3 || memObs.flg == 0xA) && memObs.flTyp == kvalobs::flag::fw) ||
                           (memObs.flg > 0 && memObs.flTyp == kvalobs::flag::fmis ) ||
                           (memObs.flg == 7 && memObs.flTyp == kvalobs::flag::fd) )
                {
                    memStore2.push_back(memObs);
                }
            } else {
                if( ((memObs.flg == 4 || memObs.flg == 5 || memObs.flg == 6) && memObs.flTyp == kvalobs::flag::fr )
                    || (memObs.flg == 2 && memObs.flTyp == kvalobs::flag::fs) )
                {
                    memStore2.push_back(memObs);
                }
            }
        }
    }

    dumpMemstore(memStore1, "1");
    dumpMemstore(memStore2, "2");

    return memStore2;
}

} // namespace Errors