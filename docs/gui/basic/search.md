# Abstract
a search component for qml  

# Attribute
* name: the instance name  
* text: the input text  
* hint: the placeholder text  
* prefix: the prefix text  
_sample_:  
```
    Search{
        text: qsTr("hello")
        hint: qsTr("Enter...")
        prefix: "@"
    }
```  
</br>

# API Pipe
* **name + _Searched**  
output the search text. its type is PipePartial  
</br>

# Test and Demo
main.qml: qsTr("search")  
</br>