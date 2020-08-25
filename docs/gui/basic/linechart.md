# Abstract
a linechart component for qml  

# Attribute
* name: the instance name  
* content: the points y data, its type is array  
</br>

# API Pipe
* **name + _updateLineChart**  
update the linechart show  
_sample_:  
```
    Pipeline2.run("_updateLineChart", [20, 30, 100, 125, 30, 10, 12, 30, 50])
```  
</br>

# Test and Demo
main.qml: qsTr("lineChart")  
</br>