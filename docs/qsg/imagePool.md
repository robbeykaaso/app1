# Abstract
a singleton for caching images by their paths. the qsgModel will only fetch images from imagePool  

# API
* **void cacheImage(const QString& aPath, const QImage& aImage)**  
read and cache a image by its path  
</br>

* **QImage readCache(const QString& aPath)**  
read the cached image. if it doesn't exisit, it will try to read it from the file system and cache it  
</br>