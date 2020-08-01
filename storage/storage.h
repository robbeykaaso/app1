#ifndef REAL_FRAMEWORK_STORAGE_H_
#define REAL_FRAMEWORK_STORAGE_H_

#include "opencv2/opencv.hpp"
#include <QString>
#include <QJsonObject>
#include <QImage>

template <typename T>
class stgData : public QString{
public:
    stgData() : QString(){}
    stgData(T aData, const QString& aPath) : QString(aPath){
        m_data = aData;
    }
    T getData() {return m_data;}
private:
    T m_data;
};

using stgJson = stgData<QJsonObject>;
using stgByteArray = stgData<QByteArray>;
using stgCVMat = stgData<cv::Mat>;
using stgQImage = stgData<QImage>;
template <typename T>
using stgVector = stgData<std::vector<T>>;

class fsStorage{
public:
    fsStorage(const QString& aRoot = "");
    virtual ~fsStorage();
    virtual bool isValid() {return true;}
protected:
    virtual std::vector<QString> listFiles(const QString& aDirectory);
    virtual void writeJson(const QString& aPath, const QJsonObject& aData);
    virtual void writeCVMat(const QString& aPath, const cv::Mat& aData);
    virtual void writeQImage(const QString& aPath, const QImage& aData);
    virtual void writeByteArray(const QString& aPath, const QByteArray& aData);
    virtual QJsonObject readJson(const QString& aPath);
    virtual cv::Mat readCVMat(const QString& aPath);
    virtual QImage readQImage(const QString& aPath);
    virtual QByteArray readByteArray(const QString& aPath);
    virtual void deletePath(const QString& aPath);
    virtual std::vector<QString> getFileList(const QString& aPath);
private:
    virtual QString stgRoot(const QString& aPath);
    void checkPath(const QString &aPath);
protected:
    QString m_root;
};

#endif
