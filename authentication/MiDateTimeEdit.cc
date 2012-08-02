
#include "MiDateTimeEdit.hh"

void MiDateTimeEdit::stepBy( int steps )
{
    const Section section = currentSection();
    int st = 0;
    if(section == YearSection) {
        setDateTime(dateTime().addYears(steps));
    } else if(section == MonthSection) {
        setDateTime(dateTime().addMonths(steps));
    } else if(section == DaySection) {
        setDateTime(dateTime().addDays(steps));
    } else if(section == HourSection) {
        setDateTime(dateTime().addSecs(3600*steps));
    } else if(section == MinuteSection) {
        setDateTime(dateTime().addSecs(60*steps));
    } else if(section == SecondSection) {
        setDateTime(dateTime().addSecs(steps));
    } else if(section == MSecSection) {
        setDateTime(dateTime().addMSecs(steps));
    } else {
        st = steps;
    }
    QDateTimeEdit::stepBy(st);
}

QAbstractSpinBox::StepEnabled MiDateTimeEdit::stepEnabled() const
{
    // very much inspired by original QDateTimeEdit::stepEnabled()
    bool not_min, not_max;
    if(!(displayedSections() & DateSections_Mask)) {
        // time only, no date
        const QTime t = time();
        not_min = ( t != minimumTime() );
        not_max = ( t != maximumTime() );
    } else if (!(displayedSections() & TimeSections_Mask)) {
        // date only, no time
        QDate d = date();
        not_max = ( d != maximumDate() );
        not_min = ( d != minimumDate() );
    } else {
        // both
        QDateTime dt = dateTime();
        not_max = ( dt != maximumDateTime() );
        not_min = ( dt != minimumDateTime() );
    }
    
    QAbstractSpinBox::StepEnabled ret = 0;
    if (not_min)
        ret |= QAbstractSpinBox::StepDownEnabled;
    if (not_max)
        ret |= QAbstractSpinBox::StepUpEnabled;
    return ret;
}
