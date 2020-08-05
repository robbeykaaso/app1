# Abstract
a log component for qml  

# Attribute
* type: the default is ["system", "train"], it could be overwritten by customized values  
* level: the default is ["info", "warning", "error"], it could be overwritten by customized values  
_sample_:
```
    Log{
        type: ["type1", "type2", "type3"]
        level: ["level1", "level2"]
    }
```  
</br>

# API Pipe
* **addLogRecord**  
add the one record to the log  
_sample_:  
```
Pipeline2.run("addLogRecord", {type: "train", level: "info", msg: "hello"}) // add the "msg" into ["type"]["level"] loglist
```  
</br>

* **showLogPanel**  
set log component "visible" to true  
</br>

# Test and Demo
main.qml: qsTr("log")  
</br>