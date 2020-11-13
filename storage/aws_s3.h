#ifndef REAL_APPLICATION2_AWS_S3_H_
#define REAL_APPLICATION2_AWS_s3_H_

#include <aws/core/Aws.h>
#include <aws/core/auth/AWSCredentialsProvider.h>
#include <aws/s3/S3Client.h>
#include <iostream>
#include <fstream>
#include <QJsonObject>

class AWSClient{
public:
    AWSClient();
    ~AWSClient();
    void initialize(std::function<bool(void)> aStartService, const QJsonObject& aConfig);
    bool create_bucket(const Aws::String &bucket_name,
                       const Aws::S3::Model::BucketLocationConstraint &region =
                           Aws::S3::Model::BucketLocationConstraint::us_east_1);
    bool delete_bucket(const Aws::String &bucket_name);
    bool put_s3_string_object(const Aws::String& s3_bucket_name,
                       const Aws::String& s3_object_name,
                       const std::shared_ptr<Aws::IOStream> stream);
    char* get_s3_object(const Aws::String &bucket_name,
                       const Aws::String &key_name,
                       int& size);
   // bool put_s3_image_object(const Aws::String& s3_bucket_name,
   //                          const Aws::String& s3_object_name,
   //                          const std::shared_ptr<Aws::> stream);
    bool delete_s3_object(const Aws::String &bucket_name, const Aws::String &key_name);
    bool list_s3_objects(const Aws::String &bucket_name, const Aws::String &perfix, std::vector<std::string> &list);
    std::string getIPPort() { return m_ip_port;}
    std::string getAccessKey() { return m_access_key;}
    std::string getSecretKey() { return m_secret_key;}
    bool isValid() {return m_valid;}
private:
    void tryStartMinIO(bool aFirstTry);
    std::shared_ptr<Aws::S3::S3Client> client_;
    std::string m_ip_port = "http://192.168.1.122:9000";
    std::string m_access_key = "YKFERVMC3AK54Y1X150B";
    std::string m_secret_key = "4y0PzVzrvyiE6RzssCbrgO7HCxPIDRrK2pO1qJ5C";
    bool m_valid = false;
};

#endif
