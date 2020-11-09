#include "reactive2.h"
#include "aws_s3.h"
#include <aws/s3/model/CreateBucketRequest.h>
#include <aws/s3/model/DeleteBucketRequest.h>
#include <aws/s3/model/PutObjectRequest.h>
#include <aws/s3/model/GetObjectRequest.h>
#include <aws/s3/model/DeleteObjectRequest.h>
#include <aws/s3/model/ListObjectsRequest.h>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDir>
#include <QHostAddress>

void AWSClient::tryStartMinIO(bool aFirstTry){
   /* dst::streamManager::instance()->emitSignal("checkMinIO", STMJSON(QJsonObject(), [this, aFirstTry](void* aStatus){
                                                   auto sts = reinterpret_cast<QJsonObject*>(aStatus);
                                                   m_valid = sts->contains("running");
                                                   if (m_valid)
                                                       if (!sts->value("running").toBool()){
                                                           if (aFirstTry){
                                                               auto cmd = "\"" + QDir::currentPath() + "/minIO/storage_start.bat\"";
                                                                       //system("start /b minio.exe server storage");
                                                               system(cmd.toStdString().c_str());
                                                           }
                                                           std::this_thread::sleep_for(std::chrono::microseconds(1000));
                                                       }
                                               }));*/
   auto cmd = "\"" + QDir::currentPath() + "/minIO/storage_start.bat\"";
   //system("start /b minio.exe server storage");
   system(cmd.toStdString().c_str());
   while (!m_valid){
       std::this_thread::sleep_for(std::chrono::milliseconds(1000));
   }
}

void AWSClient::initialize(std::function<bool(void)> aStartService, const QJsonObject& aConfig){
    m_valid = aStartService();
    QDir dir("minIO");
    m_local_storage = dir.exists();

    QJsonObject cfg;
    if (m_local_storage){
        //tryStartMinIO(true);
        if (!m_valid){
            std::cout << "minIO start failed!" << std::endl;
            return;
        }

        /*QString cfg_pth = "storage/.minio.sys/config/config.json";
        while (!dir.exists(cfg_pth))
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        QFile minio_cfg(cfg_pth);
        minio_cfg.open(QFile::ReadOnly);
        auto cfg = QJsonDocument().fromJson(minio_cfg.readAll()).object();
        auto credential = cfg.value("credential").toObject();*/
        m_ip_port = rea::GetLocalIP().toString().toStdString() + ":9000";
        m_secret_key = "deepsight";//credential.value("secretKey").toString();
        m_access_key = "deepsight"; //credential.value("accessKey").toString();
        //minio_cfg.close();
    }else{
        QString s3_ip_port = aConfig.value("ip").toString(); //"http://192.168.1.122:9000";
        QString s3_access_key = aConfig.value("access").toString(); //"YKFERVMC3AK54Y1X150B";
        QString s3_secret_key = aConfig.value("secret").toString(); //"4y0PzVzrvyiE6RzssCbrgO7HCxPIDRrK2pO1qJ5C";
    }

    Aws::SDKOptions options;
    //options.loggingOptions.logLevel = Aws::Utils::Logging::LogLevel::Trace;
    Aws::InitAPI(options);

    Aws::Client::ClientConfiguration config;
    config.endpointOverride = m_ip_port.data();
    config.enableEndpointDiscovery = true;
    config.scheme = Aws::Http::Scheme::HTTP;
    config.verifySSL = false;
    config.region = "us-east-1";
    config.followRedirects = true;
    //config.connectTimeoutMs = 1000;
    //config.requestTimeoutMs = 1000;

    // Create the bucket
    client_ = std::make_shared<Aws::S3::S3Client>(
        Aws::Auth::AWSCredentials(m_access_key.data(), m_secret_key.data()), config,
        Aws::Client::AWSAuthV4Signer::PayloadSigningPolicy::Always, false);
}

AWSClient::AWSClient(){

}

AWSClient::~AWSClient(){
    Aws::SDKOptions options;
    Aws::ShutdownAPI(options);
    if (m_local_storage){
        //auto cmd = "\"" + QDir::currentPath() + "/minIO/storage_stop.bat\"";
        //system(cmd.toStdString().c_str());
        //system("taskkill /im minio.exe /f");
    }
}

bool AWSClient::create_bucket(const Aws::String &bucket_name,
                   const Aws::S3::Model::BucketLocationConstraint &region) {
    if (!m_valid)
        return false;
    // Set up the request
    Aws::S3::Model::CreateBucketRequest request;
    request.SetBucket(bucket_name);

    // Is the region other than us-east-1 (N. Virginia)?
    if (region != Aws::S3::Model::BucketLocationConstraint::us_east_1) {
        // Specify the region as a location constraint
        Aws::S3::Model::CreateBucketConfiguration bucket_config;
        bucket_config.SetLocationConstraint(region);
        request.SetCreateBucketConfiguration(bucket_config);
    }

    auto outcome = client_->CreateBucket(request);
    if (outcome.IsSuccess())
        std::cout << "Create: " << bucket_name << " Success" << std::endl;
    else{
        auto err = outcome.GetError();
        std::cout << "Create: " << bucket_name << "Error: " << err.GetExceptionName() << ": "
                  << err.GetMessage() << std::endl;
        return false;
    }
    return true;
}

bool AWSClient::delete_bucket(const Aws::String &bucket_name)
{
    if (!m_valid)
        return false;
  /*  auto outcome0 = client_->ListBuckets();

    if (outcome0.IsSuccess())
    {
        std::cout << "Your Amazon S3 buckets:" << std::endl;

        Aws::Vector<Aws::S3::Model::Bucket> bucket_list =
            outcome0.GetResult().GetBuckets();

        for (auto const &bucket : bucket_list)
        {
            std::cout << "  * " << bucket.GetName() << std::endl;
        }
    }
    else
    {
        std::cout << "ListBuckets error: "
                  << outcome0.GetError().GetExceptionName() << " - "
                  << outcome0.GetError().GetMessage() << std::endl;
    }*/

    Aws::S3::Model::DeleteBucketRequest bucket_request;
//    bucket_request.SetBucket(bucket_name);
    bucket_request.WithBucket(bucket_name);

    auto outcome = client_->DeleteBucket(bucket_request);

    if (outcome.IsSuccess())
    {
        std::cout << "Delete: " << bucket_name << " Success" << std::endl;
        return true;
    }
    else
    {
        std::cout << "Delete: " << bucket_name <<" Error: "
                  << outcome.GetError().GetExceptionName() << ": "
                  << outcome.GetError().GetMessage() << std::endl;
        return false;
    }
}

bool AWSClient::put_s3_string_object(const Aws::String& s3_bucket_name, const Aws::String& s3_object_name,
                   const std::shared_ptr<Aws::IOStream> stream)
{
    if (!m_valid)
        return false;
    Aws::S3::Model::PutObjectRequest object_request;

    object_request.SetBucket(s3_bucket_name);
    object_request.SetKey(s3_object_name);
    object_request.SetBody(stream);
    // Put the object
    auto put_object_outcome = client_->PutObject(object_request);
    if (put_object_outcome.IsSuccess()){
        std::cout << "Put: " << s3_object_name << " Success" << std::endl;
    }else{
        auto error = put_object_outcome.GetError();
        std::cout << "Put: " << s3_object_name << "Error: " << error.GetExceptionName() << ": "
                  << error.GetMessage() << std::endl;
        return false;
    }
    return true;
}

char* AWSClient::get_s3_object(const Aws::String &bucket_name, const Aws::String &key_name, int& size)
{
    if (!m_valid)
        return nullptr;
    Aws::S3::Model::GetObjectRequest object_request;
    object_request.WithBucket(bucket_name).WithKey(key_name);
    //object_request.SetKey(key_name);

    // Get the object
    auto get_object_outcome = client_->GetObject(object_request);
    if (get_object_outcome.IsSuccess())
    {
        std::cout << "Get: " << key_name << " Success" << std::endl;
        // Get an Aws::IOStream reference to the retrieved file
        auto &retrieved_file = get_object_outcome.GetResultWithOwnership().GetBody();

        retrieved_file.seekg(0, retrieved_file.end);
        size = retrieved_file.tellg();
        retrieved_file.seekg(0, retrieved_file.beg);

        char * buffer = new char[size];
        retrieved_file.read(buffer, size);

        return buffer;
    }
    else
    {
        auto error = get_object_outcome.GetError();
        std::cout << "Get: " << key_name << " Error: " << error.GetExceptionName() << ": "
                  << error.GetMessage() << std::endl;
        size = 0;
        return nullptr;
    }
}

bool AWSClient::delete_s3_object(const Aws::String &bucket_name, const Aws::String &key_name){
    if (!m_valid)
        return false;
    Aws::S3::Model::DeleteObjectRequest object_request;
    object_request.WithBucket(bucket_name).WithKey(key_name);

    auto delete_object_outcome = client_->DeleteObject(object_request);

    if (delete_object_outcome.IsSuccess())
    {
        std::cout << "Delete: " << key_name << " Success" << std::endl;
        return true;
    }
    else
    {
        std::cout << "Delete: " << key_name <<" Error: " <<
            delete_object_outcome.GetError().GetExceptionName() << ": " <<
            delete_object_outcome.GetError().GetMessage() << std::endl;
        return false;
    }
}

bool AWSClient::list_s3_objects(const Aws::String &bucket_name, const Aws::String &perfix, std::vector<std::string> &list){
    if (!m_valid)
        return false;
    Aws::S3::Model::ListObjectsRequest object_request;
    object_request.WithBucket(bucket_name).WithPrefix(perfix);
    auto list_object_outcome = client_->ListObjects(object_request);
    if (list_object_outcome.IsSuccess()){
        auto contents = list_object_outcome.GetResult().GetContents();
        for (auto i : contents)
            list.push_back(i.GetKey().data());
        return true;
    }else{
        std::cout << "List: " << perfix << " Error: " <<
            list_object_outcome.GetError().GetExceptionName() << ": " <<
            list_object_outcome.GetError().GetMessage() << std::endl;
        return false;
    }
}
