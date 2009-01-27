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
#include "helptree.h"
#include <qstatusbar.h>
#include <qpixmap.h>
#include <q3popupmenu.h>
#include <qmenubar.h>
#include <q3toolbar.h>
#include <qtoolbutton.h>
#include <qicon.h>
#include <qfile.h>
#include <q3textstream.h>
#include <q3stylesheet.h>
#include <qmessagebox.h>
#include <q3filedialog.h>
#include <qapplication.h>
#include <qcombobox.h>
#include <qevent.h>
#include <qlineedit.h>
#include <qobject.h>
#include <qfileinfo.h>
#include <qfile.h>
#include <qdatastream.h>
#include <qprinter.h>
#include <q3simplerichtext.h>
#include <qpainter.h>
#include <q3paintdevicemetrics.h>
//Added by qt3to4:
#include <Q3Frame>

#include <ctype.h>

HelpTree::HelpTree( const QString& home_, const QString& _path,
			QWidget* parent, const char *name )
    : Q3MainWindow( parent, name, Qt::WDestructiveClose ),
      pathCombo( 0 )
{
    readHistory();

    browser = new Q3TextBrowser( this );

    browser->mimeSourceFactory()->setFilePath( _path );
    browser->setFrameStyle( Q3Frame::Panel | Q3Frame::Sunken );
    connect( browser, SIGNAL( sourceChanged(const QString& ) ),
	     this, SLOT( sourceChanged( const QString&) ) );

    setCentralWidget( browser );

    if ( !home_.isEmpty() )
	browser->setSource( home_ );

    connect( browser, SIGNAL( highlighted( const QString&) ),
	     statusBar(), SLOT( message( const QString&)) );

    setMinimumSize( 840,700 );

    Q3PopupMenu* file = new Q3PopupMenu( this );
    file->insertItem( tr("&New Window"), this, SLOT( newWindow() ), Qt::CTRL+Qt::Key_N );
    file->insertItem( tr("&Open File"), this, SLOT( openFile() ), Qt::CTRL+Qt::Key_O );
    file->insertItem( tr("&Print"), this, SLOT( print() ), Qt::CTRL+Qt::Key_P );
    file->insertSeparator();
    file->insertItem( tr("&Close"), this, SLOT( close() ), Qt::CTRL+Qt::Key_Q );
    file->insertItem( tr("E&xit"), qApp, SLOT( closeAllWindows() ), Qt::CTRL+Qt::Key_X );

    // The same three icons are used twice each.
    QIcon icon_back( QPixmap("back.xpm") );
    QIcon icon_forward( QPixmap("forward.xpm") );
    QIcon icon_home( QPixmap("home.xpm") );

    Q3PopupMenu* go = new Q3PopupMenu( this );
    backwardId = go->insertItem( icon_back,
				 tr("&Backward"), browser, SLOT( backward() ),
				 Qt::CTRL+Qt::Key_Left );
    forwardId = go->insertItem( icon_forward,
				tr("&Forward"), browser, SLOT( forward() ),
				Qt::CTRL+Qt::Key_Right );
    go->insertItem( icon_home, tr("&Home"), browser, SLOT( home() ) );

    hist = new Q3PopupMenu( this );
    QStringList::Iterator it = history.begin();
    for ( ; it != history.end(); ++it )
	mHistory[ hist->insertItem( *it ) ] = *it;
    connect( hist, SIGNAL( activated( int ) ),
	     this, SLOT( histChosen( int ) ) );

    menuBar()->insertItem( tr("&File"), file );
    menuBar()->insertItem( tr("&Go"), go );
    menuBar()->insertItem( tr( "History" ), hist );

    menuBar()->setItemEnabled( forwardId, FALSE);
    menuBar()->setItemEnabled( backwardId, FALSE);
    connect( browser, SIGNAL( backwardAvailable( bool ) ),
	     this, SLOT( setBackwardAvailable( bool ) ) );
    connect( browser, SIGNAL( forwardAvailable( bool ) ),
	     this, SLOT( setForwardAvailable( bool ) ) );


    Q3ToolBar* toolbar = new Q3ToolBar( this );
    addToolBar( toolbar, "Toolbar");
    QToolButton* button;

    button = new QToolButton( icon_back, tr("Backward"), "", browser, SLOT(backward()), toolbar );
    connect( browser, SIGNAL( backwardAvailable(bool) ), button, SLOT( setEnabled(bool) ) );
    button->setEnabled( FALSE );
    button = new QToolButton( icon_forward, tr("Forward"), "", browser, SLOT(forward()), toolbar );
    connect( browser, SIGNAL( forwardAvailable(bool) ), button, SLOT( setEnabled(bool) ) );
    button->setEnabled( FALSE );
    button = new QToolButton( icon_home, tr("Home"), "", browser, SLOT(home()), toolbar );

    toolbar->addSeparator();

    pathCombo = new QComboBox( TRUE, toolbar );
    connect( pathCombo, SIGNAL( activated( const QString & ) ),
	     this, SLOT( pathSelected( const QString & ) ) );
    toolbar->setStretchableWidget( pathCombo );
    setRightJustification( TRUE );
    setDockEnabled( Qt::DockLeft, FALSE );
    setDockEnabled( Qt::DockRight, FALSE );

    pathCombo->insertItem( home_ );
    browser->setFocus();

}


void HelpTree::setBackwardAvailable( bool b)
{
    menuBar()->setItemEnabled( backwardId, b);
}

void HelpTree::setForwardAvailable( bool b)
{
    menuBar()->setItemEnabled( forwardId, b);
}


void HelpTree::sourceChanged( const QString& url )
{
    if ( browser->documentTitle().isNull() )
	setCaption( "Hqc hjelp - " + url );
    else
	setCaption( "Hqc hjelp - " + browser->documentTitle() ) ;

    if ( !url.isEmpty() && pathCombo ) {
	bool exists = FALSE;
	int i;
	for ( i = 0; i < pathCombo->count(); ++i ) {
	    if ( pathCombo->text( i ) == url ) {
		exists = TRUE;
		break;
	    }
	}
	if ( !exists ) {
	    pathCombo->insertItem( url, 0 );
	    pathCombo->setCurrentItem( 0 );
	    mHistory[ hist->insertItem( url ) ] = url;
	} else
	    pathCombo->setCurrentItem( i );
    }
}

HelpTree::~HelpTree()
{
    history =  mHistory.values();

    QFile f( QDir::currentDirPath() + "/.history" );
    f.open( QIODevice::WriteOnly );
    QDataStream s( &f );
    s << history;
    f.close();

}

void HelpTree::openFile()
{
#ifndef QT_NO_FILEDIALOG
    QString fn = Q3FileDialog::getOpenFileName( QString::null, QString::null, this );
    if ( !fn.isEmpty() )
	browser->setSource( fn );
#endif
}

void HelpTree::newWindow()
{
    ( new HelpTree(browser->source(), "qbrowser") )->showMaximized();
}

void HelpTree::print()
{
#ifndef QT_NO_PRINTER
    QPrinter printer( QPrinter::HighResolution );
    printer.setFullPage(TRUE);
    if ( printer.setup( this ) ) {
	QPainter p( &printer );
	if( !p.isActive() ) // starting printing failed
	    return;
	Q3PaintDeviceMetrics metrics(p.device());
	int dpiy = metrics.logicalDpiY();
	int margin = (int) ( (2/2.54)*dpiy ); // 2 cm margins
	QRect body( margin, margin, metrics.width() - 2*margin, metrics.height() - 2*margin );
	Q3SimpleRichText richText( browser->text(),
				  QFont(),
				  browser->context(),
				  browser->styleSheet(),
				  browser->mimeSourceFactory(),
				  body.height() );
	richText.setWidth( &p, body.width() );
	QRect view( body );
	int page = 1;
	do {
	    richText.draw( &p, body.left(), body.top(), view, colorGroup() );
	    view.moveBy( 0, body.height() );
	    p.translate( 0 , -body.height() );
	    p.drawText( view.right() - p.fontMetrics().width( QString::number(page) ),
			view.bottom() + p.fontMetrics().ascent() + 5, QString::number(page) );
	    if ( view.top()  >= richText.height() )
		break;
	    printer.newPage();
	    page++;
	} while (TRUE);
    }
#endif
}

void HelpTree::pathSelected( const QString &_path )
{
    browser->setSource( _path );
    if ( mHistory.values().contains(_path) )
	mHistory[ hist->insertItem( _path ) ] = _path;
}

void HelpTree::readHistory()
{
    if ( QFile::exists( QDir::currentDirPath() + "/.history" ) ) {
	QFile f( QDir::currentDirPath() + "/.history" );
	f.open( QIODevice::ReadOnly );
	QDataStream s( &f );
	s >> history;
	f.close();
	while ( history.count() > 20 )
	    history.remove( history.begin() );
    }
}

void HelpTree::histChosen( int i )
{
    if ( mHistory.contains( i ) )
	browser->setSource( mHistory[ i ] );
}
