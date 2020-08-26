# Abstract
a color dialog for qml  

# Attribute
* name: the instance name  
</br>

# API Pipe
* **name + _selectColor**  
open color dialog and select color. it is the delegate of pipe `name + _colorSelected`  
_sample_:  
```
Pipeline2.run("_selectColor", 
              {tag: {tag: "manual2"}})  //the service tag, the default is {tag: "manual"}
```  
</br>

* **name + _colorSelected**  
output the selected color  
</br>

# Test and Demo
main.qml: qsTr("color")  
</br>