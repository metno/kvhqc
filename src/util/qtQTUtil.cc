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
#include <qcombobox.h>
#include <q3listbox.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qlcdnumber.h>
#include <qcheckbox.h>
#include <qslider.h>
#include <qpixmap.h>

#include <qtQTUtil.h>

#include <iostream>

//#define IN_COLOR  QColor(0,80,0)
#define OUT_COLOR QColor(192,192,192)
#define isMotif true

namespace /*anonymous*/ {

/*********************************************/
QPixmap* linePixmap(const std::string& pattern, int linewidth)
{

  // make a 32x20 pixmap of a linepattern of length 16 (where ' ' is empty)

  char xpmEmpty[] = "################################";
  char xpmLine[] =  "................................";
  int lw = linewidth;
  if (lw<1)
      lw=1;
  else if (lw>20)
      lw=20;

  if (pattern.length()>=16) {
    for (int i=0; i<16; i++)
      if (pattern[i]==' ') xpmLine[16+i]= xpmLine[i] = '#';
  }

  int l1= 10 - lw/2;
  int l2= l1 + lw;

  const char* xpmData[3+20];
  xpmData[0] = "32 20 2 1";
  xpmData[1] = ". c #000000";
  xpmData[2] = "# c None";

  for (int i=0;  i<l1; i++) xpmData[3+i]= xpmEmpty;
  for (int i=l1; i<l2; i++) xpmData[3+i]= xpmLine;
  for (int i=l2; i<20; i++) xpmData[3+i]= xpmEmpty;

  return new QPixmap(xpmData);
}


} // anonymous namespace

/*********************************************/
QLabel* TitleLabel( const char* name, QWidget* parent){
  QLabel* label= new QLabel( name, parent );
  label->setPaletteForegroundColor ( QColor(0,0,128) );

  return label;
}



/*********************************************/
QPushButton* SmallPushButton( const char* name, QWidget* parent){
  QPushButton* b = new QPushButton( name, parent);

  QString qstr=name;
  int height = int(b->fontMetrics().height()*1.4);
  int width  = int(b->fontMetrics().width(qstr)+ height*0.5);
  b->setMaximumSize( width, height );

  return b;
}


/*********************************************/
QPushButton* NormalPushButton( const char* name, QWidget* parent){
  QPushButton* b = new QPushButton( name, parent);
  return b;
}


/*********************************************/
QPushButton* PixmapButton(const QPixmap& pixmap, QWidget* parent,
			  int deltaWidth, int deltaHeight ) {

  QPushButton* b = new QPushButton( parent );

  b->setIconSet(QIcon(pixmap));

  int width  = pixmap.width()  + deltaWidth;
  int height = pixmap.height() + deltaHeight;

  b->setMinimumSize( width, height );
  b->setMaximumSize( width, height );

  return b;
}

/*********************************************/
QComboBox* ComboBox( QWidget* parent, QColor* pixcolor, int nr_colors,
		     bool Enabled, int defItem  ){
  int t;
  QPixmap** pmap = new QPixmap*[nr_colors];
  for( t=0; t<nr_colors; t++ )
    pmap[t] = new QPixmap( 20, 20 );

  for( t=0; t<nr_colors; t++ )
    pmap[t]->fill( pixcolor[t] );

  QComboBox* box = new QComboBox( false, parent );

  for( int i=0; i < nr_colors; i++){
    box->insertItem ( *pmap[i] );
  }

  box->setEnabled( Enabled );

//   if( isMotif )
//     box->setPalette( QPalette( OUT_COLOR, OUT_COLOR ) );

  box->setCurrentItem(defItem);

  for( t=0; t<nr_colors; t++ ){
    delete pmap[t];
    pmap[t]=0;
  }

  delete[] pmap;
  pmap=0;

  return box;
}

/*********************************************/
QComboBox* LinetypeBox( QWidget* parent, bool Enabled, int defItem  )
{

  QComboBox* box = new QComboBox( false, parent );

  int nr_linetypes= 5;
  QPixmap** pmapLinetypes = new QPixmap*[nr_linetypes];

  pmapLinetypes[0]= linePixmap("--------------- ",3);
  pmapLinetypes[1]= linePixmap("------   ------ ",3);
  pmapLinetypes[2]= linePixmap("---   ---   --- ",3);
  pmapLinetypes[3]= linePixmap("-------  -   -  ",3);
  pmapLinetypes[4]= linePixmap("--    --    --  ",3);

  for( int i=0; i < nr_linetypes; i++)
    box->insertItem ( *pmapLinetypes[i] );

  return box;
}

/*********************************************/
QLCDNumber* LCDNumber( uint numDigits, QWidget * parent ){
  QLCDNumber* lcdnum = new QLCDNumber( numDigits, parent );
  lcdnum->setSegmentStyle ( QLCDNumber::Flat );
//   lcdnum->setMinimumSize( lcdnum->sizeHint() );
//   lcdnum->setMaximumSize( lcdnum->sizeHint() );
  return lcdnum;
}


/*********************************************/
QSlider* Slider( int minValue, int maxValue, int pageStep, int value,
		 Qt::Orientation orient, QWidget* parent, int width ){
  QSlider* slider = new QSlider( minValue, maxValue, pageStep, value,
				 orient, parent);
  slider->setMinimumSize( slider->sizeHint() );
  slider->setMaximumWidth( width );
  return slider;
}

/*********************************************/
QSlider* Slider( int minValue, int maxValue, int pageStep, int value,
		 Qt::Orientation orient, QWidget* parent ){
  QSlider* slider = new QSlider( minValue, maxValue, pageStep, value,
				 orient, parent);
  slider->setMinimumSize( slider->sizeHint() );
  slider->setMaximumSize( slider->sizeHint() );
  return slider;
}
