#include "modbusMaster.h"
#include "reactive2.h"
#include <QSerialPort>

modBusMaster::modBusMaster(const QJsonObject& aConfig) : QObject(){

    connect(&m_modbus, &QModbusClient::errorOccurred, [this](QModbusDevice::Error) {
        rea::pipeline::run<QJsonObject>("modbusBoardcast", rea::Json("value", "modbus error: " + m_modbus.errorString()));
        // statusBar()->showMessage(modbusDevice->errorString(), 5000);
    });

    connect(&m_modbus, &QModbusClient::stateChanged, [this](QModbusDevice::State aState){
        if (aState == QModbusDevice::ConnectedState)
            rea::pipeline::run<QJsonObject>("modbusBoardcast", rea::Json("value", "modbus is connected"));
        else if (aState == QModbusDevice::UnconnectedState)
            rea::pipeline::run<QJsonObject>("modbusBoardcast", rea::Json("value", "modbus is unconnected"));
    });

    for (auto i : aConfig.keys())
        m_modbus.setConnectionParameter(i.toInt(), aConfig.value(i));

    m_modbus.connectDevice();

    rea::pipeline::add<QJsonObject, rea::pipeDelegate>([this](rea::stream<QJsonObject>* aInput){
        auto dt = aInput->data();
        auto req = QModbusRequest(QModbusRequest::FunctionCode(dt.value("func").toInt()),
                                  QByteArray::fromHex(dt.value("payload").toString().toUtf8()));
        sendRequest(req, dt.value("server").toInt(1));
    }, rea::Json("name", "callSlave", "param", rea::Json("delegate", "receiveFromSlave")));

    rea::pipeline::add<QByteArray>([](rea::stream<QByteArray>* aInput){
        aInput->out();
    }, rea::Json("name", "receiveFromSlave"));
}

void modBusMaster::sendRequest(const QModbusRequest& aRequest, int aServer) {

    if (auto* reply = m_modbus.sendRawRequest(aRequest, aServer)) {
        if (!reply->isFinished()) {
            QEventLoop loop;
            auto connection = connect(
                reply, &QModbusReply::finished, this, [this, reply, &loop]() {
                    if (reply->error() == QModbusDevice::ProtocolError) {
                        rea::pipeline::run<QJsonObject>("modbusBoardcast", rea::Json("value", "modbus protocal error: " + m_modbus.errorString()));
                    } else if (reply->error() != QModbusDevice::NoError) {
                        auto test = m_modbus.errorString();
                        rea::pipeline::run<QJsonObject>("modbusBoardcast", rea::Json("value", "modbus reply error: " + m_modbus.errorString()));
                    } else {
                        rea::pipeline::run<QByteArray>("receiveFromSlave", reply->rawResult().data().toHex());
                    }
                    reply->deleteLater();
                    loop.quit();
                });

            loop.exec();
            QObject::disconnect(connection);
        } else {
            // broadcast replies return immediately
            reply->deleteLater();
        }
    } else
        rea::pipeline::run<QJsonObject>("modbusBoardcast", rea::Json("value", "modbus send error: " + m_modbus.errorString()));
}

void modBusMaster::close() {
    if (m_modbus.state() == QModbusDevice::ConnectedState)
        m_modbus.disconnectDevice();
}

static rea::regPip<int> unit_test([](rea::stream<int>* aInput){
    const QJsonObject testBus = rea::Json(QString::number(QModbusDevice::SerialPortNameParameter), "COM2",
                                          QString::number(QModbusDevice::SerialParityParameter), QSerialPort::Parity::EvenParity,
                                          QString::number(QModbusDevice::SerialBaudRateParameter), QSerialPort::Baud19200,
                                          QString::number(QModbusDevice::SerialDataBitsParameter), QSerialPort::DataBits::Data8,
                                          QString::number(QModbusDevice::SerialStopBitsParameter), QSerialPort::StopBits::OneStop);

    static modBusMaster modbus(testBus);
//QLoggingCategory::setFilterRules(QStringLiteral("qt.modbus* = true"));

    rea::pipeline::find("callSlave")
    ->next(rea::pipeline::add<QByteArray>([](rea::stream<QByteArray>* aInput){
        auto dt = aInput->data();
        assert(dt == "0100");
        aInput->out<QString>("Pass: testModbusMaster ", "testSuccess");
    }))
    ->next("testSuccess");

    rea::pipeline::run<QJsonObject>("callSlave", rea::Json("func", QModbusRequest::FunctionCode::ReadCoils,
                                                           "payload", "00000001"));

    aInput->out();
}, QJsonObject(), "unitTest");
