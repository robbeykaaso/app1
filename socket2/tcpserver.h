#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <QTcpServer>
class TcpSocket;
class QThread;

class TcpServer : public QTcpServer
{
    Q_OBJECT
public:
    static TcpServer *getInstance();
    virtual ~TcpServer() override;
    void writeMsgtoClient(const QString& writeWho, const QString& content);
    void setCurStation(const QString& stationName);
    QString getCurStation() const;
    void setDefaultClientConnStatus(const QString& clientName, const QString& status = "unConnected");
    void setIpAlias(const QString&key, const QString& value);
    void setPort(int port);

protected:
    virtual void incomingConnection(qintptr handle) override;

private:
    explicit TcpServer(QTcpServer *parent = nullptr);
    TcpServer& operator=(const TcpServer& ser);
    TcpServer(const TcpServer& server);

    void listenPort();
    void connectSignalSlot(TcpSocket* socket);
    void tryInsertSocketHash(QString ip, TcpSocket* socket);
    void onClientDisconnect(QString clientName, int descriptor);
    TcpSocket* findClient(QString ipAlias);
    void setClientNameIp(TcpSocket* tcpSocket, QString ip);

    QString getClientIpByName(QString name);
    QString getClientStatusByName(QString name);
    int getPort();

signals:
    void sigReadFromSocket(QString clientName, QString msg);
    void sigClientDisconnect(QString clientName, QString msg);
    void sigConnStatusChanged(QString clientName, QString msg);

private slots:
    void updateClientConnStatus(const QString& clientName, const QString& status);

private:
    int mPort;
    QThread* mThread;
    QHash<QString, QString> mIpAliasHash;
    QHash<QString, QString> mClientConnStatusHash;
    QHash<QString, TcpSocket*> mSocketHash;
    QString mCurStation;
    static TcpServer* mInstance;
};

#endif // TCPSERVER_H
