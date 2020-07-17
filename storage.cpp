#include "storage.h"
#include "reactive2.h"
#include "aws_s3.h"
#include <QFile>
#include <QJsonDocument>
#include <QDir>
#include <QBuffer>
#include <QVector>
//#include <Windows.h>

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
    return cv::imread((stgRoot(aPath)).toLocal8Bit().toStdString().data(), cv::IMREAD_UNCHANGED);
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
    writeStorage(Json);
    writeStorage(ByteArray);
    writeStorage(CVMat);

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

class minIOStorage : public fsStorage{
public:
    minIOStorage(const QString& aType = "");
    bool isValid() override {return m_aws.isValid();}
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
private:
    AWSClient m_aws;
};

minIOStorage::minIOStorage(const QString& aType) : fsStorage(aType){
    m_aws.create_bucket(m_root.toStdString().c_str());
}

std::vector<QString> minIOStorage::getFileList(const QString& aPath){
    std::vector<QString> ret;
    std::vector<std::string> lst;
    m_aws.list_s3_objects(m_root.toStdString().data(), aPath.toStdString().data(), lst);
    for (auto i : lst){
        auto pth = QString::fromStdString(i);
        if (pth.indexOf(".") >= 0)
            ret.push_back(pth);
        else{
            auto clst = getFileList(pth);
            ret.insert(ret.end(), clst.begin(), clst.end());
        }
    }
    return ret;
}

std::vector<QString> minIOStorage::listFiles(const QString& aDirectory){
    std::vector<std::string> lst;
    m_aws.list_s3_objects(m_root.toStdString().data(), aDirectory.toStdString().data(), lst);
    std::vector<QString> ret;
    for (auto i : lst){
        auto fl = QString::fromStdString(i);
        fl = fl.mid(fl.lastIndexOf("/") + 1, fl.length());
        ret.push_back(fl);
    }
    return ret;
}

void minIOStorage::writeJson(const QString& aPath, const QJsonObject& aData){
    auto stm = Aws::MakeShared<Aws::StringStream>("object");

    QJsonDocument doc(aData);
    auto str = doc.toJson().toStdString();
    stm->write(str.c_str(), str.size());

    m_aws.put_s3_string_object(m_root.toStdString().c_str(), aPath.toStdString().c_str(), stm);
}

void minIOStorage::writeCVMat(const QString& aPath, const cv::Mat& aData){
    auto stm = Aws::MakeShared<Aws::StringStream>("object");
    //https://blog.csdn.net/weixin_42112458/article/details/83117305?depth_1-utm_source=distribute.pc_relevant.none-task&utm_source=distribute.pc_relevant.none-task
    auto suffix = aPath.mid(aPath.lastIndexOf("."), aPath.length());
    std::vector<uchar> buff;
    cv::imencode(suffix.toStdString().data(), aData, buff);
    char* dt = new char[buff.size()];
    memcpy(dt, reinterpret_cast<char*>(&buff[0]), buff.size());
    stm->write(dt, buff.size());
    delete[] dt;
    m_aws.put_s3_string_object(m_root.toStdString().c_str(), aPath.toStdString().c_str(), stm);
}

void minIOStorage::writeByteArray(const QString& aPath, const QByteArray& aData){
    auto stm = Aws::MakeShared<Aws::StringStream>("object");

    auto str = aData.toStdString();
    stm->write(str.c_str(), str.size());

    m_aws.put_s3_string_object(m_root.toStdString().c_str(), aPath.toStdString().c_str(), stm);
}

QJsonObject minIOStorage::readJson(const QString& aPath){
    QJsonObject ret;
    int sz;
    auto str = m_aws.get_s3_object(m_root.toStdString().c_str(), aPath.toStdString().c_str(), sz);
    if (str != nullptr){
        QJsonDocument doc = QJsonDocument::fromJson(QString::fromLocal8Bit(str, sz).toUtf8());
        ret = doc.object();
        delete[]str;
    }
    return ret;
}

cv::Mat minIOStorage::readCVMat(const QString& aPath){
    cv::Mat ret;
    int sz;
    auto img = m_aws.get_s3_object(m_root.toStdString().c_str(), aPath.toStdString().c_str(), sz);
    if (img != nullptr){
        std::string str2(img, sz);
        std::vector<char> vec(str2.c_str(), str2.c_str() + str2.size());
        ret = cv::imdecode(vec, cv::IMREAD_UNCHANGED);
    }
    return ret;
}

QByteArray minIOStorage::readByteArray(const QString& aPath){
    QByteArray ret;
    int sz;
    auto str = m_aws.get_s3_object(m_root.toStdString().c_str(), aPath.toStdString().c_str(), sz);
    if (str != nullptr){
        ret = QByteArray(str, sz);
        delete[]str;
    }
    return ret;
}

void deleteAWSDirectory(AWSClient& aClient, const QString& aBucket, const QString& aPath){
    if (aPath.indexOf(".") >= 0)
        aClient.delete_s3_object(aBucket.toStdString().c_str(), aPath.toStdString().c_str());
    else{
        std::vector<std::string> lst;
        aClient.list_s3_objects(aBucket.toStdString().c_str(), aPath.toStdString().c_str(), lst);
        for (auto i : lst)
            deleteAWSDirectory(aClient, aBucket, QString::fromStdString(i));
    }
}

void minIOStorage::deletePath(const QString& aPath){
    deleteAWSDirectory(m_aws, m_root, aPath);
}

/*REGISTERPipe(initializeBackend, inistg, [](std::shared_ptr<dst::streamData> aInput){
    QJsonObject debug_cfg;
    dst::configObject::loadJsonFileConfig("debugInfo", debug_cfg);

    if (debug_cfg.value("localfs").toBool()){
        static fsStorage data_stg;
    }else{
        static minIOStorage data_stg;
        if (!data_stg.isValid()){
            std::cout << "Storage is initialized failed!" << std::endl;
            aInput->callback(nullptr);
            return std::shared_ptr<dst::streamData>(nullptr);
        }
    }
    static fsStorage local_stg("local_");

    dst::document::instance()->findModel("socketClient");
    if (debug_cfg.value("test_server").toBool()){
        dst::document::instance()->findModel("socketServer");
        TRIG("tryLinkServer", STMJSON(dst::Json("ip", "127.0.0.1", "port", "8081", "id", "No2")));
    }else
        TRIG("tryLinkServer", STMJSON(QJsonObject()));

    return aInput;
}, 0);*/

#define FUNCT(aType, aRoot, aFunc) \
    pipeline::add<aType>([aRoot](stream<aType>* aInput){aFunc})

void testStorage(const QString aRoot = ""){
    using namespace rea;
    auto tag = Json("tag", "testStorage");
    pipeline::find(aRoot + "writeJson")
        ->next(local<stgJson>(aRoot + "readJson"), tag)
        ->next(FUNCT(stgJson, aRoot,
            auto js = aInput->data().getData();
            assert(js.value("hello") == "world2");
            aInput->out<stgCVMat>(stgCVMat(cv::Mat(10, 10, CV_8UC1, cv::Scalar(0)), "testMat.png"), aRoot + "writeCVMat");
        ))
        ->next(local<stgCVMat>(aRoot + "writeCVMat"))
        ->next(local<stgCVMat>(aRoot + "readCVMat"))
        ->next(FUNCT(stgCVMat, aRoot,
            auto dt = aInput->data().getData();
            assert(dt.cols == 10 && dt.rows == 10);
            aInput->out<stgByteArray>(stgByteArray(QJsonDocument(Json("hello", "world")).toJson(), "testFS.json"), aRoot + "writeByteArray");
        ))
        ->next(local<stgByteArray>(aRoot + "writeByteArray"))
        ->next(local<stgByteArray>(aRoot + "readByteArray"))
        ->next(FUNCT(stgByteArray, aRoot,
            auto dt = aInput->data().getData();
            auto cfg = QJsonDocument::fromJson(dt).object();
            assert(cfg.value("hello") == "world");

            std::vector<stgByteArray> dts;
            dts.push_back(stgByteArray(dt, "testFS.json"));
            stgVector<stgByteArray> stm(dts, "testDir");
            aInput->out<stgVector<stgByteArray>>(stm, aRoot + "writeDir");
        ))
        ->next(local<stgVector<stgByteArray>>(aRoot + "writeDir"))
        ->next(local<stgVector<stgByteArray>>(aRoot + "readDir"))
        ->next(FUNCT(stgVector<stgByteArray>, aRoot,
            auto dt = aInput->data().getData();
            assert(QDir().exists(aInput->data() + "/" + dt.at(0)));
            aInput->out<stgVector<QString>>(stgVector<QString>(std::vector<QString>(), aInput->data()), aRoot + "listFiles");
        ))
        ->next(local<stgVector<QString>>(aRoot + "listFiles"))
        ->next(FUNCT(stgVector<QString>, aRoot,
            auto dt = aInput->data().getData();
            assert(dt.size() == 3);
            aInput->out<QString>("testDir", aRoot + "deletePath");
            aInput->out<QString>("testFS.json", aRoot + "deletePath");
            aInput->out<QString>("testMat.png", aRoot + "deletePath");
        ))
        ->next(local<QString>(aRoot + "deletePath"))
        ->next(buffer<QString>(3))
        ->next(FUNC(std::vector<QString>,
            auto pths = aInput->data();
            for (auto i : pths){
                assert(!QDir().exists(i));
            }
            aInput->out<QString>("Pass: testStorage", "testSuccess");
        ))
        ->next("testSuccess");

    pipeline::run<stgJson>(aRoot + "writeJson", stgJson(rea::Json("hello", "world2"), "testFS.json"), tag);
}

static rea::regPip<int> unit_test([](rea::stream<int>* aInput){
    static fsStorage local_storage;
    testStorage();
    //static minIOStorage minio_storage("testMinIO");
    //testStorage("testMinIO");
    aInput->out();
}, QJsonObject(), "unitTest");
