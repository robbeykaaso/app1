# Abstract
the track component for qml  

# Attribute
* caption: the Text object  
* slider: the Slider object  
* ratio: the ratio between caption and background  
* interval: the interval count  
_sample_:  
```
    Track{
        width: 180
        caption.text: qsTr("attri4") + ":"
        ratio: 0.4
    }
```  
</br>

# API
* **onIndexChanged(aIndex)**  
emit index changed signal  
</br>

# Test and Demo
main.qml: qsTr("baseCtrl")  
</br>