#include "camera.h"
#include "imagePool.h"
#include <QImage>

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

using camCommandSubject = rx::subjects::subject<ds::DsCameraCommand>;

rxCameras::rxCameras(){
    rea::pipeline::add<QJsonObject>([this](rea::stream<QJsonObject>* aInput){
        auto cfg = aInput->data();
        auto nm = cfg.value("name").toString();
        if (m_cameras.contains(nm))
            return;

        QString strJson(QJsonDocument(cfg).toJson(QJsonDocument::Compact));

        rapidjson::Document config;
        if (config.Parse(strJson.toStdString().c_str()).HasParseError()) {
            return;
        }

        auto cam = std::make_shared<ds::DsRxCamera>(config["initial"]);
        m_cameras.insert(nm, cameraInfo(cam, std::make_shared<camCommandSubject>()));

        cam->GetStateObservable().subscribe([nm](ds::DsCameraState aState) {
            rea::pipeline::run<QJsonObject>(nm + "_cameraStated", rea::Json("state", QString::fromStdString(ds::DsCameraStateTypeString[aState.type])));
        });

        cam->GetFrameObservable()
            .observe_on(rxcpp::observe_on_new_thread())
            .buffer(1)
            .subscribe([nm](std::vector<ds::DsFrameData> aFrames) {
                rea::pipeline::run<std::vector<ds::DsFrameData>>(nm + "_cameraCaptured", aFrames);
            });

        rea::pipeline::add<QJsonObject>([](rea::stream<QJsonObject>* aInput){
            aInput->out();
        }, rea::Json("name", nm + "_cameraStated"));

        rea::pipeline::add<std::vector<ds::DsFrameData>>([](rea::stream<std::vector<ds::DsFrameData>>* aInput){
            auto dt = aInput->data();
            aInput->out();
        }, rea::Json("name", nm + "_cameraCaptured"))
        ->next(rea::pipeline::add<std::vector<ds::DsFrameData>>([nm](rea::stream<std::vector<ds::DsFrameData>>* aInput){
                static int counter = 0;

            auto dt = aInput->data();
            for (auto i : dt){
                auto img = cvMat2QImage(i.image);
                rea::imagePool::cacheImage("camera_" + QString::number(counter), img);
                if (!counter){
                    aInput->out<QJsonObject>(rea::Json("width", img.width(),
                                                       "height", img.height(),
                                                       "objects", rea::Json(
                                                                      "img", rea::Json(
                                                                                 "type", "image",
                                                                                 "path", "camera_" + QString::number(counter))
                                                                          )),
                                             "replaceQSGModel_" + nm);
                }else
                    aInput->out<QJsonObject>(rea::Json("obj", "img", "key", rea::JArray("path"), "val", "camera_" + QString::number(counter)), "updateQSGAttr_" + nm);
                counter++;
            }
        }, rea::Json("name", nm + "_cameraDemoShow", "thread", 2)))
        ->nextB(0, "replaceQSGModel_" + nm, QJsonObject())
        ->next("updateQSGAttr_" + nm);

        rea::pipeline::add<QJsonObject>([this, nm](rea::stream<QJsonObject>* aInput){
            auto dt = aInput->data();
            if (m_cameras.contains(nm)){
                auto inf = m_cameras.value(nm);
                if (dt.value("on").toBool()){
                    inf.trigger->get_subscriber().on_next(ds::DsCameraCommand{ds::DsCameraCommandOpen});
                    inf.trigger->get_subscriber().on_next(ds::DsCameraCommand{ds::DsCameraCommandStart});
                }else
                    inf.trigger->get_subscriber().on_next(ds::DsCameraCommand{ds::DsCameraCommandStop});
            }
            aInput->out();
        }, rea::Json("name", nm + "_turnCamera"));

        rea::pipeline::add<QJsonObject, rea::pipeDelegate>([this, nm](rea::stream<QJsonObject>* aInput){
            auto dt = aInput->data();
            if (m_cameras.contains(nm)){
                auto inf = m_cameras.value(nm);
                inf.trigger->get_subscriber().on_next(ds::DsCameraCommand{ds::DsCameraCommandGrabOne});
            }
        }, rea::Json("name", nm + "_captureCamera", "param", rea::Json("delegate", nm + "_cameraCaptured")));

        rea::pipeline::add<QJsonObject>([this, nm](rea::stream<QJsonObject>* aInput){
            auto dt = aInput->data();
            if (m_cameras.contains(nm)){
                auto inf = m_cameras.value(nm);
                QString prm(QJsonDocument(dt.value("param").toObject()).toJson(QJsonDocument::Compact));
                inf.trigger->get_subscriber().on_next(ds::DsCameraCommand{ds::DsCameraCommandSet, 0, 0, prm.toStdString()});
            }
            aInput->out();
        }, rea::Json("name", nm + "_setCamera"));

    }, rea::Json("name", "addCamera"));
}

static rea::regPip<int> unit_test([](rea::stream<int>* aInput){
    static rxCameras cameras;

    /*const auto cam_cfg = rea::Json("name", "camera1",
                                   "initial", rea::Json(//"directory", "./image",
                                                         //"filepattern", "^((?!\\/\\.).)*\\.(?:png|bmp|jpg|jpeg$)",
                                                         //"type", "cam_sim",
                                                         //"loopable", true,
                                                  "config", "W:/configs/MV-CE200-10GC.mfs",
                                                  "type", "cam_hikvision",
                                                  //"id", "MV-CE200-10GC",
                                                  "id", "DS",
                                                  "name", "Ds MVCE200 10 GC Camera #1"));*/
    //rea::pipeline::run<QJsonObject>("addCamera", cam_cfg);
    //rea::pipeline::run<QJsonObject>("turnCamera", rea::Json("name", "camera1", "on", true));
    //rea::pipeline::run<QJsonObject>("setCamera", rea::Json("triggermode", 0));
    //rea::pipeline::run<QJsonObject>("captureCamera", rea::Json("name", "camera1"));
}, QJsonObject(), "unitTest");
