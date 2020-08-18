# Abstract
a progress window for qml  

# API Pipe
* **updateProgress**  
update progress window and output the calculated double progress. the progress window will opened when the progress is not 1. its type is pipePartial  
_sample_:  
```
Pipeline2.run("updateProgress", {title: "demo: ", sum: 10}, {tag: "service1"})  //initialize the progress window
Pipeline2.run("updateProgress", {step: 2}, {tag: "service1"})  //update the progress, if there is no "step", the default is 1
```  
</br>

# Test and Demo
main.qml: qsTr("progress")  
</br>