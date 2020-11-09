#ifndef REAL_FRAMEWORK_AWSSTORAGE_H_
#define REAL_FRAMEWORK_AWSSTORAGE_H_

#include "aws_s3.h"
#include "storage.h"

class awsStorage : public fsStorage {
public:
     awsStorage(const QString& aType = "", const QJsonObject& aConfig = QJsonObject());
     bool isValid() override { return m_aws.isValid(); }

protected:
    std::vector<QString> listFiles(const QString& aDirectory) override;
    void writeJson(const QString& aPath, const QJsonObject& aData) override;
    void writeCVMat(const QString& aPath, const cv::Mat& aData) override;
    void writeByteArray(const QString& aPath, const QByteArray& aData) override;
    std::vector<QString> getFileList(const QString& aPath) override;
    QJsonObject readJson(const QString& aPath) override;
    cv::Mat readCVMat(const QString& aPath) override;
    QByteArray readByteArray(const QString& aPath) override;
    void deletePath(const QString& aPath) override;
protected:
    AWSClient m_aws;
};

#endif
