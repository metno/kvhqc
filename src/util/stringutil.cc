/*
  HQC - Free Software for Manual Quality Control of Meteorological Observations

  Copyright (C) 2018 met.no

  Contact information:
  Norwegian Meteorological Institute
  Box 43 Blindern
  0313 OSLO
  NORWAY
  email: kvalobs-dev@met.no

  This file is part of HQC

  HQC is free software; you can redistribute it and/or modify it under
  the terms of the GNU General Public License as published by the Free
  Software Foundation; either version 2 of the License, or (at your
  option) any later version.

  HQC is distributed in the hope that it will be useful, but WITHOUT ANY
  WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
  for more details.

  You should have received a copy of the GNU General Public License
  along with HQC; if not, write to the Free Software Foundation, Inc.,
  51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
*/


#include "stringutil.hh"

#include "Helpers.hh"

#include <cmath>
#include <sstream>

namespace Helpers {

char int2char(int i)
{
  if (i < 10)
    return ('0' + i);
  else
    return ('A' + (i - 10));
}

// ------------------------------------------------------------------------

QString fromLatin1(const std::string& text)
{
  return QString::fromStdString(text);
}

// ------------------------------------------------------------------------

QString fromUtf8(const std::string& text)
{
  return QString::fromUtf8(text.c_str());
}

// ------------------------------------------------------------------------

QString& appendText(QString& text, const QString& append, const QString& separator)
{
  if (append.isEmpty())
    return text;
  if (not text.isEmpty())
    text += separator;
  text += append;
  return text;
}

// ------------------------------------------------------------------------

QString appendedText(const QString& text, const QString& append, const QString& separator)
{
  QString t(text);
  appendText(t, append, separator);
  return t;
}

// ------------------------------------------------------------------------

float parseFloat(const QString& text, int nDecimals)
{
  bool numOk = false;
  const float num = text.toFloat(&numOk);
  if (not numOk)
    throw std::runtime_error("cannot parse number");
  const float factor = std::pow(10, nDecimals), numf = num * factor, roundedf = Helpers::round(numf, 1);
  if (std::fabs(numf - roundedf) >= 1e-8) {
    std::ostringstream w;
    w << "text '" << text.toStdString() << "' converted to value " << num << " has unsupported precision (rounded value is " << roundedf / factor << ")";
    throw std::runtime_error(w.str());
  }
  return num;
}

} // namespace Helpers
