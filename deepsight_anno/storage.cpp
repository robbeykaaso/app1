#include "storage.h"
#include "protocal.h"
#include "reactive2.h"

static QSet<QString> anno_json;

bool checkValidOperation(const QString& aPath, bool aDelete = false) {
    if (aPath.startsWith("user/") || aPath.endsWith("project.json"))
        return true;
    else if (!aDelete && anno_json.contains(aPath)){ //forbid delete anno.json and add anno.json, only permit modify anno.json
        return true;
    }
    return false;
}

QJsonObject fsStorage2::readJson(const QString& aPath){
    auto tmp = aPath.mid(0, aPath.lastIndexOf("/"));
    if (tmp.endsWith("/image"))
        anno_json.insert(aPath);
    auto ret = fsStorage::readJson(aPath);
    return ret;
}

void fsStorage2::writeJson(const QString& aPath, const QJsonObject& aData) {
    if (checkValidOperation(aPath) || m_is_owner())
        fsStorage0::writeJson(aPath, aData);
}

void fsStorage2::writeQImage(const QString& aPath, const QImage& aData) {
    if (checkValidOperation(aPath) || m_is_owner())
        fsStorage0::writeQImage(aPath, aData);
}

void fsStorage2::writeByteArray(const QString& aPath, const QByteArray& aData) {
    if (checkValidOperation(aPath) || m_is_owner())
        fsStorage0::writeByteArray(aPath, aData);
}

void fsStorage2::deletePath(const QString& aPath) {
    if (checkValidOperation(aPath, true) || m_is_owner())
        fsStorage0::deletePath(aPath);
}

void fsStorage2::writeCVMat(const QString& aPath, const cv::Mat& aData) {
    if (checkValidOperation(aPath) || m_is_owner())
        fsStorage::writeCVMat(aPath, aData);
}

awsStorage2::awsStorage2(const QString& aType, const QJsonObject& aConfig, std::function<bool()> isOwner) : awsStorage(aType, aConfig) {
  m_is_owner = isOwner;
  // tell server aws info
  rea::pipeline::find("clientBoardcast")
      ->next(rea::pipeline::add<QJsonObject>([this](rea::stream<QJsonObject>* aInput) {
        if (aInput->data().value("value") == "socket is connected") {
          aInput->out<QJsonObject>(
              rea::Json(protocal.value(protocal_connect)
                            .toObject()
                            .value("req")
                            .toObject(),
                        "s3_ip_port",
                        QString::fromStdString(m_aws.getIPPort()),
                        "s3_access_key",
                        QString::fromStdString(m_aws.getAccessKey()),
                        "s3_secret_key",
                        QString::fromStdString(m_aws.getSecretKey())),
              "callServer");
        }
      }))
      ->next("callServer")
      ->next(rea::pipeline::add<QJsonObject>([](rea::stream<QJsonObject>* aInput) {
            assert(aInput->data().value("type") == "connect");
      }), rea::Json("tag", protocal_connect));
}

QJsonObject awsStorage2::readJson(const QString& aPath){
    auto tmp = aPath.mid(0, aPath.lastIndexOf("/"));
    if (tmp.endsWith("/image"))
        anno_json.insert(aPath);
    return awsStorage::readJson(aPath);
}

void awsStorage2::writeJson(const QString& aPath, const QJsonObject& aData) {
    if (checkValidOperation(aPath) || m_is_owner())
        awsStorage::writeJson(aPath, aData);
}

void awsStorage2::writeCVMat(const QString& aPath, const cv::Mat& aData) {
    if (checkValidOperation(aPath) || m_is_owner())
        awsStorage::writeCVMat(aPath, aData);
}

void awsStorage2::writeByteArray(const QString& aPath, const QByteArray& aData) {
    if (checkValidOperation(aPath) || m_is_owner())
        awsStorage::writeByteArray(aPath, aData);
}

void awsStorage2::deletePath(const QString& aPath) {
    if (checkValidOperation(aPath, true) || m_is_owner())
        awsStorage::deletePath(aPath);
}
