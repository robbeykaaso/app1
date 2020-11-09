#ifndef REAL_APP_STORAGE_H_
#define REAL_APP_STORAGE_H_

#include "../storage/awsStorage.h"
#include "../storage/storage.h"

class fsStorage2 : public fsStorage {
 public:
  fsStorage2(const QString& aRoot, std::function<bool()> isOwner = nullptr)
      : fsStorage(aRoot) {
    m_is_owner = isOwner;
  }

 protected:
  QJsonObject readJson(const QString& aPath) override;
  void writeJson(const QString& aPath, const QJsonObject& aData) override;
  void writeQImage(const QString& aPath, const QImage& aData) override;
  void writeByteArray(const QString& aPath, const QByteArray& aData) override;
  void deletePath(const QString& aPath) override;
  void writeCVMat(const QString& aPath, const cv::Mat& aData) override;

 private:
  std::function<bool()> m_is_owner;
};

class awsStorage2 : public awsStorage {
 public:
     awsStorage2(const QString& aType, const QJsonObject& aConfig = QJsonObject(), std::function<bool()> isOwner = nullptr);

 protected:
  QJsonObject readJson(const QString& aPath) override;
  void writeJson(const QString& aPath, const QJsonObject& aData) override;
  void writeCVMat(const QString& aPath, const cv::Mat& aData) override;
  void writeByteArray(const QString& aPath, const QByteArray& aData) override;
  void deletePath(const QString& aPath) override;

 private:
  std::function<bool()> m_is_owner;
};

#endif
