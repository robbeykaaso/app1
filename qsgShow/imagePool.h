#ifndef REAL_FRAMEWORK_IMAGEPOOL_H_
#define REAL_FRAMEWORK_IMAGEPOOL_H_

#include "reactive2.h"
#include "storage/storage.h"
#include <QImage>

class imagePool{
public:
    static imagePool* instance();
    static void cacheImage(const QString& aPath, const QImage& aImage);
    static QImage readCache(const QString& aPath);
protected:
    imagePool(){}
    ~imagePool(){}
private:
    QHash<QString, QImage> m_images;
};

#endif
