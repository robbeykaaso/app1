#include "tcpserver.h"
#include "tcpsocket.h"
#include <QHostAddress>
#include <QThread>
#include <QMutex>
#include <QMutexLocker>
#include <QJsonObject>
#include "logcat.h"

static QMutex gMutex;
TcpServer* TcpServer::mInstance = nullptr;

TcpServer *TcpServer::getInstance()
{
    if (mInstance)
    {
        return mInstance;
    }
    QMutexLocker lg(&gMutex);
    if (!mInstance)
    {
        static TcpServer tcpServer;
        mInstance = &tcpServer;
    }
    return mInstance;
}

TcpServer::TcpServer(QTcpServer *parent) : QTcpServer(parent)
  , mPort(11111)
  , mThread(nullptr)
  , mCurStation("")
{
    listenPort();
}

TcpServer &TcpServer::operator=(const TcpServer& ser)
{
    (void)ser;
    return *this;
}

TcpServer::TcpServer(const TcpServer &server)
{
    (void)server;
}

void TcpServer::connectSignalSlot(TcpSocket* socket)
{
    if (socket)
    {
        connect(socket, &TcpSocket::sigWriteMsg, socket, &TcpSocket::writeMsg);
        if (mThread)
        {
            connect(mThread, &QThread::finished, socket, &TcpSocket::deleteLater);
        }
        connect(socket, &TcpSocket::sigReadReady, this, &TcpServer::sigReadFromSocket);
        connect(socket, &TcpSocket::sigSocketDisconnect, this, &TcpServer::onClientDisconnect);
    }
}

void TcpServer::tryInsertSocketHash(QString ip, TcpSocket *socket)
{
    if (socket)
    {
        mSocketHash.insert(ip, socket);
        qDebug()<<"ip = "<< ip <<" hash insert success";
    }
}

void TcpServer::onClientDisconnect(QString clientName, int descriptor)
{
    (void)clientName;
    TcpSocket *senderSocket = nullptr;
    QList<QString>tmpList = mSocketHash.keys();
    QString ip;
    bool flag = false;
    for (int i=0; i<tmpList.count(); ++i)
    {
        ip = tmpList.at(i);
        senderSocket = mSocketHash.value(ip);
        if (senderSocket && senderSocket->socketDescriptor() == descriptor)
        {
            mSocketHash.remove(ip);
            updateClientConnStatus(mIpAliasHash.key(ip), "unConnected");
            qDebug()<<"remove ip = "<<ip;
            senderSocket->deleteLater();
            flag = true;
            break;
        }
    }
    if (!flag)
    {
        qDebug()<<"not found disconnect socket";
    }
}

TcpSocket *TcpServer::findClient(QString ipAlias)
{
    QList<QString>ipList = mSocketHash.keys();
    QString ip = mIpAliasHash.value(ipAlias);
    for (int i=0; i<ipList.count(); ++i)
    {
        if (ip == ipList.at(i))
        {
            qDebug()<<"find client ip = "<<ip;
            return mSocketHash.value(ip);
        }
    }
    qDebug()<<"not find client return nullptr";
    return nullptr;
}

void TcpServer::setClientNameIp(TcpSocket* tcpSocket, QString ip)
{
    qDebug()<<Q_FUNC_INFO;
    QList<QString>ipList = mIpAliasHash.keys();
    if (tcpSocket)
    {
        for (int i=0; i<ipList.count(); ++i)
        {
            if (ip == mIpAliasHash.value(ipList.at(i)))
            {
                QString clientName = ipList.at(i);
                qDebug()<<"setClientName = "<<clientName;
                tcpSocket->setClientName(clientName);
                updateClientConnStatus(clientName, "connected");
                break;
            }
        }
    }
}

QString TcpServer::getClientIpByName(QString name)
{
    QString clientIp = mIpAliasHash.value(name);
    qDebug()<<"clientIp = "<< clientIp;
    return clientIp;
}

QString TcpServer::getClientStatusByName(QString name)
{
    QString clientStatus = mClientConnStatusHash.value(name);
    qDebug()<<"clientStatus = "<< clientStatus;
    return clientStatus;
}

void TcpServer::incomingConnection(qintptr handle)
{
    TcpSocket* tmpSocket = new(std::nothrow)TcpSocket;
    if (!mThread)
    {
        mThread = new(std::nothrow)QThread;
    }
    if (tmpSocket)
    {
        connectSignalSlot(tmpSocket);
        tmpSocket->setSocketDescriptor(handle);
        QString ipAddr = tmpSocket->peerAddress().toString();
        qDebug()<<Q_FUNC_INFO<<"lines = "<<__LINE__<<"new connect ipAddr = "<<ipAddr;
        QString ip = ipAddr.mid(ipAddr.lastIndexOf(":") + 1);
        tryInsertSocketHash(ip, tmpSocket);
        setClientNameIp(tmpSocket, ip);

        if (mThread)
        {
            tmpSocket->moveToThread(mThread);
            mThread->start();
        }
    }
}

void TcpServer::listenPort()
{
    bool tmp = listen(QHostAddress::Any, static_cast<quint16>(mPort));
    qDebug()<<"listen "<<tmp;
    if (!tmp)
    {
        qDebug()<<"listen errorString = "<<errorString();
    }
}

void TcpServer::setDefaultClientConnStatus(const QString &clientName, const QString &status)
{
    qDebug()<<"clientName = "<<clientName<<" status = "<<status;
    mClientConnStatusHash.insert(clientName, status);
}

void TcpServer::setIpAlias(const QString &key, const QString &value)
{
    mIpAliasHash.insert(key, value); // alias ip
    qDebug()<<"key = "<<key<<" value = "<<value;
}

QString TcpServer::getCurStation() const
{
    qDebug()<<"mCurStation = "<<mCurStation;
    return mCurStation;
}

void TcpServer::setPort(int port)
{
    if (mPort != port)
    {
        mPort = port;
        listenPort();
    }
}

int TcpServer::getPort()
{
    return mPort;
}

void TcpServer::writeMsgtoClient(const QString& writeWho, const QString& content)
{
    TcpSocket* tmpSocket = findClient(writeWho);
    if (tmpSocket)
    {
        tmpSocket->sigWriteMsg(content); // 发个信号让对应的socket写
    }
}

void TcpServer::setCurStation(const QString &stationName)
{
    if (mCurStation != stationName)
    {
        mCurStation = stationName;
        mDebug("set mCurStation = %s", mCurStation.toLatin1().data());
    }
}

void TcpServer::updateClientConnStatus(const QString &clientName, const QString &status)
{
    mClientConnStatusHash.insert(clientName, status);
    qDebug()<<"updateClientConnStatus "<<clientName<<" "<<status;
    emit sigConnStatusChanged(clientName, status);
}

TcpServer::~TcpServer()
{
    qDebug()<<Q_FUNC_INFO;
    if (mThread)
    {
        mThread->requestInterruption();
        mThread->exit(0);
        bool flag = mThread->wait(100);

        qDebug()<<Q_FUNC_INFO<<"after wait(1000) flag = "<<flag;
        delete mThread;
        mThread = nullptr;
    }
    QList<QString>ipList = mSocketHash.keys();
    TcpSocket* tmpSocket = nullptr;
    for (int i=0; i<ipList.count(); ++i)
    {
        tmpSocket = mSocketHash.value(ipList.at(i));
        if (tmpSocket)
        {
            tmpSocket->disconnect();
            delete tmpSocket;
            tmpSocket = nullptr;
            qDebug()<<Q_FUNC_INFO<<"delete tmpSocket";
        }
    }
}
