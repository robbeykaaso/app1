# Abstract
a file dialog component for qml  

# Attribute
* name: the instance name  

# API Pipe
* **name + _selectFile**  
open file dialog and select files or folder. it is the delegate of pipe `name + _fileSelected`  
_sample_:  
```
Pipeline2.run("_selectFile", {folder: true,  //select file or folder, the default is false
                              title: "hello",  //the dialog title, the default is "Please choose folder" and "Please choose files"
                              filter: ["Image files (*.jpg *.png *.jpeg *.bmp)"]  //the file filters, the default is ""
                              tag: {tag: "manual2"}})  //the service tag, the default is {tag: "manual"}
```  
</br>

* **name + _fileSelected**  
output the selected files or directory  
</br>

# Test and Demo
main.qml: qsTr("file")  
</br>