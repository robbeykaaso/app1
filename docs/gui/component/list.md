# Abstract
a list component for qml

# Attribute
* name: the instance name  
_sample_:  
```
    List{
        name: "hello"
    }
```  
</br>

# Usage
customer could select the items by left button with "Ctrl" or "Shift"  
</br>

# API Pipe
* **name + _updateListView**  
renew the list gui by model data  
_sample_:  
```
    Pipeline2.run("_updateListView", {title: ["cat", "dog", "sheep", "rat"],  //the list title names
                                        selects: [1, 3, 5],  //the selected items indexes
                                        data: [  //the datas, please ensure the columns length equaled to the titles length
                                        {entry: [4, 6, 2, 3]},  
                                        {entry: [4, 6, 2, 3]},
                                        {entry: [4, 6, 2, 3]},
                                        {entry: [4, 6, 2, 3]},
                                        {entry: [4, 6, 2, 3]},
                                        {entry: [4, 6, 2, 3]}
                                        ]})

    Pipeline2.run("_updateListView", {index: [2, 4, 5],  //if contains "index", the gui will modify the specific data rather than replace all old data
                                    fontclr: "red",  //the font color for each entry
                                    data: [
                                    {entry: [1, 3, 2, 3]},
                                    {},  //if there is no need to change old data, it could be empty
                                    {entry: [2, 3, 2, 3]}
                                    ]})
```  
</br>

* **name + _listViewSelected**  
output the selected indexes in the list. its type is PipePartial and output is the array type  
_sample_:  
```
    Pipeline2.find("_listViewSelected").next(function(aInput){
        console.log(aInput)
    }, {tag: "manual"}, {vtype: []})
```  
</br>

# Test and Demo
main.qml: qsTr("list")  
</br>