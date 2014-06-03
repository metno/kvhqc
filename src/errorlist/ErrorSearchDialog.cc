/*
  HQC - Free Software for Manual Quality Control of Meteorological Observations

  Copyright (C) 2013-2014 met.no

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
#include "ErrorSearchDialog.hh"

#include "common/FindAllParameters.hh"
#include "common/KvMetaDataBuffer.hh"
#include "common/StInfoSysBuffer.hh"
#include "common/HqcApplication.hh"
#include "common/TimeSpanControl.hh"
#include "util/timeutil.hh"

#include <QtCore/QSettings>
#include <QtGui/QInputDialog>
#include <QtGui/QMessageBox>
#include <QtGui/QSortFilterProxyModel>
#include <QtGui/QStandardItem>
#include <QtGui/QStandardItemModel>
#include <QtSql/QSqlQuery>

#include <boost/foreach.hpp>

#include <algorithm>

#include "ui_error_search_dialog.h"

#define MILOGGER_CATEGORY "kvhqc.ErrorSearchDialog"
#include "util/HqcLogging.hh"

namespace /* anonymous */ {

enum { C_ITEM_COUNTY_DB = Qt::UserRole + 1 };

const char QSETTINGS_GROUP[] = "errorlist_search";

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
  int nChecked = 0, nPartial = 0;
  for (int c=0; c<cc; ++c) {
    QStandardItem* child = parent->child(c, 0);
    const Qt::CheckState ccs = child->checkState();
    if (ccs == Qt::Checked)
      nChecked += 1;
    else if (ccs == Qt::PartiallyChecked)
      nPartial += 1;
  }
  const Qt::CheckState pcs = parent->checkState();
  const Qt::CheckState pcs_new = (nChecked == cc) ? Qt::Checked
      : (((nChecked + nPartial) > 0) ? Qt::PartiallyChecked : Qt::Unchecked);
  if (pcs != pcs_new)
    parent->setCheckState(pcs_new);
}

class StationFilterProxyModel : public QSortFilterProxyModel
{
public:
  StationFilterProxyModel(QObject* parent=0)
    : QSortFilterProxyModel(parent) { }
  
protected:
  bool filterAcceptsRow(int row, const QModelIndex &parent) const;
};

bool StationFilterProxyModel::filterAcceptsRow(int row, const QModelIndex &parent) const
{
  // accept highest level (region)
  if (not parent.isValid())
    return true;
  // accept next-highest level (county)
  if (not parent.parent().isValid())
    return true;

  const QRegExp& filter = filterRegExp();
  const QString stationid = sourceModel()->data(parent.child(row, 0)).toString();
  if (filter.indexIn(stationid) == 0) // match station id at start
    return true;
  const QString stationname = sourceModel()->data(parent.child(row, 1)).toString();
  if (filter.indexIn(stationname) >= 0) // match station name anywhere
    return true;
  // do not match other columns
  return false;
}
} // anonymous namespace

// ========================================================================

ErrorSearchDialog::ErrorSearchDialog(QWidget* parent)
  : QDialog(parent)
  , ui(new Ui::ErrorSearchDialog)
  , mTimeControl(new TimeSpanControl(this))
  , mIsInToggle(false)
{
  ui->setupUi(this);

  setupStationTab();
  setupParameterTab();

  connect(ui->buttonSave,    SIGNAL(clicked()), this, SLOT(onSaveSettings()));
  connect(ui->buttonRestore, SIGNAL(clicked()), this, SLOT(onRestoreSettings()));
    
  mTimeControl->setMinimumGap(6);
  mTimeControl->install(ui->fromTime, ui->toTime);
    
  enableButtons();
}

ErrorSearchDialog::~ErrorSearchDialog()
{
}

void ErrorSearchDialog::changeEvent(QEvent *event)
{
  if (event->type() == QEvent::LanguageChange) {
    ui->retranslateUi(this);

    mStationModel->setHorizontalHeaderLabels(QStringList()
        << tr("Station/Region") << tr("Name") << tr("HOH")
        << tr("County") << tr("Commune") << tr("Pri"));

    // from ErrorSearchDialog::setupParameterTab()
    const int npgs = ui->comboParamGroup->count();
    ui->comboParamGroup->setItemText(npgs-2, tr("All in obs pgm"));
    ui->comboParamGroup->setItemText(npgs-1, tr("All defined"));
  }
  QDialog::changeEvent(event);
}

void ErrorSearchDialog::setupStationTab()
{
  METLIBS_LOG_SCOPE();
  const listStat_l& listStat = StationInfoBuffer::instance()->getStationDetails();
    
  mStationModel.reset(new QStandardItemModel(this));
  QStandardItem *root = mStationModel->invisibleRootItem();

  mStationModel->setHorizontalHeaderLabels(QStringList()
      << tr("Station/Region") << tr("Name") << tr("HOH")
      << tr("County") << tr("Commune") << tr("Pri"));

  typedef std::map<QString, QStandardItem*> county2item_t;
  county2item_t county2item;
  
  QSqlQuery queryRegions(hqcApp->systemDB());
  queryRegions.prepare("SELECT sr.id, srl.label FROM station_regions AS sr, station_region_labels AS srl"
      " WHERE sr.id = srl.region_id AND srl.language = 'nb' ORDER BY sr.sortkey");
    
  QSqlQuery queryCounties(hqcApp->systemDB());
  queryCounties.prepare("SELECT sc.db_name, scl.label FROM station_county_labels AS scl, station_counties AS sc"
      " WHERE sc.id = scl.county_id AND scl.language = 'nb' AND sc.region_id = ? ORDER BY sc.sortkey");
    
  queryRegions.exec();
  while (queryRegions.next()) {
    const int regionId = queryRegions.value(0).toInt();
    const QString regionLabel = queryRegions.value(1).toString();

    QStandardItem *r_item = new QStandardItem(regionLabel);
    r_item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsTristate);
    r_item->setCheckable(true);
    root->appendRow(r_item);

    queryCounties.bindValue(0, regionId);
    queryCounties.exec();
    while (queryCounties.next()) {
      const QString countyDB = queryCounties.value(0).toString();
      const QString countyLabel = queryCounties.value(1).toString();

      QStandardItem *c_item = new QStandardItem(countyLabel);
      c_item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsTristate);
      c_item->setCheckable(true);
      c_item->setData(countyDB, C_ITEM_COUNTY_DB);

      r_item->appendRow(c_item);
      county2item.insert(std::make_pair(countyDB, c_item));
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

    county2item_t::iterator c_it = county2item.find(cu);
    if (c_it != county2item.end()) {
      c_it->second->appendRow(s_items);
    } else {
      HQC_LOG_WARN("no county '" << s.fylke << "' for station " << s.stationid);
    }
  }

  StationFilterProxyModel *proxyModel = new StationFilterProxyModel(this);
  proxyModel->setSourceModel(mStationModel.get());
  proxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
  ui->treeStations->setModel(proxyModel);

  ui->treeStations->expandAll();
  ui->treeStations->header()->resizeSections(QHeaderView::ResizeToContents);
  ui->treeStations->header()->setResizeMode(QHeaderView::Interactive);
  ui->treeStations->collapseAll();

  ui->treeStations->setSelectionMode(QAbstractItemView::NoSelection);

  connect(mStationModel.get(), SIGNAL(itemChanged(QStandardItem*)),
      this, SLOT(onItemChanged(QStandardItem*)));
  
  if (not ui->checkRememberTimes->isChecked())
    onSetRecentTimes();
}

void ErrorSearchDialog::onFilterStations(const QString& text)
{
  if (not text.isEmpty())
    ui->treeStations->expandAll();
  StationFilterProxyModel* proxyModel = static_cast<StationFilterProxyModel*>(ui->treeStations->model());
  proxyModel->setFilterFixedString(text);
}

void ErrorSearchDialog::setupParameterTab()
{
  METLIBS_LOG_SCOPE();

  mParamSelectedModel.reset(new ParamIdModel);
  ui->listParamChosen->setModel(mParamSelectedModel.get());
  mParamAvailableModel.reset(new ParamIdModel);
  ui->listParamAvailable->setModel(mParamAvailableModel.get());

  connect(ui->listParamChosen,    SIGNAL(doubleClicked(const QModelIndex&)), this, SLOT(delParameter2Click(const QModelIndex&)));
  connect(ui->listParamAvailable, SIGNAL(doubleClicked(const QModelIndex&)), this, SLOT(addParameter2Click(const QModelIndex&)));

  connect(ui->buttonParamSelect, SIGNAL(clicked()), this, SLOT(selectParameters()));
  connect(ui->buttonParamDeselect, SIGNAL(clicked()), this, SLOT(deselectParameters()));
  connect(ui->buttonParamSelectAll, SIGNAL(clicked()), this, SLOT(selectAllParameters()));
  connect(ui->buttonParamDeselectAll, SIGNAL(clicked()), this, SLOT(deselectAllParameters()));
  connect(ui->comboParamGroup, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(showParamGroup(const QString&)));

  QStringList labels;
  mParameterGroups.clear();

  QSqlQuery queryGroups(hqcApp->systemDB());
  queryGroups.prepare("SELECT pg.id, pgl.label FROM param_groups AS pg, param_group_labels AS pgl"
      " WHERE pg.id = pgl.group_id AND pgl.language = 'nb' ORDER BY pg.sortkey");
    
  QSqlQuery queryParams(hqcApp->systemDB());
  queryParams.prepare("SELECT paramid FROM param_order WHERE group_id = ? ORDER BY sortkey");
    
  queryGroups.exec();
  while (queryGroups.next()) {
    const int groupId = queryGroups.value(0).toInt();
    const QString groupLabel = queryGroups.value(1).toString();
    METLIBS_LOG_DEBUG(LOGVAL(groupLabel));

    labels << groupLabel;
    std::vector<int>& parameters = mParameterGroups[groupLabel];
            
    queryParams.bindValue(0, groupId);
    queryParams.exec();
    while (queryParams.next())
      parameters.push_back(queryParams.value(0).toInt());
  }

  try {
    const QString labelAll = tr("All in obs pgm");
    labels << labelAll;
    mParameterGroups.insert(std::make_pair(labelAll, Helpers::findAllParameters(true)));
  } catch (std::exception& ex) {
    HQC_LOG_WARN("failed to generate list of all parameters from kvalobs.obs_pgm table");
  }

  try {
    const std::list<kvalobs::kvParam>& allParams = KvMetaDataBuffer::instance()->allParams();
    const QString labelAll = tr("All defined");
    labels << labelAll;
    std::vector<int>& parameters = mParameterGroups[labelAll];

    BOOST_FOREACH(const kvalobs::kvParam& p, allParams)
        parameters.push_back(p.paramID());
  } catch (std::exception& ex) {
    HQC_LOG_WARN("failed to generate list of all parameters from kvalobs.param table");
  }

  ui->comboParamGroup->addItems(labels);
  ui->comboParamGroup->setCurrentIndex(0);
}

void ErrorSearchDialog::saveSettings(QSettings& settings)
{
  settings.beginGroup(QSETTINGS_GROUP);
  doSaveSettings(settings);
  settings.endGroup();
}

void ErrorSearchDialog::restoreSettings(QSettings& settings)
{
  settings.beginGroup(QSETTINGS_GROUP);
  doRestoreSettings(settings);
  settings.endGroup();
}

void ErrorSearchDialog::doSaveSettings(QSettings& settings)
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

void ErrorSearchDialog::doRestoreSettings(QSettings& settings)
{
  {
    const QStringList parameters = settings.value("selected_parameters").toStringList();
    std::vector<int> params;
    BOOST_FOREACH(const QString& p, parameters)
        params.push_back(p.toInt());

    mParamSelectedModel->setValues(params);
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

void ErrorSearchDialog::onSaveSettings()
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

void ErrorSearchDialog::onRestoreSettings()
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

void ErrorSearchDialog::onSetRecentTimes()
{
  QDateTime t = timeutil::nowWithMinutes0Seconds0();
  QDateTime f = t.addSecs(-2*24*3600 + 3600*(17-t.time().hour()) + 60*45);
  ui->fromTime->setDateTime(f);
  ui->toTime->setDateTime(t);
}

void ErrorSearchDialog::showParamGroup(const QString& paramGroup)
{
  const std::vector<int>& sel = mParamSelectedModel->values();
  std::set<int> sel_set(sel.begin(), sel.end());

  const std::vector<int>& group = mParameterGroups[paramGroup];
  std::vector<int> avail;
  BOOST_FOREACH(int pid, group) {
    if (sel_set.find(pid) == sel_set.end())
      avail.push_back(pid);
  }
  mParamAvailableModel->setValues(avail);
  enableButtons();
}

void ErrorSearchDialog::addParameter2Click(const QModelIndex& index)
{
  const std::vector<int>& add = mParamAvailableModel->values();
  const int paramId = add.at(index.row());

  const std::vector<int>& sel = mParamSelectedModel->values();
  if (std::find(sel.begin(), sel.end(), paramId) == sel.end()) {
    std::vector<int> sel2(sel);
    sel2.push_back(paramId);
    mParamSelectedModel->setValues(sel2);
  }
  showParamGroup(ui->comboParamGroup->currentText());
}

void ErrorSearchDialog::delParameter2Click(const QModelIndex& index)
{
  std::vector<int> sel = mParamSelectedModel->values();
  sel.erase(sel.begin() + index.row());
  mParamSelectedModel->setValues(sel);
  showParamGroup(ui->comboParamGroup->currentText());
}

void ErrorSearchDialog::selectParameters()
{
  const std::vector<int>& add = mParamAvailableModel->values();

  std::vector<int> sel = mParamSelectedModel->values();
  std::set<int> sel_set(sel.begin(), sel.end());

  const QItemSelectionModel* selection = ui->listParamAvailable->selectionModel();
  for (size_t i=0; i<add.size(); ++i) {
    if (sel_set.find(add[i]) == sel_set.end() and selection->isRowSelected(i, QModelIndex()))
      sel.push_back(add[i]);
  }

  mParamSelectedModel->setValues(sel);
  showParamGroup(ui->comboParamGroup->currentText());
}

void ErrorSearchDialog::selectAllParameters()
{
  const std::vector<int>& add = mParamAvailableModel->values();

  std::vector<int> sel = mParamSelectedModel->values();
  std::set<int> sel_set(sel.begin(), sel.end());

  for (size_t i=0; i<add.size(); ++i) {
    if (sel_set.find(add[i]) == sel_set.end())
      sel.push_back(add[i]);
  }

  mParamSelectedModel->setValues(sel);
  showParamGroup(ui->comboParamGroup->currentText());
}

void ErrorSearchDialog::deselectParameters()
{
  const std::vector<int>& sel = mParamSelectedModel->values();
  std::vector<int> new_sel;
  const QItemSelectionModel* selection = ui->listParamChosen->selectionModel();
  for (size_t i=0; i<sel.size(); ++i) {
    if (not selection->isRowSelected(i, QModelIndex()))
      new_sel.push_back(sel[i]);
  }

  mParamSelectedModel->setValues(new_sel);
  showParamGroup(ui->comboParamGroup->currentText());
}

void ErrorSearchDialog::deselectAllParameters()
{
  mParamSelectedModel->setValues(std::vector<int>());
  showParamGroup(ui->comboParamGroup->currentText());
}

TimeSpan ErrorSearchDialog::getTimeSpan() const
{
  return mTimeControl->timeRange();
}

void ErrorSearchDialog::enableButtons()
{
  const bool haveStations   = not getSelectedStations()  .empty();
  const bool haveParameters = not getSelectedParameters().empty();

  const bool allowApply = haveStations and haveParameters;
  ui->buttonOk->setEnabled(allowApply);
}

QStringList ErrorSearchDialog::getSelectedCounties()
{
  METLIBS_LOG_SCOPE();

  QStringList t;

  QStandardItem* root = mStationModel->invisibleRootItem();
  const int nRegions = root->rowCount();
  for (int r=0; r<nRegions; ++r) {
    QStandardItem* region = root->child(r, 0);
    const int nCounties = region->rowCount();
    for (int c=0; c<nCounties; ++c) {
      QStandardItem* county = region->child(c, 0);
      if (county->checkState() != Qt::Unchecked) {
        METLIBS_LOG_DEBUG("county '" << county->text() << "' in region '" << region->text() << "' is selected");
        const QString countyDB = county->data(C_ITEM_COUNTY_DB).toString();
        t << countyDB;
      }
    }
  }
  
  return t;
}

void ErrorSearchDialog::setSelectedCounties(const QStringList& selectedCounties)
{
  METLIBS_LOG_SCOPE();

  QStandardItem* root = mStationModel->invisibleRootItem();
  const int nRegions = root->rowCount();
  for (int r=0; r<nRegions; ++r) {
    QStandardItem* region = root->child(r, 0);
    const int nCounties = region->rowCount();
    for (int c=0; c<nCounties; ++c) {
      QStandardItem* county = region->child(c, 0);
      METLIBS_LOG_DEBUG("(de)selecting county '" << county->text() << "' in region '" << region->text() << "'");
      const QString countyDB = county->data(C_ITEM_COUNTY_DB).toString();
      county->setCheckState(selectedCounties.contains(countyDB) ? Qt::Checked : Qt::Unchecked);
    }
  }
  enableButtons();
}

std::vector<int> ErrorSearchDialog::getSelectedStations()
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

std::vector<int> ErrorSearchDialog::getSelectedParameters()
{
  return mParamSelectedModel->values();
}

void ErrorSearchDialog::onItemChanged(QStandardItem* item)
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
