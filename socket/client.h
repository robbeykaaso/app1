#ifndef REAL_FRAMEWORK_CLIENT_H_
#define REAL_FRAMEWORK_CLIENT_H_

#include "ssdp.h"
#include <QTcpSocket>
#include <QTimer>

class normalClient : public QObject{
    Q_OBJECT
public:
    normalClient(const QJsonObject& aConfig = QJsonObject());
    ~normalClient();
    void RegistOnStateChanged(std::function<void(QAbstractSocket::SocketState)> aEvent);
signals:
    void connected();
public slots:
    void ServerFound(QString aIP, QString aPort, QString aID);
    void ReceiveState(QAbstractSocket::SocketState aState);
    void ReceiveMessage();
private:
    void tryConnectServer();
    QTcpSocket m_socket;
    rea::DiscoveryManager ssdp_;
    bool m_valid = false;
    QTimer search_timer_;
};

#endif
