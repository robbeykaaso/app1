# Abstract
a normal tcp socket server which maintains multi-clients

# API Pipe
* **callClient**  
send the json message to the clients with specific names, data type: clientMessage  
</br>

* **receiveFromClient**  
got the message from a client, it is a pipePartial. if there is a pipe after it, the "tag" is the "type" in the json message. data type: clientMessage  
</br>

# Test and Demo
unitTest.cpp: testSocket()  
</br>

# Reference
[normalClient](normalClient.md)