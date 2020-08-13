#include "cv.h"

QImage cvMat2QImage(const cv::Mat& mat)
{
    if (mat.type() == CV_8UC1)
    {
        QImage image(mat.cols, mat.rows, QImage::Format_Indexed8);
        image.setColorCount(256);
        for (int i = 0; i < 256; i++)
        {
            image.setColor(i, qRgb(i, i, i));
        }
        uchar *pSrc = mat.data;
        for (int row = 0; row < mat.rows; row++)
        {
            uchar *pDest = image.scanLine(row);
            memcpy(pDest, pSrc, mat.cols);
            pSrc += mat.step;
        }
        // cv::Mat dst;
        // cv::cvtColor(mat, dst, cv::COLOR_GRAY2RGB);
        //  return QImage(dst.data, dst.cols, dst.rows, dst.step, QImage::Format_RGB888).copy();
        //return image.convertToFormat(QImage::Format_RGB888);
        return image;
    }
    else if (mat.type() == CV_8UC3)
    {
        // const uchar *pSrc = (const uchar*)mat.data;
        cv::Mat dst;
        cv::cvtColor(mat, dst, cv::COLOR_BGR2RGB);
        auto image = QImage(dst.data, mat.cols, mat.rows, mat.step, QImage::Format_RGB888).copy();
        return image.rgbSwapped();
    }
    else if (mat.type() == CV_8UC4)
    {
        const uchar *pSrc = (const uchar*)mat.data;
        QImage image(pSrc, mat.cols, mat.rows, mat.step, QImage::Format_ARGB32);
        return image.copy();
    }
    else
    {
        return QImage();
    }
}

cv::Mat QImage2cvMat(QImage image)
{
    cv::Mat mat;
    //    qDebug() << image.format();
    switch(image.format())
    {
    case QImage::Format_ARGB32:
    case QImage::Format_RGB32:
    case QImage::Format_ARGB32_Premultiplied:
        mat = cv::Mat(image.height(), image.width(), CV_8UC4, (void*)image.constBits(), image.bytesPerLine());
        break;
    case QImage::Format_RGB888:
        mat = cv::Mat(image.height(), image.width(), CV_8UC3, (void*)image.constBits(), image.bytesPerLine());
        cv::cvtColor(mat, mat, cv::COLOR_BGR2RGB);
        break;
    case QImage::Format_Indexed8:
        mat = cv::Mat(image.height(), image.width(), CV_8UC1, (void*)image.constBits(), image.bytesPerLine());
        break;
    }
    return mat;
}

std::vector< std::vector<cv::Point>> extractCounter(const cv::Point& aTopLeft, const QImage& aImage, int aEpsilon){
    auto img = QImage2cvMat(aImage);
    std::vector< std::vector<cv::Point>> contours;
    cv::cvtColor(img, img, cv::COLOR_BGRA2GRAY);
    cv::threshold(img, img, 50, 255, cv::THRESH_BINARY);
    cv::findContours(
        img,
        contours,
        cv::noArray(),
        cv::RETR_CCOMP,
        cv::CHAIN_APPROX_SIMPLE,
        {aTopLeft.x - 1, aTopLeft.y - 1}
    );

    for (int i = 0; i < contours.size(); ++i){
        //epsilon = 0.1*cv.arcLength(cnt,True)
        std::vector<cv::Point> ret2;
        cv::approxPolyDP(contours[i], ret2, aEpsilon, true);
        ret2.push_back(ret2[0]);
        contours[i] = ret2;
    }
    return contours;
}
