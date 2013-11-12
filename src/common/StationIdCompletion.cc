
#include "StationIdCompletion.hh"

#include "StationIdModel.hh"

#include <QtGui/QCompleter>
#include <QtGui/QHeaderView>
#include <QtGui/QLineEdit>
#include <QtGui/QTableView>
#include <QtGui/QValidator>

namespace Helpers {

void installStationIdCompleter(QObject* parent, QLineEdit* editStation, StationIdModel* cmodel)
{
  QCompleter *completer = new QCompleter(parent);

  QTableView* completionPopup = new QTableView(editStation);
  completionPopup->horizontalHeader()->setVisible(false);
  completionPopup->verticalHeader()->setVisible(false);
  completionPopup->verticalHeader()->setDefaultSectionSize(20);
  completionPopup->horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents);
  completer->setPopup(completionPopup);

  completer->setModel(cmodel);

  completer->setCompletionColumn(0);
  completer->setModelSorting(QCompleter::CaseInsensitivelySortedModel);
  completer->setCaseSensitivity(Qt::CaseInsensitive);
  editStation->setCompleter(completer);

  QValidator *validator = new QIntValidator(cmodel->minStationId(), cmodel->maxStationId(), parent);
  editStation->setValidator(validator);
} // namespace anonymous


void installStationIdCompleter(QObject* parent, QLineEdit* editStation)
{
  installStationIdCompleter(parent, editStation, new StationIdModel(parent));
}

} // namespace Helpers
