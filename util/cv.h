#ifndef REAL_FRAMEWORK_CV_H_
#define REAL_FRAMEWORK_CV_H_

#include "opencv2/opencv.hpp"
#include "qsgModel.h"

QImage cvMat2QImage(const cv::Mat& mat);
cv::Mat QImage2cvMat(QImage image);
std::vector<std::vector<rea::pointList>> extractCounter(const QPoint& aTopLeft, const QImage& aImage, int aEpsilon, int aThreshold = 50);

struct InterfaceExtractCounter{
public:
    InterfaceExtractCounter(){

    }
    InterfaceExtractCounter(QPoint aTopLeft, QImage aImage, int aEpsilon = 1, int aThreshold = 50){
        topLeft = aTopLeft;
        image = aImage;
        epsilon = aEpsilon;
        threshold = aThreshold;
    }
    QPoint topLeft;
    QImage image;
    int epsilon;
    int threshold;
    std::vector<std::vector<rea::pointList>> counters;
};

#endif
