
#ifndef MiDateTimeEdit_hh
#define MiDateTimeEdit_hh 1

#include <QtGui/QDateTimeEdit>

/**
 * @brief A QDateTimeEdit with different arrow up/down behavior.
 *
 * For example, if the cursor is positioned in the minute field and
 * the down arrow key is pressed with time 00:00, the time jumps to
 * 23:59 on the previous day.
 */
class MiDateTimeEdit : public QDateTimeEdit
{   Q_OBJECT

public:
    MiDateTimeEdit(QWidget* parent=0)
        : QDateTimeEdit(parent) { }

    MiDateTimeEdit(const QDateTime & datetime, QWidget* parent = 0)
        : QDateTimeEdit(datetime, parent) { }

    MiDateTimeEdit(const QDate& date, QWidget* parent=0)
        : QDateTimeEdit(date, parent) { }

    MiDateTimeEdit(const QTime& time, QWidget* parent=0 )
        : QDateTimeEdit(time, parent) { }

public:
    virtual void stepBy( int steps );
protected:
    virtual StepEnabled stepEnabled() const;
};

#endif
