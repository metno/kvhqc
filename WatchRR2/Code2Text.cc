
#include "Code2Text.hh"

#include "Helpers.hh"

Code2Text::Code2Text()
    : mMinValue(-100)
    , mMaxValue(5000)
{
    addCode(kvalobs::MISSING,  (QStringList() << "mis" << "m"), "value is missing");
    addCode(kvalobs::REJECTED, (QStringList() << "rej" << "r"), "value is rejected");
}

Code2Text::~Code2Text()
{
}

QString Code2Text::asTip(float value)
{
    QMap<int,Code>::const_iterator it = mCodes.find(value);
    if (it == mCodes.end())
        return "";
    return it.value().explain;
}

QString Code2Text::asText(float value)
{
    QMap<int,Code>::const_iterator it = mCodes.find(value);
    if (it == mCodes.end())
        return kvalobs::formatValue(value);

    return it.value().shortText.front();
}

float Code2Text::fromText(const QString& text)
{
    QMap<int, Code>::const_iterator it = mCodes.constBegin();
    ++it; // do not allow setting "mis"
    for (; it != mCodes.constEnd(); ++it) {
        const Code& c = it.value();
        if( c.shortText.contains(text) )
            return it.key();
    }

    bool numOk = false;
    const float num = text.toFloat(&numOk);
    if (not numOk or num < mMinValue or num > mMaxValue)
        throw "bad value";
    return num;
}

void Code2Text::addCode(int value, const QStringList& shortText, const QString& explain)
{
    mCodes.insert(value, Code(shortText, explain));
}

void Code2Text::setRange(float mini, float maxi)
{
    mMinValue = mini;
    mMaxValue = maxi;
}
