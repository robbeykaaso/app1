# Abstract
a matrix component for qml  

# Attribute
* name: the instance name  
* rowcap: the row title Label  
* colcap: the column title Label  
* content: the matrix data, it is a 2D array  
_sample_:  
```
Matrix{
    rowcap.text: "hello"
    colcap.text: "world"
    content: [[1, 2], [3, 4], [5, 6]]
}
```  
</br>

# API Pipe
* **name + _updateMatrix**  
update the matrix show  
_sample_:  
```
Pipeline2.run("_updateMatrix", {rowcap: "hello2",  //the row title
                                colcap: "world2",  //the column title
                                content: [[1, 2, 3, 4],  //the content
                                          [5, 6, 7, 8],
                                          [9, 10, 11, 12],
                                          [13, 14, 15, 16],
                                          [17, 18, 19, 20]]})
```  
</br>

* **name + _matrixSelected**
output the 1D index in the content. its type is PipePartial  
</br>

# Test and Demo
main.qml: qsTr("matrix")  
</br>