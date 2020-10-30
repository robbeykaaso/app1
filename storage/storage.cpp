#include "storage.h"
#include "reactive2.h"
#include <QFile>
#include <QJsonDocument>
#include <QDir>
#include <QBuffer>
#include <QVector>

void fsStorage::writeCVMat(const QString& aPath, const cv::Mat& aData){
    auto pth = stgRoot(aPath).toLocal8Bit();
    checkPath(pth);
    cv::imwrite(pth.toStdString().data(), aData);
}

cv::Mat fsStorage::readCVMat(const QString& aPath){
    auto ret = cv::imread((stgRoot(aPath)).toLocal8Bit().toStdString().data(), cv::IMREAD_UNCHANGED);
//    cv::imwrite("xxxxxx2.png", ret);
    return ret;
}

fsStorage::fsStorage(const QString& aRoot) : fsStorage0(aRoot){
    REGREADSTORAGE(CVMat);
    REGWRITESTORAGE(CVMat);
}

static rea::regPip<QJsonObject, rea::pipePartial> json2stg([](rea::stream<QJsonObject>* aInput){
    auto dt = aInput->data();
    aInput->out<rea::stgJson>(rea::stgJson(dt.value("data").toObject(), dt.value("path").toString()), dt.value("next").toString(), dt.value("param").toObject());
}, rea::Json("name", "json2stg"));

/*bool safetyWrite(const QString& aPath, const QByteArray& aData){
    QDir().mkdir("Temp");
    auto tmp = "Temp/" + aPath.mid(aPath.lastIndexOf("/") + 1, aPath.length());
    QFile fl(tmp);
    if (fl.open(QFile::WriteOnly)){
        fl.write(aData);
        fl.close();
        if (!MoveFileExA(tmp.toLocal8Bit().toStdString().data(), aPath.toLocal8Bit().toStdString().data(), MOVEFILE_REPLACE_EXISTING)){
            std::cout << "write file error: " << GetLastError() << std::endl;
            return false;
        }
        return true;
    }
    return false;
}*/
