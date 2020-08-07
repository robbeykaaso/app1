#include "reactive2.h"
#include "storage.h"
#include "aws_s3.h"
#include <QJsonDocument>
#include <QDir>

class awsStorage : public fsStorage{
public:
    awsStorage(const QString& aType = "");
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

#include <Windows.h>
//
QJsonObject checkMinIO(){
    // 打开服务管理对象
    SC_HANDLE hSC = ::OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT | SC_MANAGER_ENUMERATE_SERVICE);

    if( hSC == NULL){
        return QJsonObject();
    }

    /*LPENUM_SERVICE_STATUS lpServices    = NULL;
    DWORD    nSize = 0;
    DWORD    n;
    DWORD    nResumeHandle = 0;

    lpServices = (LPENUM_SERVICE_STATUS) LocalAlloc(LPTR, 64 * 1024);      //注意分配足够的空间
    EnumServicesStatus(hSC,SERVICE_WIN32,
                       SERVICE_STATE_ALL,
                       (LPENUM_SERVICE_STATUS)lpServices,
                       64 * 1024,
                       &nSize,
                       &n,
                       &nResumeHandle);
    for (int i = 0; i < n; i++)
    {
        auto test = lpServices[i];
        if (QString::fromStdString(test.lpServiceName) == "MinIO")
            std::cout << test.lpServiceName << std::endl;
    }*/

    SC_HANDLE hSvc = ::OpenService( hSC, "MinIO", SERVICE_QUERY_STATUS);

    //auto tmp = GetLastError();

    if( hSvc == NULL)
    {
        ::CloseServiceHandle( hSC);
        return QJsonObject();
    }
    // 获得服务的状态
    SERVICE_STATUS status;
    if( ::QueryServiceStatus( hSvc, &status) == FALSE)
    {
        ::CloseServiceHandle( hSvc);
        ::CloseServiceHandle( hSC);
        return QJsonObject();
    }else{
        QJsonObject ret;
        ret.insert("running", status.dwCurrentState == SERVICE_RUNNING);
        ::CloseServiceHandle( hSvc);
        ::CloseServiceHandle( hSC);
        return ret;
    }
    //如果处于停止状态则启动服务，否则停止服务。
    /*    if( status.dwCurrentState == SERVICE_RUNNING)
    {
        // 停止服务
        if( ::ControlService( hSvc,
                             SERVICE_CONTROL_STOP, &status) == FALSE)
        {
            ::CloseServiceHandle( hSvc);
            ::CloseServiceHandle( hSC);
            return;
        }
        // 等待服务停止
        while( ::QueryServiceStatus( hSvc, &status) == TRUE)
        {
            ::Sleep( status.dwWaitHint);
            if( status.dwCurrentState == SERVICE_STOPPED)
            {
                ::CloseServiceHandle( hSvc);
                ::CloseServiceHandle( hSC);
                return;
            }
        }
    }
    else if( status.dwCurrentState == SERVICE_STOPPED)
    {
        // 启动服务
        if( ::StartService( hSvc, NULL, NULL) == FALSE)
        {
            ::CloseServiceHandle( hSvc);
            ::CloseServiceHandle( hSC);
            return;
        }
        // 等待服务启动
        while( ::QueryServiceStatus( hSvc, &status) == TRUE)
        {
            ::Sleep( status.dwWaitHint);
            if( status.dwCurrentState == SERVICE_RUNNING)
            {
                ::CloseServiceHandle( hSvc);
                ::CloseServiceHandle( hSC);
                return;
            }
        }
    }*/
}

awsStorage::awsStorage(const QString& aType) : fsStorage(aType){
    m_aws.initialize([](){
        bool ret = false;
        auto sts = checkMinIO();
        do{
            if (sts.contains("running"))
                ret = true;
            if (!sts.value("running").toBool()){
                auto cmd = "\"" + QDir::currentPath() + "/minIO/storage_start.bat\"";
                //system("start /b minio.exe server storage");
                system(cmd.toStdString().c_str());
                std::this_thread::sleep_for(std::chrono::microseconds(1000));
            }else
                break;
            auto cmd = "\"" + QDir::currentPath() + "/minIO/storage_start.bat\"";
            //system("start /b minio.exe server storage");
            system(cmd.toStdString().c_str());
        }while(0);
        return ret;
    });
    m_aws.create_bucket(m_root.toStdString().c_str());
}

std::vector<QString> awsStorage::getFileList(const QString& aPath){
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

std::vector<QString> awsStorage::listFiles(const QString& aDirectory){
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

void awsStorage::writeJson(const QString& aPath, const QJsonObject& aData){
    auto stm = Aws::MakeShared<Aws::StringStream>("object");

    QJsonDocument doc(aData);
    auto str = doc.toJson().toStdString();
    stm->write(str.c_str(), str.size());

    m_aws.put_s3_string_object(m_root.toStdString().c_str(), aPath.toStdString().c_str(), stm);
}

void awsStorage::writeCVMat(const QString& aPath, const cv::Mat& aData){
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

void awsStorage::writeByteArray(const QString& aPath, const QByteArray& aData){
    auto stm = Aws::MakeShared<Aws::StringStream>("object");

    auto str = aData.toStdString();
    stm->write(str.c_str(), str.size());

    m_aws.put_s3_string_object(m_root.toStdString().c_str(), aPath.toStdString().c_str(), stm);
}

QJsonObject awsStorage::readJson(const QString& aPath){
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

cv::Mat awsStorage::readCVMat(const QString& aPath){
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

QByteArray awsStorage::readByteArray(const QString& aPath){
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

void awsStorage::deletePath(const QString& aPath){
    deleteAWSDirectory(m_aws, m_root, aPath);
}

/*REGISTERPipe(initializeBackend, inistg, [](std::shared_ptr<dst::streamData> aInput){
    QJsonObject debug_cfg;
    dst::configObject::loadJsonFileConfig("debugInfo", debug_cfg);

    if (debug_cfg.value("localfs").toBool()){
        static fsStorage data_stg;
    }else{
        static awsStorage data_stg;
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

void testStorage(const QString& aRoot = ""){ //for fs: aRoot == ""; for minIO: aRoot != ""
    using namespace rea;
    auto tag = Json("tag", "testStorage");
    pipeline::find(aRoot + "writeJson")
        ->next(local(aRoot + "readJson"), tag)
        ->next(FUNCT(stgJson, aRoot,
                     auto js = aInput->data().getData();
                     assert(js.value("hello") == "world2");
                     aInput->out<stgCVMat>(stgCVMat(cv::Mat(10, 10, CV_8UC1, cv::Scalar(0)), "testMat.png"), aRoot + "writeCVMat");
                             ))
        ->next(local(aRoot + "writeCVMat"))
        ->next(local(aRoot + "readCVMat"))
        ->next(FUNCT(stgCVMat, aRoot,
                     auto dt = aInput->data().getData();
                     assert(dt.cols == 10 && dt.rows == 10);
                     aInput->out<stgByteArray>(stgByteArray(QJsonDocument(Json("hello", "world")).toJson(), "testFS.json"), aRoot + "writeByteArray");
                             ))
        ->next(local(aRoot + "writeByteArray"))
        ->next(local(aRoot + "readByteArray"))
        ->next(FUNCT(stgByteArray, aRoot,
                     auto dt = aInput->data().getData();
                     auto cfg = QJsonDocument::fromJson(dt).object();
                     assert(cfg.value("hello") == "world");

                     std::vector<stgByteArray> dts;
                     dts.push_back(stgByteArray(dt, "testFS.json"));
                     stgVector<stgByteArray> stm(dts, "testDir");
                     aInput->out<stgVector<stgByteArray>>(stm, aRoot + "writeDir");
                             ))
        ->next(local(aRoot + "writeDir"))
        ->next(local(aRoot + "readDir"))
        ->next(FUNCT(stgVector<stgByteArray>, aRoot,
                     auto dt = aInput->data().getData();
                     if (aRoot == "")
                         assert(QDir().exists(aInput->data() + "/" + dt.at(0)));
                     aInput->out<stgVector<QString>>(stgVector<QString>(std::vector<QString>(), aInput->data()), aRoot + "listFiles");
                             ))
        ->next(local(aRoot + "listFiles"))
        ->next(FUNCT(stgVector<QString>, aRoot,
                     auto dt = aInput->data().getData();
                     if (aRoot == "")
                         assert(dt.size() == 3);
                     else
                     assert(dt.size() == 1);
                     aInput->out<QString>("testDir", aRoot + "deletePath");
                     aInput->out<QString>("testFS.json", aRoot + "deletePath");
                     aInput->out<QString>("testMat.png", aRoot + "deletePath");
                             ))
        ->next(local(aRoot + "deletePath"))
        ->next(buffer<QString>(3))
        ->next(FUNCT(std::vector<QString>, aRoot,
                     aInput->out<stgVector<QString>>(stgVector<QString>(std::vector<QString>(), aRoot + "/testDir"), aRoot + "listFiles");
                             ))
        ->next(local(aRoot + "listFiles"))
        ->next(FUNCT(stgVector<QString>, aRoot,
                     auto dt = aInput->data().getData();
                     assert(dt.size() == 0);
                     aInput->out<QString>("Pass: testStorage " + aRoot, "testSuccess");
                             ))
        ->next("testSuccess");

    pipeline::run<stgJson>(aRoot + "writeJson", stgJson(Json("hello", "world2"), "testFS.json"), tag);
}

static rea::regPip<int> unit_test([](rea::stream<int>* aInput){
    static fsStorage local_storage;
    testStorage();
    static awsStorage minio_storage("testminio");
    testStorage("testminio");
    aInput->out();
}, QJsonObject(), "unitTest");
