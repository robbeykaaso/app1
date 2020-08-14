#include "reactive2.h"
#include "imagePool.h"
#include "util/cv.h"
#include <ds_rx_camera.h>
#include <QJsonObject>
#include <QJsonDocument>

using camCommandSubject = rx::subjects::subject<ds::DsCameraCommand>;

class rxCameras{
private:
    class cameraInfo{
    public:
        cameraInfo(){}
        cameraInfo(std::shared_ptr<ds::DsRxCamera> aCamera, std::shared_ptr<camCommandSubject> aTrigger){
            m_camera = aCamera;
            trigger = aTrigger;
            m_camera->SetCommandObservable(trigger->get_observable().observe_on(rxcpp::synchronize_new_thread()));
        }
    private:
        std::shared_ptr<camCommandSubject> trigger;
        std::shared_ptr<ds::DsRxCamera> m_camera;
        friend rxCameras;
    };
public:
    rxCameras();
private:
    QHash<QString, cameraInfo> m_cameras;
    QSet<QString> m_model_init;
};

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
            rea::pipeline::run<QString>(nm + "_cameraStated", QString::fromStdString(ds::DsCameraStateTypeString[aState.type]));
        });

        cam->GetFrameObservable()
            .observe_on(rxcpp::observe_on_new_thread())
            .buffer(1)
            .subscribe([nm](std::vector<ds::DsFrameData> aFrames) {
                rea::pipeline::run<std::vector<ds::DsFrameData>>(nm + "_cameraCaptured", aFrames);
            });

        rea::pipeline::add<QString>([](rea::stream<QString>* aInput){
            aInput->out();
        }, rea::Json("name", nm + "_cameraStated"));

        rea::pipeline::add<std::vector<ds::DsFrameData>>([](rea::stream<std::vector<ds::DsFrameData>>* aInput){
            aInput->out();
        }, rea::Json("name", nm + "_cameraCaptured"));

        rea::pipeline::add<std::vector<ds::DsFrameData>>([this, nm](rea::stream<std::vector<ds::DsFrameData>>* aInput){
            auto dt = aInput->data();
            for (auto i : dt){
                auto img = cvMat2QImage(i.image);
                rea::imagePool::cacheImage("camera_" + nm, img);
                if (!m_model_init.contains(nm)){
                    m_model_init.insert(nm);
                    aInput->out<QJsonObject>(rea::Json("width", img.width(),
                                                       "height", img.height(),
                                                       "objects", rea::Json(
                                                                      "img", rea::Json(
                                                                                 "type", "image",
                                                                                 "path", "camera_" + nm)
                                                                          )),
                                             "updateQSGModel_" + nm);
                }else
                    aInput->out<QJsonObject>(rea::Json("obj", "img",
                                                       "key", rea::JArray("path"),
                                                       "val", "camera_" + nm,
                                                       "force", true),
                                             "updateQSGAttr_" + nm);
            }
        }, rea::Json("name", nm + "_cameraShow", "thread", 2))
        ->nextB(0, "updateQSGModel_" + nm, QJsonObject())
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

static rxCameras cameras;
static rea::regPip<int> unit_test([](rea::stream<int>* aInput){
    QString nm = "camera1";
    rea::pipeline::find(nm + "_cameraCaptured")
    ->next(FUNCT(std::vector<ds::DsFrameData>, nm,
        std::cout << "this is demo process!" << std::endl;
        aInput->out<std::vector<ds::DsFrameData>>(aInput->data(), nm + "_cameraShow");
    ))
    ->next(nm + "_cameraShow");
}, QJsonObject(), "unitTest");
