#include "reactive2.h"
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
