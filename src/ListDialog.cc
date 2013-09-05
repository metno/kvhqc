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
#include "StInfoSysBuffer.hh"
#include "TimeRangeControl.hh"
#include "timeutil.hh"

#include "ui_listdialog.h"

#include <QtGui/QInputDialog>
#include <QtGui/QMessageBox>
#include <QtGui/QStandardItem>
#include <QtGui/QStandardItemModel>

#include <boost/foreach.hpp>

#include <algorithm>

#define MILOGGER_CATEGORY "kvhqc.ListDialog"
#include "HqcLogging.hh"

namespace /* anonymous */ {

// TODO build county list automatically -- but how to define regions?

const int NCOUNTIES = 23;
const char* counties[NCOUNTIES] =  {
    "Oslo", "Akershus", "Østfold", "Hedmark", "Oppland", "Buskerud", "Vestfold", "Telemark", "Aust-Agder",
    "Vest-Agder", "Rogaland", "Hordaland", "Sogn og Fjordane",
    "Møre og Romsdal", "Sør-Trøndelag", "Nord-Trøndelag", "Nordland",
    "Troms", "Finnmark", "Ishavet",
    "Maritime", "Skip", "Andre"
};
const char* countiesU[NCOUNTIES] =  {
    "OSLO", "AKERSHUS", "ØSTFOLD", "HEDMARK", "OPPLAND", "BUSKERUD", "VESTFOLD", "TELEMARK", "AUST-AGDER",
    "VEST-AGDER", "ROGALAND", "HORDALAND", "SOGN OG FJORDANE",
    "MØRE OG ROMSDAL", "SØR-TRØNDELAG", "NORD-TRØNDELAG", "NORDLAND",
    "TROMS", "FINNMARK", "ISHAVET",
    "MARITIME", "SKIP", "OTHER"
};

const int NREGIONS = 5;
const int REGION_COUNTIES[NREGIONS+1] = { 0, 9, 13, 16, 20, NCOUNTIES };
const char* regions[NREGIONS] =  {
  "Østlandet", "Vestlandet", "Midt-Norge", "Nord-Norge", "Andre"
};

const char QSETTINGS_GROUP[] = "lstdlg";

void setChildren(QStandardItem* parent, bool on)
{
  const int cc = parent->rowCount();
  for (int c=0; c<cc; ++c) {
    QStandardItem* child = parent->child(c, 0);
    child->setCheckState(on ? Qt::Checked : Qt::Unchecked);
    setChildren(child, on);
  }
}
void checkChildren(QStandardItem* parent)
{
  const int cc = parent->rowCount();
  int nChecked = 0;
  for (int c=0; c<cc; ++c) {
    QStandardItem* child = parent->child(c, 0);
    if (child->checkState() != Qt::Unchecked)
      nChecked += 1;
  }
  const Qt::CheckState pcs = parent->checkState();
  const Qt::CheckState pcs_new = (nChecked == cc) ? Qt::Checked
      : ((nChecked == 0) ? Qt::Unchecked : Qt::PartiallyChecked);
  if (pcs != pcs_new)
    parent->setCheckState(pcs_new);
}
} // anonymous namespace

// ========================================================================

ListDialog::ListDialog(HqcMainWindow* parent)
    : QDialog(parent)
    , ui(new Ui::ListDialog)
    , mTimeControl(new TimeRangeControl(this))
    , mIsInToggle(false)
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

void ListDialog::setupStationTab()
{
  METLIBS_LOG_SCOPE();
  const listStat_l& listStat = StationInfoBuffer::instance()->getStationDetails();
    
  mStationModel.reset(new QStandardItemModel(this));
  QStandardItem *root = mStationModel->invisibleRootItem();

  mStationModel->setHorizontalHeaderLabels(QStringList()
      << tr("Station/Region") << tr("Name") << tr("HOH")
      << tr("County") << tr("Commune") << tr("Pri"));

  typedef std::map<QString, QStandardItem*> county2item_t;
  county2item_t county2region, county2county;
  
  for (int i=0; i<NREGIONS; ++i) {
    QStandardItem *r_item = new QStandardItem(regions[i]);
    r_item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsTristate);
    r_item->setCheckable(true);
    root->appendRow(r_item);
    for (int j=REGION_COUNTIES[i]; j<REGION_COUNTIES[i+1]; ++j)
      county2region.insert(std::make_pair(QString(countiesU[j]), r_item));
  }
  
  for (int i=0; i<NCOUNTIES; ++i) {
    QStandardItem *c_item = new QStandardItem(counties[i]);
    c_item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsTristate);
    c_item->setCheckable(true);
    const QString cu(countiesU[i]);
    county2item_t::iterator r_it = county2region.find(cu);
    if (r_it != county2region.end()) {
      r_it->second->appendRow(c_item);
      county2county.insert(std::make_pair(cu, c_item));
    } else {
      METLIBS_LOG_WARN("no region for county '" << cu << "'");
    }
  }
    
  BOOST_FOREACH(const listStat_t& s, listStat) {
    const QString cu = QString::fromStdString(s.fylke);
    const QString prty = (s.pri > 0) ? QString("PRI%1").arg(s.pri) : QString();

    QStandardItem *s_item = new QStandardItem(QString::number(s.stationid));

    QList<QStandardItem*> s_items;
    s_items << s_item
            << new QStandardItem(QString::fromStdString(s.name))
            << new QStandardItem(QString::number(s.altitude, 'f', 0))
            << new QStandardItem(cu)
            << new QStandardItem(QString::fromStdString(s.kommune))
            << new QStandardItem(prty);
    Q_FOREACH(QStandardItem* i, s_items) {
      i->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    }
    s_item->setCheckable(true);

    county2item_t::iterator c_it = county2county.find(cu);
    if (c_it != county2county.end()) {
      c_it->second->appendRow(s_items);
    } else {
      METLIBS_LOG_WARN("no county '" << s.fylke << "' for station " << s.stationid);
    }
  }

  ui->treeStations->setModel(mStationModel.get());

  ui->treeStations->expandAll();
  ui->treeStations->header()->resizeSections(QHeaderView::ResizeToContents);
  ui->treeStations->header()->setResizeMode(QHeaderView::Interactive);
  ui->treeStations->collapseAll();

  ui->treeStations->setSelectionMode(QAbstractItemView::ExtendedSelection);

  connect(mStationModel.get(), SIGNAL(itemChanged(QStandardItem*)),
      this, SLOT(onItemChanged(QStandardItem*)));
  
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
    QStringList counties = settings.value("counties", countiesDefault).toStringList();
    setSelectedCounties(counties);

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

void ListDialog::enableButtons()
{
    const bool haveStations   = not getSelectedStations()  .empty();
    const bool haveParameters = not getSelectedParameters().empty();

    const bool allowApply = haveStations and haveParameters;
    ui->hab->setCanApply(allowApply);
}

QStringList ListDialog::getSelectedCounties()
{
  METLIBS_LOG_SCOPE();

  QStringList t;

  typedef std::map<QString, QString> county2county_t;
  county2county_t county2county;
  for (int i=0; i<NCOUNTIES; ++i)
    county2county.insert(std::make_pair(QString::fromLatin1(counties[i]), QString::fromLatin1(countiesU[i])));

  QStandardItem* root = mStationModel->invisibleRootItem();
  const int nRegions = root->rowCount();
  for (int r=0; r<nRegions; ++r) {
    QStandardItem* region = root->child(r, 0);
    const int nCounties = region->rowCount();
    for (int c=0; c<nCounties; ++c) {
      QStandardItem* county = region->child(c, 0);
      if (county->checkState() != Qt::Unchecked) {
        county2county_t::const_iterator cit = county2county.find(county->text());
        if (cit != county2county.end()) {
          METLIBS_LOG_DEBUG("county '" << county->text() << "' in region '" << region->text() << "' is selected");
          t << cit->second;
        } else {
          METLIBS_LOG_WARN("county '" << county->text() << "' not known");
        }
      }
    }
  }
  
  return t;
}

void ListDialog::setSelectedCounties(const QStringList& selectedCounties)
{
  METLIBS_LOG_SCOPE();

  typedef std::map<QString, QString> county2county_t;
  county2county_t county2county;
  for (int i=0; i<NCOUNTIES; ++i)
    county2county.insert(std::make_pair(QString::fromLatin1(counties[i]), QString::fromLatin1(countiesU[i])));

  QStandardItem* root = mStationModel->invisibleRootItem();
  const int nRegions = root->rowCount();
  for (int r=0; r<nRegions; ++r) {
    QStandardItem* region = root->child(r, 0);
    const int nCounties = region->rowCount();
    for (int c=0; c<nCounties; ++c) {
      QStandardItem* county = region->child(c, 0);
      county2county_t::const_iterator cit = county2county.find(county->text());
      if (cit != county2county.end()) {
        METLIBS_LOG_DEBUG("(de)selecting county '" << county->text() << "' in region '" << region->text() << "'");
        county->setCheckState(selectedCounties.contains(cit->second) ? Qt::Checked : Qt::Unchecked);
      } else {
        METLIBS_LOG_WARN("county '" << county->text() << "' not known");
      }
    }
  }
  enableButtons();
}

std::vector<int> ListDialog::getSelectedStations()
{
  std::vector<int> stations;

  QStandardItem* root = mStationModel->invisibleRootItem();
  const int nRegions = root->rowCount();
  for (int r=0; r<nRegions; ++r) {
    QStandardItem* region = root->child(r, 0);
    const int nCounties = region->rowCount();
    for (int c=0; c<nCounties; ++c) {
      QStandardItem* county = region->child(c, 0);
      const int nStations = county->rowCount();
      for (int s=0; s<nStations; ++s) {
        QStandardItem* station = county->child(s, 0);
        if (station->checkState() != Qt::Unchecked)
          stations.push_back(station->text().toInt());
      }
    }
  }
  
  return stations;
}

std::vector<int> ListDialog::getSelectedParameters()
{
  return mParamSelectedModel->parameterIds();
}

void ListDialog::onItemChanged(QStandardItem* item)
{
  if (mIsInToggle)
    return;

  METLIBS_LOG_SCOPE();
  mIsInToggle = true;

  QStandardItem* parent = item->parent();
  if (not parent) {
    METLIBS_LOG_DEBUG("region '" << item->text() << "' toggled");
    setChildren(item, item->checkState() != Qt::Unchecked);
  } else {
    QStandardItem* grandparent = parent->parent();
    if (not grandparent) {
      METLIBS_LOG_DEBUG("county '" << item->text() << "' toggled");
      setChildren(item, item->checkState() != Qt::Unchecked);
      checkChildren(parent);
    } else {
      METLIBS_LOG_DEBUG("station '" << item->text() << "' toggled");
      checkChildren(parent);
      checkChildren(grandparent);
    }
  }
  mIsInToggle = false;
  enableButtons();
}
