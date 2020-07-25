# Abstract
a serial modbus master, the constructor config is QModbusDevice::SerialPortNameParameter, QModbusDevice::SerialParityParameter, QModbusDevice::SerialBaudRateParameter, QModbusDevice::SerialDataBitsParameter and QModbusDevice::SerialStopBitsParameter

# API Pipe
* **callSlave**  
send "func", "payload", and "server" to the slave. it is a pipeDelegate for _receiveFromSlave_. data type: QJsonObject  
</br>

* **receiveFromSlave**  
got the message from slave. datt type: QByteArray  
</br>

# Test and Demo
modbusMaster.cpp: unit_test  
</br>