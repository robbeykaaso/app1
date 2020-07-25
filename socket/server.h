#ifndef REAL_FRAMEWORK_SERVER_H_
#define REAL_FRAMEWORK_SERVER_H_

#include <QObject>
#include <QJsonObject>
#include <QTcpServer>

class clientMessage : public QJsonObject{
public:
    clientMessage(){}
    clientMessage(const QJsonObject& aData) : QJsonObject(aData){}
    QTcpSocket* client_socket;
};

class normalServer : public QObject{
    Q_OBJECT
public:
    normalServer(const QJsonObject& aConfig = QJsonObject());
    ~normalServer();
public slots:
    void NewConnect();
    void DisConnected();
    void ReadMessage();
private:
    QTcpServer socket_;
    QHash<QTcpSocket*, QString> m_clients;
};

#endif
