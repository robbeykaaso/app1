#include "task.h"
#include "command.h"

////////////////////////////////////////////////////////roi//////////////////////////////////////////////////////////
class roiMode : public taskMode{
public:
    roiMode(task* aParent);
    void initialize() override;
protected:
    std::function<void(void)> prepareQSGObjects(QJsonObject& aObjects) override;
    void updateShowConfig(QJsonObject& aConfig) override;
private:
    QJsonObject getImages();
    QJsonObject getROI(const QJsonObject& aTask);
    void setROI(QJsonObject& aTask, const QJsonObject& aROI);
    QJsonObject getDefaultROI();
    bool modifyROI(const QJsonArray& aModification, QJsonObject& aROI, QString& aPath);
    int getROICount(const QJsonObject& aROI, const QString& aTag);
    QJsonObject parseParam(const QJsonObject& aROI);
    bool m_local = false;
};

QJsonObject roiMode::getImages(){
    return m_task->value("images").toObject();
}

int roiMode::getROICount(const QJsonObject& aROI, const QString& aTag){
    auto shps = getShapes(aROI);
    auto tg = "roi_" + aTag + "_";
    int cnt = 0;
    while (shps.contains(tg + QString::number(cnt)))
        cnt++;
    return cnt;
}

QJsonObject roiMode::parseParam(const QJsonObject& aROI){
    auto img = getImageData();
    auto w = img.value("width").toDouble(), h = img.value("height").toDouble();
    auto shps = getShapes(aROI);
    QHash<QString, QVector<QJsonArray>> rois;
    for (auto i : shps.keys()){
        auto pts = shps.value(i).toObject().value("points").toArray();
        pts = pts[0].toArray();
        std::vector<double> val = {pts[0].toDouble(), pts[1].toDouble(), pts[4].toDouble(), pts[5].toDouble()};
        rea::tryFind(&rois, i.split("_")[1])->push_back(rea::JArray(val[0] / w, val[1] / h, (val[2] - val[0]) / w, (val[3] - val[1]) / h));
    }
    QJsonObject ret;
    for (auto i : rois.keys()){
        QJsonArray ret0;
        for (auto j : rois.value(i))
            ret0.push_back(j);
        ret.insert(i, ret0);
    }
    return ret;
}

void roiMode::updateShowConfig(QJsonObject& aConfig) {
    aConfig.remove("text");
    aConfig.insert("id", getTaskJsonPath());
}

void roiMode::initialize(){

    //interface save
    rea::pipeline::add<QJsonArray>([this](rea::stream<QJsonArray>* aInput){
        //if (getTaskID() == "")
        //    return;
        QString cur = getTaskJsonPath();
        QString pth = cur;
        auto roi = getROI(*m_task);
        if (modifyROI(aInput->data(), roi, pth)){
            belongThisMode("roi", pth);
            if (getTaskID() != ""){
                setROI(*m_task, roi);
                if (isCurrentMode("roi"))
                    aInput->out<QJsonObject>(rea::Json("visible", true, "count", getROICount(roi, m_local ? getImageID() : "whole")), "updateROIGUI");
                aInput->out<rea::stgJson>(rea::stgJson(*m_task, pth), s3_bucket_name() + "writeJson");
            }
        }else if (pth != cur && belongThisMode("roi", pth)){
            aInput->cache<QJsonArray>(aInput->data())->out<rea::stgJson>(rea::stgJson(QJsonObject(), pth));
        }
    }, rea::Json("name", "roi_tryModifyCurrentModel"))
        ->next("updateROIGUI");

    //interface remote save
    rea::pipeline::add<rea::stgJson>([this](rea::stream<rea::stgJson>* aInput){
        auto dt = aInput->data().getData();
        auto pth = QString(aInput->data());
        auto roi = getROI(dt);
        modifyROI(aInput->cacheData<QJsonArray>(0), roi, pth);
        setROI(dt, roi);
        aInput->out<rea::stgJson>(rea::stgJson(dt, pth), s3_bucket_name() + "writeJson");
    }, rea::Json("name", "roi_modifyRemoteModel"));

    //interface show
    rea::pipeline::add<stgCVMat>([this](rea::stream<stgCVMat>* aInput){
        auto dt = aInput->data();
        auto ch = aInput->cacheData<int>(0);
        auto add_show = showQSGModel(ch, dt);
        if (ch == getShowCount() - 1){
            if (add_show)
                add_show();
            rea::pipeline::run<rea::stgJson>(s3_bucket_name() + "readJson", rea::stgJson(QJsonObject(), getImageResultJsonPath()), rea::Json("tag", "updateImageResult"));
        }
    }, rea::Json("name", "roi_showQSGModel"));

    //interface param
    rea::pipeline::find("startJob")
        ->next(rea::pipeline::add<QJsonObject>([this](rea::stream<QJsonObject>* aInput){
            auto prm = aInput->data().value("param").toObject();
            prm.insert("roi", parseParam(getROI(*m_task)));
            aInput->setData(rea::Json(aInput->data(), "param", prm));
        }, rea::Json("name", "collect_roi_param")));

    //close roi panel
    rea::pipeline::find("updateQSGCtrl_taskimage_gridder0")
        ->next(rea::pipeline::add<QJsonArray>([](rea::stream<QJsonArray>* aInput){
            if (!aInput->data().empty()){
                if (aInput->data()[0].toObject().value("type") != "roi")
                    aInput->out<QJsonObject>(QJsonObject(), "updateROIGUI");
            }
        }))
        ->next("updateROIGUI");

    //update ROI count
    rea::pipeline::add<double>([this](rea::stream<double>* aInput){
        auto cnt = int(aInput->data());
        auto roi = getROI(*m_task);
        auto shps = getShapes(roi);
        QJsonArray dels;
        auto tag = m_local ? getImageID() : "whole";
        auto cur = getROICount(roi, tag);
        for (int i = cur - 1; i > cnt - 1; --i)
            dels.push_back("roi_" + tag + "_" + QString::number(i));
        if (dels.size() > 0)
            aInput->out<QJsonArray>(dels, "taskimage_gridder0_deleteShapes");
        auto id = getTaskJsonPath();
        for (int i = cur; i < cnt; ++i){
            auto shp = "roi_" + tag + "_" + QString::number(i);
            auto val = getDefaultROI();
            auto ad = [shp, id, val](){
                rea::pipeline::run("updateQSGAttr_taskimage_gridder0",
                                   rea::Json("key", rea::JArray("objects"),
                                             "type", "add",
                                             "tar", shp,
                                             "val", val,
                                             "cmd", true,
                                             "id", id)
                                                                                                                                                      );
            };
            auto rm = [shp, id](){
                rea::pipeline::run("updateQSGAttr_taskimage_gridder0",
                                   rea::Json("key", rea::JArray("objects"),
                                             "type", "del",
                                             "tar", shp,
                                             "cmd", true,
                                             "id", id));
            };
            ad();
            aInput->out<rea::ICommand>(rea::ICommand(ad, rm), "addCommand");
        }
    }, rea::Json("name", "updateROICount"))
        ->nextB("taskimage_gridder0_deleteShapes")
        ->next("addCommand");

    //update roi local mode
    rea::pipeline::add<bool>([this](rea::stream<bool>* aInput){
        m_local = aInput->data();
        auto roi = getROI(*m_task);
        auto shps = getShapes(roi);
        auto roi_img = "roi_" + getImageID() + "_";
        bool sv = false;
        if (m_local){
            if (!shps.contains(roi_img + "0")){
                shps.insert(roi_img + "0", getDefaultROI());
                sv = true;
            }
        }else{
            int idx = 0;
            do{
                auto k = roi_img + QString::number(idx++);
                if (shps.contains(k))
                    shps.remove(k);
                else
                    break;
                sv = true;
            }while(1);
        }
        if (sv){
            setShapes(roi, shps);
            setROI(*m_task, roi);
            aInput->out<rea::stgJson>(rea::stgJson(*m_task, getTaskJsonPath()), s3_bucket_name() + "writeJson");
        }
        updateCurrentImage();
        aInput->out<QJsonArray>(QJsonArray(), "task_image_listViewSelected");
    }, rea::Json("name", "updateROILocalMode"))
        ->nextB(s3_bucket_name() + "writeJson")
        ->next("task_image_listViewSelected", rea::Json("tag", "manual"));

    //auto roi
    rea::pipeline::add<QJsonObject>([this](rea::stream<QJsonObject>* aInput){
        auto imgs = getImages();
        QJsonArray lst;
        for (auto i : imgs.keys())
            lst.push_back(i);
        aInput->setData(rea::Json("roi", getShapes(getROI(*m_task)), "images", lst, "root", "project/" + getProjectID() + "/image/"))->out();
    }, rea::Json("name", "autoROI"))
        ->next("calcAutoROI")
        ->next(rea::pipeline::add<QJsonObject>([this](rea::stream<QJsonObject>* aInput){
            auto dt = aInput->data();
            auto roi = getROI(*m_task);
            setShapes(roi, dt.value("roi").toObject());
            setROI(*m_task, roi);
            aInput->out<rea::stgJson>(rea::stgJson(*m_task, getTaskJsonPath()), s3_bucket_name() + "writeJson");
            updateCurrentImage();
            aInput->out<QJsonArray>(QJsonArray(), "task_image_listViewSelected");
        }))
        ->nextB(s3_bucket_name() + "writeJson")
        ->next("task_image_listViewSelected", rea::Json("tag", "manual"));
}

roiMode::roiMode(task* aParent) : taskMode(aParent){

}

std::function<void(void)> roiMode::prepareQSGObjects(QJsonObject& aObjects){
    auto roi = getROI(*m_task);
    auto shps = getShapes(roi);

    m_local = false;
    auto tag = getImageID();
    int idx = 0;
    do{
        auto shp_id = "roi_" + tag + "_" + QString::number(idx++);
        if (!shps.contains(shp_id))
            break;
        aObjects.insert(shp_id, shps.value(shp_id));
        m_local = true;
    }while(1);

    if (aObjects.size() == 1){
        QString tag = "whole";
        int idx = 0;
        do{
            auto shp_id = "roi_" + tag + "_" + QString::number(idx++);
            if (!shps.contains(shp_id))
                break;
            aObjects.insert(shp_id, shps.value(shp_id));
        }while(1);
    }

    if (aObjects.size() == 1){
        auto dflt = getDefaultROI();
        shps.insert("roi_whole_0", dflt);
        aObjects.insert("roi_whole_0", dflt);
        setShapes(roi, shps);
        setROI(*m_task, roi);
    }

    auto roi_cfg = rea::Json("count", aObjects.size() - 1, "visible", true, "l", m_local);
    return [roi_cfg](){
        rea::pipeline::run<QJsonObject>("updateROIGUI", roi_cfg);
    };
}

QJsonObject roiMode::getROI(const QJsonObject& aTask){
    return aTask.value("roi").toObject();
}

void roiMode::setROI(QJsonObject& aTask, const QJsonObject& aROI){
    aTask.insert("roi", aROI);
}

QJsonObject roiMode::getDefaultROI(){
    auto img = getImageData();
    auto w = img.value("width").toInt(), h = img.value("height").toInt();
    return rea::Json("type", "poly", "color", "pink", "points", rea::JArray(QJsonArray(), rea::JArray(0, 0, w, 0, w, h, 0, h, 0, 0)));
}

bool roiMode::modifyROI(const QJsonArray& aModification, QJsonObject& aROI, QString& aPath){
    bool modified = false;
    for (auto i : aModification){
        auto dt = i.toObject();
        if (dt.value("cmd").toBool()){
            if (dt.contains("id") && dt.value("id") != aPath){
                aPath = dt.value("id").toString();
                return false;
            }
            auto shps = getShapes(aROI);
            if (dt.value("key") == QJsonArray({"objects"})){
                if (dt.value("type") == "add"){
                    shps.insert(dt.value("tar").toString(), dt.value("val").toObject());
                    setShapes(aROI, shps);
                    modified = true;
                }else if (dt.value("type") == "del"){
                    shps.remove(dt.value("tar").toString());
                    setShapes(aROI, shps);
                    modified = true;
                }
            }else{
                auto nm = dt.value("obj").toString();
                if (shps.contains(nm)){
                    auto shp = shps.value(nm).toObject();
                    auto key = dt.value("key").toArray()[0].toString();
                    shp.insert(key, dt.value("val"));
                    shps.insert(nm, shp);
                    setShapes(aROI, shps);
                    modified = true;
                }
            }
        }
    }
    return modified;
}

static rea::regPip<QQmlApplicationEngine*> init_roi_mode([](rea::stream<QQmlApplicationEngine*>* aInput){
    rea::pipeline::find("loadCustomModes")
        ->next(rea::pipeline::add<task*>([](rea::stream<task*>* aInput){
            auto md = new roiMode(aInput->data());
            md->initialize();
            aInput->out<QString>("roi", "customModesLoaded");
        }))
        ->next("customModesLoaded");
    aInput->out();
}, rea::Json("name", "install2_roi_mode"), "regQML");

/////////////////////////////////////////////////////////modelParam//////////////////////////////////////////////////////////

class modelParam{
public:
    modelParam(){
        //interface param
        rea::pipeline::find("startJob")
            ->next(rea::pipeline::add<QJsonObject>([this](rea::stream<QJsonObject>* aInput){
                auto prm = aInput->data().value("param").toObject();
                for (auto i : m_current_param.keys())
                    prm.insert(i, m_current_param.value(i));
                aInput->setData(rea::Json(aInput->data(), "param", prm));
            }, rea::Json("name", "collect_model_param")));

        //set job parameter
        rea::pipeline::find("_selectFile")
            ->next(rea::pipeline::add<QJsonArray>([](rea::stream<QJsonArray>* aInput){
                       auto pths = aInput->data();
                       if (pths.size() > 0){
                           aInput->out<rea::stgJson>(rea::stgJson(QJsonObject(), pths[0].toString()), "readJson");
                       }
                   }), rea::Json("tag", "setJobParameter"))
            ->next(rea::local("readJson", rea::Json("thread", 10)))
            ->next(rea::pipeline::add<rea::stgJson>([this](rea::stream<rea::stgJson>* aInput){
                m_current_param = aInput->data().getData();
            }));
        rea::pipeline::find("editJobParam")
            ->next(rea::pipeline::add<QJsonObject>([this](rea::stream<QJsonObject>* aInput){
                m_current_param = aInput->data();
            }));
    }
private:
    QJsonObject m_current_param;
};

static rea::regPip<QQmlApplicationEngine*> init_model_param([](rea::stream<QQmlApplicationEngine*>* aInput){
    static modelParam prm;
},QJsonObject(), "regQML");
