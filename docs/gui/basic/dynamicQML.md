# Abstract
a qmls loader component for qml  

# Attribute
* name: the folder name. it will load all the qml files in the directory `cwd/plugin/name`  
* onLoaded: a slot for operating the loaded component instance  
_sample_:  
```
    DynamicQML{
        name: "menu"  //load the qmls in the menu folder
        onLoaded: function(aItem){
            mainmenu.addMenu(aItem)  // add a menu to the mainmenu
        }
    }
```  
</br>
