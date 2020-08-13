#ifndef REAL_FRAMEWORK_CV_H_
#define REAL_FRAMEWORK_CV_H_

#include "opencv2/opencv.hpp"
#include "qsgShow/qsgModel.h"
#include <QImage>

QImage cvMat2QImage(const cv::Mat& mat);
cv::Mat QImage2cvMat(QImage image);
std::vector< std::vector<cv::Point>> extractCounter(const cv::Point& aTopLeft, const QImage& aImage, int aEpsilon);

#endif
