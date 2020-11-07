#include "cv.h"

QImage cvMat2QImage(const cv::Mat& mat)
{
    if (mat.type() == CV_8UC1)
    {
        QImage image(mat.cols, mat.rows, QImage::Format_Indexed8);
        if (!image.isNull()){
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
        //return image.rgbSwapped();
        return image;
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
    case QImage::Format_ARGB32_Premultiplied:{
        mat = cv::Mat(image.height(), image.width(), CV_8UC4, (void*)image.constBits(), image.bytesPerLine());
        break;
    }
    case QImage::Format_RGB888:{
        mat = cv::Mat(image.height(), image.width(), CV_8UC3, (void*)image.constBits(), image.bytesPerLine());
        //        cv::imwrite("xxxxxx.png", mat);
        cv::cvtColor(mat, mat, cv::COLOR_RGB2BGR);
        break;
    }
    case QImage::Format_Indexed8:
        mat = cv::Mat(image.height(), image.width(), CV_8UC1, (void*)image.constBits(), image.bytesPerLine());
        break;
    }
    return mat;
}

std::vector<std::vector<rea::pointList>> extractCounter(const QPoint& aTopLeft, const QImage& aImage, int aEpsilon, int aThreshold){
    auto img = QImage2cvMat(aImage);
    std::vector< std::vector<cv::Point>> contours;
    cv::cvtColor(img, img, cv::COLOR_BGRA2GRAY);
    cv::threshold(img, img, aThreshold, 255, cv::THRESH_BINARY);

    //https://blog.csdn.net/guduruyu/article/details/69220296
    std::vector<cv::Vec4i> topo;
    cv::findContours(img, contours, topo, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE, {aTopLeft.x() - 1, aTopLeft.y() - 1});

    QSet<int> got;
    std::vector<std::vector< std::vector<QPointF>>> ret;
    for (int i = 0; i < topo.size(); ++i)
        if (!got.contains(i)){
            got.insert(i);
            std::vector< std::vector<QPointF>> polys;
            std::vector<cv::Point> poly;
            cv::approxPolyDP(contours[i], poly, aEpsilon, true);
            poly.push_back(poly[0]);
            std::vector<QPointF> ply;
            for (auto i : poly)
                ply.push_back(QPointF(i.x, i.y));
            polys.push_back(ply);

            auto ch = topo[i][2];
            while (ch >= 0){
                auto ch_topo = topo[ch];
                got.insert(ch);
                poly.clear();
                cv::approxPolyDP(contours[ch], poly, aEpsilon, true);
                poly.push_back(poly[0]);
                ply.clear();
                for (auto i : poly)
                    ply.push_back(QPointF(i.x, i.y));
                polys.push_back(ply);
                ch = ch_topo[0];
            }
            ret.push_back(polys);
        }
    return ret;
}

static rea::regPip<InterfaceExtractCounter> init_extract_counter([](rea::stream<InterfaceExtractCounter>* aInput){
    auto dt = aInput->data();
    dt.counters = extractCounter(dt.topLeft, dt.image, dt.epsilon, dt.threshold);
    aInput->setData(dt);
}, rea::Json("name", "extractAllCounters"));
