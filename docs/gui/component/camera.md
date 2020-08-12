# Abstract
a camera window component based on deepsight sdk  

# Attribute
* name: the default is "camera1", it should be noticed that different camera instance should have different name  
_sample_:  
```
    Camera{
        name: "camera2"
    }
```  
</br>

# API Pipe
* **name + "_captureCamera"**  
trig the camera to capture image once. it is a pipeDelegate of "_cameraCaptured"  
</br>

* **name + "_setCamera"**  
set the camera parameter  
</br>

* **name + "_turnCamera"**  
turn on/off camera. the parameter "on" is boolean  
</br>

* **name + "_cameraStated"**  
output a QJsonObject of the camera. the parameter "state" is the camera state  
</br>

* **name + "_cameraCaptured"**  
output std::vector<ds::DsFrameData\>  
</br>

* **name + "_cameraShow"**  
show the std::vector<ds::DsFrameData\> on the gui  
</br>

# Test and Demo
* main.qml: camera  
* camera.cpp: unitTest  
</br>