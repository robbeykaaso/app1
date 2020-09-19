#ifndef TCPSOCKET_H
#define TCPSOCKET_H

#include <QTcpSocket>

class TcpSocket : public QTcpSocket
{
    Q_OBJECT
public:
    explicit TcpSocket(QTcpSocket *parent = nullptr);
    TcpSocket(int port, QString serIp);
    virtual ~TcpSocket();

    void setServerIp(const QString &serverIp);
    void setClientName(const QString &clientName); // 客户端的名字用配置文件中ipList的key来区分
    void connectServer();
    void writeMsg(const QString& msg);
    bool getCurConnFlag() const;
    QString getCurStaion();
    void setPort(int port);

signals:
    void sigReadReady(QString clientName, QString msg);
    void sigSocketDisconnect(QString clientName, int description);
    void sigError(QString clientName, QString error);
    void sigStateChanged(QString clientName, QString curState);
    void sigWriteMsg(const QString& msg); // 自己的信号绑定自己的槽函数，让槽函数在子线程中执行

private:
    TcpSocket(const TcpSocket& socket);
    TcpSocket& operator=(const TcpSocket& socket);

    void connectSignalAndSlot();
    void setCurConnFlag(bool curConnFlag);
    void setCurStation(const QString& curStation);
    QString curConnState() const; // 记录客户端最终的连接状态，是否正常连接上了服务器
    void setCurConnState(const QString &curConnState);
    int port() const;
    QString clientName() const; // 客户端的名字用来和socket绑定，用于服务器发送sokcet时识别发给谁的

private slots:
    void onError(QAbstractSocket::SocketError socketError);
    void onStateChanged(QAbstractSocket::SocketState socketState);
    void onReadMessage();
    void onDisconnected();

private:
    int mPort;
    QString mServerIp;
    QString mClientName;
    QString mCurConnState;
    bool mCurConnFlag;
    QString mCurStation;
};

#endif // TCPSOCKET_H
