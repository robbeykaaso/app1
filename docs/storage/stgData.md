# Abstract
a container of the API interface for storage, it inherits from QString and it self is the path

# Sample
```
    using stgJson = stgData<QJsonObject>

    template<typename T>
    using stgVector = stgData<std::vector<T>>

    stgData<QJsonObject>(QJsonObject(), "hello.txt")  //construct an object for QJsonObject
```  
</br>

# Reference
[fsStorage](fsStorage.md)  
[awsStorage](awsStorage.md)  