#ifndef REAL_APPLICATION2_MODBUS_H_
#define REAL_APPLICATION2_MODBUS_H_

#include <QtSerialBus>

class modBusMaster : public QObject
{
    Q_OBJECT
public:
    modBusMaster(const QJsonObject& aConfig);
    ~modBusMaster(){
        close();
    }
private:
    void sendRequest(const QModbusRequest &aRequest, int aServer);
    void close();
    QModbusRtuSerialMaster m_modbus;
};


#endif
