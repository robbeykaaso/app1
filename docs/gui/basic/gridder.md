# Abstract
a automatic grid component for qml  

# Attribute
* name: a grid name and also the prefix of its item name, the item name is `name + "_gridder" + index`  
* com: the component of the grid item, it must have `width`, `height` and `name` attributes  
* size: the grid size. if it is integer, the layout will be automatically calculated. else if it is an array, the layout will be specifid by the size  
_sample_:  
```
Gridder{
    name: qsTr("demo")
    size: [2, 2]
    com: Component{
        Rectangle{
            property string name
            width: parent.width / parent.columns
            height: parent.height / parent.rows

            color: "transparent"
            border.color: "red"
        }
    }
}
```  
</br>

# API Pipe
* **name + _updateViewCount**  
renew the grid count and layout  
_sample_:  
```
Pipeline2.run(gridder_cld.name + "_updateViewCount", {size: [5, 5]})
```  
</br>

# Test and Demo
main.qml: qsTr("gridder")  
</br>