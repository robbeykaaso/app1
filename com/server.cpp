#include "server.h"
#include "reactive2.h"
#include "protocal.h"
#include <QTcpSocket>
#include <QJsonDocument>

normalServer::normalServer(const QJsonObject& aConfig) : QObject()
{
    //QHostAddress add("127.0.0.1");
    socket_.listen(QHostAddress::LocalHost, 8081);
    connect(&socket_,SIGNAL(newConnection()),this,SLOT(NewConnect()));

    rea::pipeline::add<clientMessage>([this](rea::stream<clientMessage>* aInput){
        auto dt = aInput->data();
        auto client = m_clients.value(dt.client_socket);
        for (auto i : m_clients.keys()){
            if (m_clients.value(i) == client){
                i->write(QJsonDocument(dt).toJson(QJsonDocument::Compact));
                i->flush();
                while (i->bytesToWrite() > 0)
                    std::this_thread::sleep_for(std::chrono::milliseconds(5));
            }
        }
    }, rea::Json("name", "callClient"));

    rea::pipeline::add<clientMessage, rea::pipePartial>([](rea::stream<clientMessage>* aInput){
        aInput->out();
    }, rea::Json("name", "receiveFromClient"));

}

normalServer::~normalServer(){
    m_clients.clear();
}

void normalServer::DisConnected(){
    QTcpSocket* client = static_cast<QTcpSocket*>(QObject::sender());  //https://stackoverflow.com/questions/25339943/how-to-know-if-a-client-connected-to-a-qtcpserver-has-closed-connection
    if (m_clients.contains(client)){
        m_clients.remove(client);
        disconnect(client,SIGNAL(readyRead()),this,SLOT(ReadMessage())); //有可读的信息，触发读函数槽
        disconnect(client, SIGNAL(disconnected()), this, SLOT(DisConnected()));
    }
}

void normalServer::NewConnect()
{
    auto client = socket_.nextPendingConnection(); //得到每个连进来的socket
    connect(client,SIGNAL(readyRead()),this,SLOT(ReadMessage())); //有可读的信息，触发读函数槽
    /*connect(client,
            QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::error),
            this, [this](QAbstractSocket::SocketError socketError) {
                std::cout << "Socket server error: "
                           << client->errorString().toStdString();
            });*/
    connect(client, SIGNAL(disconnected()), this, SLOT(DisConnected()));
    m_clients.insert(client, "");
}

void normalServer::ReadMessage()	//读取信息
{
    auto client = static_cast<QTcpSocket*>(QObject::sender());
    QByteArray qba= client->readAll(); //读取
    QString ss=QVariant(qba).toString();

    //std::cout << ss.toStdString() << std::endl;

    auto strs = rea::parseJsons(ss);
    for (auto str : strs){
        auto req = QJsonDocument::fromJson(str.toUtf8()).object();
        auto tp = req.value("type").toString();
        if (protocal.contains(tp))
        {
            clientMessage msg(req);
            msg.client_socket = client;
            rea::pipeline::run<clientMessage>("receiveFromClient", msg, rea::Json("tag", tp));
        }
    }
}
