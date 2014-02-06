
#include "PlotOptions.h"

namespace POptions {

const char T_HOUR    [] = "HOUR";
const char T_DAY     [] = "DAY";
const char T_DAYNUM  [] = "DAYNUM";
const char T_DATE    [] = "DATE";
const char T_WEEK    [] = "WEEK";
const char T_MONTH   [] = "MONTH";
const char T_MONTHNUM[] = "MONTHNUM";
const char T_YEAR    [] = "YEAR";

static const Colour BLACK(0,0,0);
static const Colour WHITE(255,255,255);

PlotOptions::PlotOptions()
    : drawgrid(true)
    , gridTimes(1)
    , language("EN")
    , plottype(type_line)
    , textcolour(BLACK)
    , linecolour(BLACK)
    , fillcolour(WHITE)
    , bordercolour(BLACK)
    , timecolour(BLACK)
    , linewidth(1)
    , axis(axis_left_left)
    , axisFixedMin(false)
    , axisMin(0)
    , axisFixedMax(false)
    , axisMax(100)
    , marker(NO_MARKER)
      //polystyle(poly_fill),
      //fontname("Helvetica"), fontface("NORMAL"), fontsize(12.0),
{
}

} // namespace POptions
