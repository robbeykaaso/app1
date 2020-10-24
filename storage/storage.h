#ifndef REAL_FRAMEWORK_STORAGE_H_
#define REAL_FRAMEWORK_STORAGE_H_

#include "storage0.h"
#include "opencv2/opencv.hpp"


using stgCVMat = rea::stgData<cv::Mat>;

class fsStorage : public rea::fsStorage0{
public:
    fsStorage(const QString& aRoot = "");
protected:
    virtual void writeCVMat(const QString& aPath, const cv::Mat& aData);
    virtual cv::Mat readCVMat(const QString& aPath);
};

#endif
