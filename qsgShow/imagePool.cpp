#include "imagePool.h"

imagePool* imagePool::instance(){
    static std::mutex imagePool_mutex;
    std::lock_guard<std::mutex> lg(imagePool_mutex);
    static imagePool ret;  //if realized in .h, there will be multi objects in different dlls
    return &ret;
}

void imagePool::cacheImage(const QString& aPath, const QImage& aImage){
    imagePool::instance()->m_images.insert(aPath, aImage);
}

QImage imagePool::readCache(const QString& aPath){
    if (!imagePool::instance()->m_images.contains(aPath))
        imagePool::instance()->m_images.insert(aPath, QImage(aPath));
    return imagePool::instance()->m_images.value(aPath);
}
