/*
HQC - Free Software for Manual Quality Control of Meteorological Observations

$Id$

Copyright (C) 2007 met.no

Contact information:
Norwegian Meteorological Institute
Box 43 Blindern
0313 OSLO
NORWAY
email: kvalobs-dev@met.no

This file is part of HQC

HQC is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

HQC is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License along
with HQC; if not, write to the Free Software Foundation Inc.,
51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/
#ifndef _qtqtutil_h
#define _qtqtutil_h

#include <miString.h>
#include <vector>
//Added by qt3to4:
#include <QPixmap>
#include <QLabel>

using namespace std; 

class QPushButton;
class QComboBox;
class Q3ListBox;
class QLabel;
class QLCDNumber;
class QCheckBox;
class QSlider;
class QPixmap;

// Lables

QLabel* TitleLabel( const char* name, QWidget* parent);

// PushButtons

QPushButton* SmallPushButton( const char* name, QWidget* parent);

QPushButton* NormalPushButton( const char* name, QWidget* parent);

QPushButton* PixmapButton( const QPixmap& pixmap, QWidget* parent,
			 int deltaWidth=0, int deltaHeight=0);

// ComboBox

QComboBox* ComboBox(QWidget* parent, vector<miString> vstr, 
		    bool Enabled=true, int defItem=0);

QComboBox* ComboBox(QWidget* parent, QColor* pixcolor, int nr_colors, 
		    bool Enabled=true, int defItem=0);

QComboBox* LinetypeBox(QWidget* parent, 
		    bool Enabled=true, int defItem=0);

QComboBox* LinewidthBox(QWidget* parent, 
		    bool Enabled=true, int defItem=0);

// Div

QLCDNumber* LCDNumber(uint numDigits, QWidget* parent=0);

QSlider* Slider( int minValue, int maxValue, int pageStep, int value,  
		 Qt::Orientation orient, QWidget* parent, int width );

QSlider* Slider( int minValue, int maxValue, int pageStep, int value,  
		 Qt::Orientation orient, QWidget* parent );

void listBox( Q3ListBox* box, vector<miString> vstr, int defItem=-1 );

QPixmap* linePixmap(const miutil::miString& pattern, int linewidth);


#endif
