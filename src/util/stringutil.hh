/* -*- c++ -*-
  Kvalobs - Free Quality Control Software for Meteorological Observations

  Copyright (C) 2012 met.no

  Contact information:
  Norwegian Meteorological Institute
  Postboks 43 Blindern
  N-0313 OSLO
  NORWAY
  email: kvalobs-dev@met.no

  This file is part of KVALOBS

  KVALOBS is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License as
  published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  KVALOBS is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with KVALOBS; if not, write to the Free Software Foundation Inc.,
  51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef UTIL_STRINGUTIL_HH
#define UTIL_STRINGUTIL_HH

#include <QString>
#include <string>

namespace Helpers {

/*! Convert an integer to a hexadecimal digit.
 * \param i integer 0-15, no range check
 * \return hexadecimal digit as char
 */
char int2char(int i);

QString fromLatin1(const std::string& text);
QString fromUtf8(const std::string& text);

/*! Append a text with a separator.
 * \param text add to this string (will be modified)
 * \param append will be appended to \c text
 * \param separator will be put between between \c text and \c append unless one of the is empty
 * \return a reference to \c text (which might have been modified)
 */
QString& appendText(QString& text, const QString& append, const QString& separator = ", ");

/*! Append text with a separator without modifying the input.
 * \sa Helpers::appendText
 * \return \c text + \c separator + \c append, or \c text if \c append is empty, or \c append if \c text is empty
 */
QString appendedText(const QString& text, const QString& append, const QString& separator = ", ");

/*! Parse a number with maximum allowed precision.
 * \param text to be parsed as number
 * \param nDecimals the parsed value, divided by \f$ 1ß ^nDecimals \f$, must be integer
 * \throw std::runtime_error if not a number or if too many decimals are given
 */
float parseFloat(const QString& text, int nDecimals);

} // namespace Helpers

#endif /* UTIL_STRINGUTIL_HH */
