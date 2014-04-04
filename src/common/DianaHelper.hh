
#ifndef DIANAHELPER_HH
#define DIANAHELPER_HH 1

#include "TimeSpan.hh"

#include <QtCore/QObject>

#include <string>
#include <vector>

class ClientButton;
class miMessage;

QT_BEGIN_NAMESPACE
class QImage;
QT_END_NAMESPACE

class DianaHelper : public QObject
{ Q_OBJECT;
public:
  DianaHelper(ClientButton* cb);

  void tryConnect();
  void sendStations(const std::vector<int>& stations);
  void sendTimes(const std::vector<timeutil::ptime>& times);
  void sendTime(const timeutil::ptime& time);
  void sendImage(const std::string& name, const QImage& image);
    
Q_SIGNALS:
  void connection(bool);
  void receivedTime(const timeutil::ptime& time);

private Q_SLOTS:
  void processConnect();
  void cleanConnection();
  void processLetter(const miMessage& m);

private:
  ClientButton* mDianaButton;
  bool mConnected;
  timeutil::ptime mDianaTime;
};

#endif /* DIANAHELPER_HH */
