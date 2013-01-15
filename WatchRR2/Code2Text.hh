
#ifndef Code2Text_hh
#define Code2Text_hh 1

#include <QtCore/QMap>
#include <QtCore/QStringList>
#include <boost/shared_ptr.hpp>

class Code2Text {
public:
    Code2Text();
    virtual ~Code2Text();
    virtual QString asTip(float value);
    virtual QString asText(float value);
    virtual bool isCode(float value);
    virtual float fromText(const QString& text);

    virtual void addCode(int value, const QStringList& shortText, const QString& explain);
    void setRange(float mini, float maxi);
    void setDecimals(int d)
        { mDecimals = d; }

private:
    float mMinValue;
    float mMaxValue;
    float mDecimals;

    struct Code {
        QStringList shortText;
        QString explain;
        Code(const QStringList& st, const QString& e)
            : shortText(st), explain(e) { }
    };
    QMap<int,Code> mCodes;

};
typedef boost::shared_ptr<Code2Text> Code2TextPtr;

#endif // Code2Text_hh
