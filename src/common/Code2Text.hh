
#ifndef Code2Text_hh
#define Code2Text_hh 1

#include <QMap>
#include <QStringList>
#include <memory>

class Code2Text {
public:
  Code2Text();
  virtual ~Code2Text();
  virtual QString asTip(float value) const;
  virtual QString asText(float value, bool editing=false) const;
  virtual bool isCode(float value) const;
  virtual float fromText(const QString& text) const;

  virtual void addCode(int value, const QStringList& shortText, const QString& explain);
  void setDecimals(int d)
    { mDecimals = d; }
  QStringList allCodes() const;
  QStringList allExplanations() const;

private:
  float mDecimals;

  struct Code {
    QStringList shortText;
    QString explain;
    Code(const QStringList& st, const QString& e)
      : shortText(st), explain(e) { }
  };
  QMap<int,Code> mCodes;

};
typedef std::shared_ptr<Code2Text> Code2TextPtr;
typedef std::shared_ptr<const Code2Text> Code2TextCPtr;

#endif // Code2Text_hh
