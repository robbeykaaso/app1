# Abstract
a normal tcp socket client which link a server

# API Pipe
* **callServer**  
send the json message to the server. it is a pipeDelegate for _receiveFromServer_. data type: QJsonObject  
</br>

* **receiveFromServer**  
got the message from the server. it is a pipePartial and the "tag" is defined by the specific service. data type: QJsonObject  
</br>

* **tryLinkServer**  
either try link server which has specific "ip", "port", "id" or use ssdp protocal to link server. data type: QJsonObject  
</br>

* **clientBoardcast**  
boardcast some information after some specific operation  
</br>

# Test and Demo
unitTest.cpp: testSocket()  
</br>

# Reference
[normalServer](normalServer.md)