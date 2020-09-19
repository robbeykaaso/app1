#include "tcpsocket.h"
#include <QThread>
#include "logcat.h"

TcpSocket::TcpSocket(QTcpSocket *parent) : QTcpSocket(parent)
  , mPort(11111)
  , mServerIp("")
  , mClientName("")
  , mCurConnState("")
  , mCurConnFlag(false)
{
    mDebug("main threadID = %x", QThread::currentThreadId());
    connectSignalAndSlot();
}

TcpSocket::TcpSocket(int port, QString serIp): QTcpSocket()
  , mPort(port)
  , mServerIp(serIp)
  , mClientName("")
  , mCurConnState("")
  , mCurConnFlag(false)
{
    mDebug("~");
    connectSignalAndSlot();
}

TcpSocket::~TcpSocket()
{
    mDebug("will disconnectFromHost");
    if (mCurConnFlag)
    {
        disconnectFromHost();
    }
}

void TcpSocket::connectSignalAndSlot()
{
    connect(this, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(onError(QAbstractSocket::SocketError)));
    connect(this, &TcpSocket::stateChanged, this, &TcpSocket::onStateChanged);
    connect(this, &TcpSocket::readyRead, this, &TcpSocket::onReadMessage);
    connect(this, &TcpSocket::disconnected, this, &TcpSocket::onDisconnected);
}

TcpSocket::TcpSocket(const TcpSocket &socket)
{
    (void)socket;
}

TcpSocket& TcpSocket::operator=(const TcpSocket &socket)
{
    (void)socket;
    return *this;
}

void TcpSocket::onError(QAbstractSocket::SocketError socketError)
{
    mDebug("socketError = %d", socketError);
    QString err = this->errorString();
    mDebug("err = %s", err.toLatin1().data());
    emit sigError(mClientName, err);
}

void TcpSocket::onStateChanged(QAbstractSocket::SocketState socketState)
{
    mDebug("socketState = %d", socketState);
    QString state;
    switch (socketState) {
    case QAbstractSocket::UnconnectedState:
        state = "unconnectedState";
        break;
    case QAbstractSocket::HostLookupState:
        state = "hostLookupState";
        break;
    case QAbstractSocket::ConnectingState:
        state = "connectingState";
        break;
    case QAbstractSocket::ConnectedState:
        state = "connectedState";
        break;
    case QAbstractSocket::BoundState:
        state = "boundState";
        break;
    case QAbstractSocket::ClosingState:
        state = "closingState";
        break;
    case QAbstractSocket::ListeningState:
        state = "listeningState";
        break;
    }
    mDebug("state = %s", state.toLatin1().data());
    if (state != mCurConnState)
    {
        mCurConnState = state;
        if (0 == QString::compare(mCurConnState, "connectedState", Qt::CaseInsensitive))
        {
            mCurConnFlag = true;
        }
        else
        {
            mCurConnFlag = false;
        }
        emit sigStateChanged(mClientName, state);
        mDebug("state changed will set mCurConnState = %s", mCurConnState.toLatin1().data());
    }
}

void TcpSocket::onReadMessage()
{
    QByteArray msg = this->readAll();
    mDebug("msg = %s threadId = %x", msg.data(), QThread::currentThreadId());
    emit sigReadReady(mClientName, msg);
}

void TcpSocket::onDisconnected()
{
    mDebug("~");
    emit sigSocketDisconnect(mClientName, static_cast<int>(this->socketDescriptor()));
}

void TcpSocket::setCurConnFlag(bool curConnFlag)
{
    mCurConnFlag = curConnFlag;
}

void TcpSocket::setCurStation(const QString &curStation)
{
    if (mCurStation != curStation)
    {
        mCurStation = curStation;
        mDebug("set curStaion = %s", mCurStation.toLatin1().data());
    }
}

QString TcpSocket::getCurStaion()
{
    return mCurStation;
}

bool TcpSocket::getCurConnFlag() const
{
    mDebug("mCurConnFlag = %d", mCurConnFlag);
    return mCurConnFlag;
}

QString TcpSocket::curConnState() const
{
    return mCurConnState;
}

void TcpSocket::setCurConnState(const QString &curConnState)
{
    mCurConnState = curConnState;
}

void TcpSocket::writeMsg(const QString& msg)
{
    if (mCurConnFlag)
    {
        qDebug()<<"writeMsg threadID = "<<QThread::currentThreadId();
        this->write(msg.toLatin1());
        waitForBytesWritten();
        this->flush();
    }
    else
    {
        mDebug("is not connected will not write socket");
    }
}

void TcpSocket::connectServer()
{
    if (mCurConnFlag)
    {
        mDebug("already connect do nothing");
        return;
    }
    this->connectToHost(mServerIp, static_cast<quint16>(mPort));
    mDebug("will connectToHost %s", mServerIp.toLatin1().data());
}

void TcpSocket::setServerIp(const QString &serverIp)
{
    mServerIp = serverIp;
    mDebug("mServerIp = %s", mServerIp.toLatin1().data());
}

int TcpSocket::port() const
{
    mDebug("mPort = %d", mPort);
    return mPort;
}

void TcpSocket::setPort(int port)
{
    mPort = port;
    mDebug("mPort = %d", mPort);
}

QString TcpSocket::clientName() const
{
    mDebug("mClientName = %s", mClientName.toLatin1().data());
    return mClientName;
}

void TcpSocket::setClientName(const QString &clientName)
{
    mClientName = clientName;
    mDebug("mClientName = %s", mClientName.toLatin1().data());
}
