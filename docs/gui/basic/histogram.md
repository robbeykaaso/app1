# Abstract
a normal histogram for qml. the x range depends on the size of data, and the y range depends on the maximum of data  

# Attribute
* name: the instance name  
</br>

# API Pipe
* **name + _updateHistogramGUI**  
update the histogram show  
_sample_:  
```
Pipeline2.run("_updateHistogramGUI", {histogram: [40, 20, 15, 25, 14, 16, 13, 30]})
```  
</br>

# Test and Demo
main.qml: qsTr("histogram")  
</br>