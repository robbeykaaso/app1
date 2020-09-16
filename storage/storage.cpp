#include "storage.h"
#include "reactive2.h"
#include <QFile>
#include <QJsonDocument>
#include <QDir>
#include <QBuffer>
#include <QVector>

void fsStorage::checkPath(const QString &aPath){
    auto dirs = aPath.split("/");
    QDir dir;
    QString origin = "";
    for (int i = 0; i < dirs.size() - 1; ++i){
        if (i > 0)
            origin += "/";
        origin += dirs[i];
        if (!dir.exists(origin))
            dir.mkdir(origin);
    }
}

void fsStorage::writeJson(const QString& aPath, const QJsonObject& aData){
    writeByteArray(aPath, QJsonDocument(aData).toJson());
}

QString fsStorage::stgRoot(const QString& aPath){
    if (m_root == "")
        return aPath;
    else
        return m_root + "/" + aPath;
}

void fsStorage::writeCVMat(const QString& aPath, const cv::Mat& aData){
    auto pth = stgRoot(aPath);
    checkPath(pth);
    cv::imwrite(pth.toStdString().data(), aData);
}

void fsStorage::writeQImage(const QString& aPath, const QImage& aData){
    auto pth = stgRoot(aPath);
    checkPath(pth);
    aData.save(pth);
}

void fsStorage::writeByteArray(const QString& aPath, const QByteArray& aData){
    auto pth = stgRoot(aPath);
    checkPath(pth);
    QFile fl(pth);
    if (fl.open(QFile::WriteOnly)){
        fl.write(aData);
        fl.close();
    }
}

QJsonObject fsStorage::readJson(const QString& aPath){
    QJsonObject ret;
    QFile fl(stgRoot(aPath));
    if (fl.open(QFile::ReadOnly)){
        QJsonDocument doc = QJsonDocument::fromJson(fl.readAll());
        ret = doc.object();
        fl.close();
    }
    return ret;
}

cv::Mat fsStorage::readCVMat(const QString& aPath){
    return cv::imread((stgRoot(aPath)).toUtf8().toStdString().data(), cv::IMREAD_UNCHANGED);
}

QImage fsStorage::readQImage(const QString& aPath){
    return QImage(stgRoot(aPath));
}

QByteArray fsStorage::readByteArray(const QString& aPath){
    QByteArray ret;
    QFile fl(stgRoot(aPath));
    if (fl.open(QFile::ReadOnly)){
        ret = fl.readAll();
        fl.close();
    }
    return ret;
}

void fsStorage::deletePath(const QString& aPath){
    if (aPath.indexOf(".") >= 0)
        QDir().remove(stgRoot(aPath));
    else
        QDir(stgRoot(aPath)).removeRecursively();
}

std::vector<QString> fsStorage::getFileList(const QString& aPath){
    std::vector<QString> ret;
    auto pth = stgRoot(aPath);
    QDir dir(pth);
    auto lst = dir.entryList();
    for (auto i : lst)
        if (i != "." && i != ".."){
            if (i.indexOf(".") >= 0)
                ret.push_back(aPath + "/" + i);
            else{
                auto clst = getFileList(aPath + "/" + i);
                ret.insert(ret.end(), clst.begin(), clst.end());
            }
        }
    return ret;
}

#define readStorage(aType) \
rea::pipeline::add<stg##aType, rea::pipePartial>([this](rea::stream<stg##aType>* aInput){ \
    auto dt = aInput->data(); \
    aInput->setData(stg##aType(read##aType(dt), dt))->out();  \
}, rea::Json("name", m_root + STR(read##aType), "thread", 10));

#define writeStorage(aType) \
rea::pipeline::add<stg##aType, rea::pipePartial>([this](rea::stream<stg##aType>* aInput){ \
    auto dt = aInput->data(); \
    write##aType(dt, dt.getData()); \
    aInput->out(); \
}, rea::Json("name", m_root + STR(write##aType), "thread", 11));

fsStorage::fsStorage(const QString& aRoot){
    m_root = aRoot;

    readStorage(Json);
    readStorage(ByteArray);
    readStorage(CVMat);
    readStorage(QImage);
    writeStorage(Json);
    writeStorage(ByteArray);
    writeStorage(CVMat);
    writeStorage(QImage);

    rea::pipeline::add<stgVector<stgByteArray>, rea::pipePartial>([this](rea::stream<stgVector<stgByteArray>>* aInput){
        auto dt = aInput->data();
        auto lst0 = getFileList(dt);
        for (auto i : lst0)
            dt.getData().push_back(stgByteArray(readByteArray(i), i));
        aInput->setData(dt)->out();
    }, rea::Json("name", m_root + "readDir", "thread", 10));

    rea::pipeline::add<stgVector<QString>, rea::pipePartial>([this](rea::stream<stgVector<QString>>* aInput){
        auto dt = aInput->data();
        aInput->setData(stgVector<QString>(listFiles(dt), dt))->out();
        aInput->out();
    }, rea::Json("name", m_root + "listFiles", "thread", 10));

    rea::pipeline::add<stgVector<stgByteArray>, rea::pipePartial>([this](rea::stream<stgVector<stgByteArray>>* aInput){
        auto dt = aInput->data().getData();
        for (auto i : dt)
            writeByteArray(aInput->data() + "/" + i, i.getData());
        aInput->out();
    }, rea::Json("name", m_root + "writeDir", "thread", 11));

    rea::pipeline::add<QString, rea::pipePartial>([this](rea::stream<QString>* aInput){
        deletePath(aInput->data());
        aInput->out();
    }, rea::Json("name", m_root + "deletePath", "thread", 11));

}

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

fsStorage::~fsStorage(){

}

std::vector<QString> fsStorage::listFiles(const QString& aDirectory){
    QDir dir(stgRoot(aDirectory));
    std::vector<QString> ret;
    auto lst = dir.entryList();
    for (auto i : lst)
        ret.push_back(i);
    return ret;
}

static rea::regPip<QJsonObject, rea::pipePartial> json2stg([](rea::stream<QJsonObject>* aInput){
    auto dt = aInput->data();
    aInput->out<stgJson>(stgJson(dt.value("data").toObject(), dt.value("path").toString()), dt.value("next").toString(), dt.value("param").toObject());
}, rea::Json("name", "json2stg"));
