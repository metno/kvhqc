/*
HQC - Free Software for Manual Quality Control of Meteorological Observations

Copyright (C) 2013 met.no

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
#include "ListDialog.hh"

#include "BusyIndicator.h"
#include "FindAllParameters.hh"
#include "hqcmain.h"
#include "hqc_paths.hh"
#include "KvMetaDataBuffer.hh"
#include "TimeRangeControl.hh"
#include "timeutil.hh"

#include "ui_listdialog.h"
#include "ui_stationselection.h"

#include <QtGui/QInputDialog>
#include <QtGui/QMessageBox>

#include <boost/foreach.hpp>

#include <algorithm>

#define MILOGGER_CATEGORY "kvhqc.ListDialog"
#include "HqcLogging.hh"

namespace /* anonymous */ {
const int NCOUNTIES = 22;
const char* counties[NCOUNTIES] =  {
    "Oslo", "Akershus", "Østfold", "Hedmark", "Oppland", "Buskerud", "Vestfold", "Telemark", "Aust-Agder",
    "Vest-Agder", "Rogaland", "Hordaland", "Sogn og Fjordane",
    "Møre og Romsdal", "Sør-Trøndelag", "Nord-Trøndelag", "Nordland",
    "Troms", "Finnmark", "Ishavet",
    "Maritime", "Other"
};
const char* countiesU[NCOUNTIES] =  {
    "OSLO", "AKERSHUS", "ØSTFOLD", "HEDMARK", "OPPLAND", "BUSKERUD", "VESTFOLD", "TELEMARK",
    "AUST-AGDER", "VEST-AGDER", "ROGALAND", "HORDALAND", "SOGN OG FJORDANE", "MØRE OG ROMSDAL",
    "SØR-TRØNDELAG", "NORD-TRØNDELAG", "NORDLAND", "TROMS", "FINNMARK", "ISHAVET",
    "MARITIME", "OTHER"
};
const int REG_EAST[2]  = { 0, 9 };
const int REG_WEST[2]  = { 9, 13 };
const int REG_MID[2]   = { 13, 16 };
const int REG_NORTH[2] = { 16, 20 };

const char QSETTINGS_GROUP[] = "lstdlg";
} // anonymous namespace

// ========================================================================

ListDialog::ListDialog(HqcMainWindow* parent)
    : QDialog(parent)
    , ui(new Ui::ListDialog)
    , statSelect(0)
    , mTimeControl(new TimeRangeControl(this))
{
    ui->setupUi(this);

    setupStationTab();
    setupParameterTab();

    connect(ui->buttonSave,    SIGNAL(clicked()), this, SLOT(onSaveSettings()));
    connect(ui->buttonRestore, SIGNAL(clicked()), this, SLOT(onRestoreSettings()));
    
    connect(ui->hab, SIGNAL(hide()), this, SLOT(hide()));
    connect(ui->hab, SIGNAL(apply()), this, SIGNAL(ListApply()));

    mTimeControl->setMinimumGap(24);
    mTimeControl->install(ui->fromTime, ui->toTime);
    
    enableButtons();
}

ListDialog::~ListDialog()
{
}

ItemCheckBox* ListDialog::addCountyCheckbox(const QString& c, const char* cu, int& x, int& y)
{
  ItemCheckBox* countyCB = new ItemCheckBox(c, cu, ui->stCounty);
  connect(countyCB, SIGNAL(clicked()), this, SLOT(allCounUnCheck()));
  ui->statCountyLayout->addWidget(countyCB, x, y);
  y += 1; if (y >= 3 ) { y = 0; x += 1; }
  return countyCB;
}

void ListDialog::setupStationTab()
{
    // insert checkbuttons for station location selection
    int x=0, y=0;
    for(int i=0; i<NCOUNTIES; ++i) {
      ItemCheckBox* countyCB = addCountyCheckbox(counties[i], countiesU[i], x, y);
      mCounties.push_back(countyCB);
    }

    mCountyAll = addCountyCheckbox(tr("All"), "ALL", x, y);
    disconnect(mCountyAll, SIGNAL(clicked()), this, SLOT(allCounUnCheck()));
    
    connect(ui->regionEastAdd,     SIGNAL(clicked()), this, SLOT(regionEastAdd()));
    connect(ui->regionEastRemove,  SIGNAL(clicked()), this, SLOT(regionEastRemove()));
    connect(ui->regionWestAdd,     SIGNAL(clicked()), this, SLOT(regionWestAdd()));
    connect(ui->regionWestRemove,  SIGNAL(clicked()), this, SLOT(regionWestRemove()));
    connect(ui->regionMidAdd,      SIGNAL(clicked()), this, SLOT(regionMidAdd()));
    connect(ui->regionMidRemove,   SIGNAL(clicked()), this, SLOT(regionMidRemove()));
    connect(ui->regionNorthAdd,    SIGNAL(clicked()), this, SLOT(regionNorthAdd()));
    connect(ui->regionNorthRemove, SIGNAL(clicked()), this, SLOT(regionNorthRemove()));
    connect(mCountyAll,    SIGNAL(clicked()), this, SLOT(allCounCheck()));
    
    connect(ui->stationSelect,    SIGNAL(clicked()), this, SLOT(showStationSelectionDialog()));
    connect(ui->stationSelectAll, SIGNAL(clicked()), this, SLOT(selectAllStations()));

    onSetRecentTimes();
}

void ListDialog::setupParameterTab()
{
    METLIBS_LOG_SCOPE();

    const std::vector<int> empty;
    mParamSelectedModel.reset(new ParamIdModel(empty));
    ui->listParamChosen->setModel(mParamSelectedModel.get());

    connect(ui->buttonParamSelect, SIGNAL(clicked()), this, SLOT(selectParameters()));
    connect(ui->buttonParamDeselect, SIGNAL(clicked()), this, SLOT(deselectParameters()));
    connect(ui->buttonParamSelectAll, SIGNAL(clicked()), this, SLOT(selectAllParameters()));
    connect(ui->buttonParamDeselectAll, SIGNAL(clicked()), this, SLOT(deselectAllParameters()));
    connect(ui->comboParamGroup, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(showParamGroup(const QString&)));

    QSettings paramOrder(::hqc::getPath(::hqc::CONFDIR) + "/paramorder", QSettings::IniFormat);

    const QStringList groups = paramOrder.value("paramgroups").toStringList();
    if (groups.empty()) {
        QMessageBox::critical(this,
                              tr("Cannot read paramorder file"),
                              tr("The paramorder file could not be opened. Please set HQC_CONFDIR correctly."),
                              QMessageBox::Abort,
                              Qt::NoButton);
        ::exit(1);
    }

    QStringList labels;
    mParameterGroups.clear();
    BOOST_FOREACH(const QString& group, groups) {
        paramOrder.beginGroup(group);
        QString label = paramOrder.value("label").toString();
        std::vector<int>& parameters = mParameterGroups[label];

        labels << label;

        const QStringList paramIds = paramOrder.value("parameters").toStringList();
        BOOST_FOREACH(const QString& paramId, paramIds)
            parameters.push_back(paramId.toInt());
        paramOrder.endGroup();
    }

    try {
      const QString labelAll = tr("All in obs pgm");
      labels << labelAll;
      mParameterGroups.insert(std::make_pair(labelAll, Helpers::findAllParameters(true)));
    } catch (std::exception& ex) {
      METLIBS_LOG_WARN("failed to generate list of all parameters from kvalobs.obs_pgm table");
    }

    try {
      const std::list<kvalobs::kvParam>& allParams = KvMetaDataBuffer::instance()->allParams();
      const QString labelAll = tr("All defined");
      labels << labelAll;
      std::vector<int>& parameters = mParameterGroups[labelAll];

      BOOST_FOREACH(const kvalobs::kvParam& p, allParams)
          parameters.push_back(p.paramID());
     } catch (std::exception& ex) {
      METLIBS_LOG_WARN("failed to generate list of all parameters from kvalobs.param table");
    }

    ui->comboParamGroup->addItems(labels);
    ui->comboParamGroup->setCurrentIndex(0);
}

void ListDialog::saveSettings(QSettings& settings)
{
    settings.beginGroup(QSETTINGS_GROUP);
    doSaveSettings(settings);
    settings.endGroup();
}

void ListDialog::restoreSettings(QSettings& settings)
{
    settings.beginGroup(QSETTINGS_GROUP);
    doRestoreSettings(settings);
    settings.endGroup();
}

void ListDialog::doSaveSettings(QSettings& settings)
{
    {
        QStringList parameters;
        const std::vector<int> params = getSelectedParameters();
        BOOST_FOREACH(int pid, params)
            parameters << QString::number(pid);
        settings.setValue("selected_parameters", parameters);
    }

    const QStringList counties = getSelectedCounties();
    settings.setValue("counties", counties);

    settings.setValue("time_remember", ui->checkRememberTimes->isChecked());
    settings.setValue("time_from", ui->fromTime->dateTime());
    settings.setValue("time_to",   ui->toTime  ->dateTime());
}

void ListDialog::doRestoreSettings(QSettings& settings)
{
    {
        const QStringList parameters = settings.value("selected_parameters").toStringList();
        std::vector<int> params;
        BOOST_FOREACH(const QString& p, parameters)
            params.push_back(p.toInt());

        mParamSelectedModel.reset(new ParamIdModel(params));
        ui->listParamChosen->setModel(mParamSelectedModel.get());
        showParamGroup(ui->comboParamGroup->currentText());
    }

    QStringList countiesDefault;
    countiesDefault << "ALL";
    QStringList counties = settings.value("counties", countiesDefault).toStringList();
    setSelectedCounties(counties);

    removeAllStatFromListbox();

    ui->checkRememberTimes->setChecked(settings.value("time_remember", false).toBool());
    if (ui->checkRememberTimes->isChecked()) {
      ui->fromTime->setDateTime(settings.value("time_from").toDateTime());
      ui->toTime  ->setDateTime(settings.value("time_to"  ).toDateTime());
    }
}

void ListDialog::onSaveSettings()
{
    QString group = QString("data_") + QDateTime::currentDateTime().toString("yyyyMMddhhmmss");

    bool ok = false;
    QString label = QInputDialog::getText(this, tr("Save data selection"),
                                          tr("Name:"), QLineEdit::Normal, group, &ok);
    if (ok && !label.isEmpty()) {
        QSettings settings;
        const QStringList groups = settings.childGroups();
        BOOST_FOREACH(const QString g, groups) {
            const QString lud = settings.value(g + "/" + "label_user_data", "").toString();
            if (lud == label) {
                QMessageBox msgBox(this);
                msgBox.setIcon(QMessageBox::Question);
                msgBox.setWindowTitle(tr("Save data selection"));
                msgBox.setText(tr("Data selection with name '%1' exists.").arg(label));
                msgBox.setInformativeText(tr("Do you want to overwrite it?"));
                msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Cancel);
                msgBox.setDefaultButton(QMessageBox::Save);
                if (msgBox.exec() != QMessageBox::Save)
                    return;
                group = g;
                break;
            }
        }
        settings.beginGroup(group);
        settings.setValue("label_user_data", label);
        doSaveSettings(settings);
        settings.endGroup();
    }
}

void ListDialog::onRestoreSettings()
{
    QSettings settings;
    const QStringList groups = settings.childGroups();
    QStringList stored;
    BOOST_FOREACH(const QString g, groups) {
        const QString lud = settings.value(g + "/" + "label_user_data", "").toString();
        if (not lud.isEmpty())
            stored << lud;
    }
    if (stored.isEmpty()) {
        QMessageBox::information(this,
                                 tr("Load data selection"),
                                 tr("No saved data selections found."),
                                 QMessageBox::Ok, Qt::NoButton);
        return;
    }

    bool ok;
    QString recall = QInputDialog::getItem(this, tr("Load data selection"),
                                           tr("Name:"), stored, 0, false, &ok);
    if (ok && !recall.isEmpty()) {
        BOOST_FOREACH(const QString g, groups) {
            const QString lud = settings.value(g + "/" + "label_user_data", "").toString();
            if (not lud.isEmpty() and lud == recall) {
                settings.beginGroup(g);
                doRestoreSettings(settings);
                settings.endGroup();
                return;
            }
        }
    }
}

void ListDialog::onSetRecentTimes()
{
  if (not ui->checkRememberTimes->isChecked()) {
    QDateTime t = timeutil::nowWithMinutes0Seconds0();
    QDateTime f = t.addSecs(-2*24*3600 + 3600*(17-t.time().hour()) + 60*45);
    ui->fromTime->setDateTime(f);
    ui->toTime->setDateTime(t);
  }
}

void ListDialog::showParamGroup(const QString& paramGroup)
{
    const std::vector<int>& sel = mParamSelectedModel->parameterIds();
    std::set<int> sel_set(sel.begin(), sel.end());

    const std::vector<int>& group = mParameterGroups[paramGroup];
    std::vector<int> avail;
    BOOST_FOREACH(int pid, group) {
        if (sel_set.find(pid) == sel_set.end())
            avail.push_back(pid);
    }
    mParamAvailableModel.reset(new ParamIdModel(avail));
    ui->listParamAvailable->setModel(mParamAvailableModel.get());
}

void ListDialog::selectParameters()
{
    const std::vector<int>& add = mParamAvailableModel->parameterIds();

    std::vector<int> sel = mParamSelectedModel->parameterIds();
    std::set<int> sel_set(sel.begin(), sel.end());

    const QItemSelectionModel* selection = ui->listParamAvailable->selectionModel();
    for (size_t i=0; i<add.size(); ++i) {
        if (sel_set.find(add[i]) == sel_set.end() and selection->isRowSelected(i, QModelIndex()))
            sel.push_back(add[i]);
    }

    mParamSelectedModel.reset(new ParamIdModel(sel));
    ui->listParamChosen->setModel(mParamSelectedModel.get());

    showParamGroup(ui->comboParamGroup->currentText());
}

void ListDialog::selectAllParameters()
{
    const std::vector<int>& add = mParamAvailableModel->parameterIds();

    std::vector<int> sel = mParamSelectedModel->parameterIds();
    std::set<int> sel_set(sel.begin(), sel.end());

    for (size_t i=0; i<add.size(); ++i) {
        if (sel_set.find(add[i]) == sel_set.end())
            sel.push_back(add[i]);
    }

    mParamSelectedModel.reset(new ParamIdModel(sel));
    ui->listParamChosen->setModel(mParamSelectedModel.get());
    
    showParamGroup(ui->comboParamGroup->currentText());
}

void ListDialog::deselectParameters()
{
    const std::vector<int>& sel = mParamSelectedModel->parameterIds();
    std::vector<int> new_sel;
    const QItemSelectionModel* selection = ui->listParamChosen->selectionModel();
    for (size_t i=0; i<sel.size(); ++i) {
        if (not selection->isRowSelected(i, QModelIndex()))
            new_sel.push_back(sel[i]);
    }

    mParamSelectedModel.reset(new ParamIdModel(new_sel));
    ui->listParamChosen->setModel(mParamSelectedModel.get());

    showParamGroup(ui->comboParamGroup->currentText());
}

void ListDialog::deselectAllParameters()
{
    const std::vector<int> empty;
    mParamSelectedModel.reset(new ParamIdModel(empty));
    ui->listParamChosen->setModel(mParamSelectedModel.get());

    showParamGroup(ui->comboParamGroup->currentText());
}

TimeRange ListDialog::getTimeRange() const
{
  return mTimeControl->timeRange();
}

void ListDialog::appendStatInListbox(QString station)
{
    ui->stationNames->insertItem(station);
    enableButtons();
}

void ListDialog::removeStatFromListbox(QString station)
{
    int rind = -1;
    for (int ind = 0; ind < ui->stationNames->numRows(); ind++) {
        if (ui->stationNames->text(ind) == station) {
            rind = ind;
        }
    }
    if (rind >= 0 )
        ui->stationNames->removeItem(rind);
    enableButtons();
}

void ListDialog::enableButtons()
{
    const bool allowSelectStation = not getSelectedCounties().empty();
    const bool haveStations = (ui->stationNames->count() > 0);

    const bool haveParameters = not getSelectedParameters().empty();

    const bool allowApply = haveStations and allowSelectStation
        and haveParameters;

    ui->stationSelect   ->setEnabled(allowSelectStation);
    ui->stationSelectAll->setEnabled(allowSelectStation);

    ui->hab->setCanApply(allowApply);
}

void ListDialog::removeAllStatFromListbox()
{
    int nuRo = ui->stationNames->count();
    for (int ind = 0; ind < nuRo; ind++)
        ui->stationNames->removeItem(0);
    enableButtons();
}

void ListDialog::regionEastAdd()
{
    regionEastToggle(true);
}

void ListDialog::regionEastRemove()
{
    regionEastToggle(false);
}

void ListDialog::regionEastToggle(bool on)
{
    for(int i=REG_EAST[0]; i<REG_EAST[1]; ++i)
        mCounties[i]->setChecked(on);
    allCounUnCheck();
}

void ListDialog::regionWestAdd()
{
    regionWestToggle(true);
}

void ListDialog::regionWestRemove()
{
    regionWestToggle(false);
}

void ListDialog::regionWestToggle(bool on)
{
    for(int i=REG_WEST[0]; i<REG_WEST[1]; ++i)
        mCounties[i]->setChecked(on);
    allCounUnCheck();
}

void ListDialog::regionMidAdd()
{
    regionMidToggle(true);
}

void ListDialog::regionMidRemove()
{
    regionMidToggle(false);
}

void ListDialog::regionMidToggle(bool on)
{
    for(int i=REG_MID[0]; i<REG_MID[1]; ++i)
        mCounties[i]->setChecked(on);
    allCounUnCheck();
}

void ListDialog::regionNorthAdd()
{
    regionNorthToggle(true);
}

void ListDialog::regionNorthRemove()
{
    regionNorthToggle(false);
}

void ListDialog::regionNorthToggle(bool on)
{
    for(int i=REG_NORTH[0]; i<REG_NORTH[1]; ++i)
        mCounties[i]->setChecked(on);
    allCounUnCheck();
}

void ListDialog::allCounCheck()
{
    if (mCountyAll->isChecked()) {
        for(int i=0; i<NCOUNTIES; ++i)
            mCounties[i]->setChecked(false);
    }
    enableButtons();
}

void ListDialog::allCounUnCheck()
{
    bool anyCountyIsChecked = false;
    for(int i=0; not anyCountyIsChecked and i<NCOUNTIES; ++i)
        anyCountyIsChecked |= mCounties[i]->isChecked();
    if (anyCountyIsChecked)
        mCountyAll->setChecked(false);
    enableButtons();
}

QStringList ListDialog::getSelectedCounties()
{
  QStringList t;
  if (mCountyAll->isChecked())
    t << "ALL";
  
  for(int i=0; i<NCOUNTIES; ++i) {
    ItemCheckBox* cb = mCounties[i];
    if (cb->isChecked())
      t << cb->getItem();
  }
  return t;
}

void ListDialog::setSelectedCounties(const QStringList& c)
{
  mCountyAll->setChecked(c.contains("ALL"));
  
  for(int i=0; i<NCOUNTIES; ++i) {
    ItemCheckBox* cb = mCounties[i];
    cb->setChecked(c.contains(cb->getItem()));
  }
  enableButtons();
}

std::vector<int> ListDialog::getSelectedStations()
{
    if (not statSelect.get())
        return std::vector<int>();

    return statSelect->getSelectedStations();
}

std::vector<int> ListDialog::getSelectedParameters()
{
    return mParamSelectedModel->parameterIds();
}

void ListDialog::prepareStationSelectionDialog()
{
    const listStat_l& listStat = StationInfoBuffer::instance()->getStationDetails();

    removeAllStatFromListbox();
    statSelect.reset(new StationSelection(listStat,
                                          getSelectedCounties(),
                                          this));
    connect(statSelect.get(), SIGNAL(stationAppended(QString)), this, SLOT(appendStatInListbox(QString)));
    connect(statSelect.get(), SIGNAL(stationRemoved(QString)),  this, SLOT(removeStatFromListbox(QString)));
}

void ListDialog::showStationSelectionDialog()
{
    prepareStationSelectionDialog();
    statSelect->show();
}

void ListDialog::selectAllStations()
{
    prepareStationSelectionDialog();
    statSelect->doSelectAllStations();
}
