# Abstract
a supper and default class for storages, it supports file system as default. its operations are respectively on the thread 10(for reading) and 11(for writing). All its pipe type is pipePartial

# API Pipe
* **afileRoot + readJson/writeJson**  
read/write the QJsonObject, data type: stgData<QJsonObject\>  
_sample_:
```
    using stgJson = stgData<QJsonObject>;
    pipeline::run<stgJson>("" + "writeJson", stgJson(Json("hello", "world"), "testFS.js"), Json("tag", "service1"));
```  
</br>

* **afileRoot + readCVMat/writeCVMat**  
read/write the cv::Mat, data type: stgData<cv::Mat\>  
</br>

* **afileRoot + readByteArray/writeByteArray**  
read/write the QByteArray, data type: stgData<QByteArray\>  
</br>

* **afileRoot + readDir/writeDir**  
read/write the std::vector<QByteArray\> to the directory, data type: stgData<std::vector<stgData<QByteArray\>\>\>  
</br>

* **afileRoot + deletePath**  
delete the directory/file recursively, data type: QString  
</br>

* **afileRoot + listFiles**  
list the files of the directory not recursively, data type: stgData<std::vector<QString\>\>  

# Test and Demo
awsStorage.cpp: testStorage(const QString& aRoot = "")

# Reference
[stgData](stgData.md)  
[awsStorage](awsStorage.md)  
