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
    QJsonObject getROI(const QJsonObject& aTask);
    void setROI(QJsonObject& aTask, const QJsonObject& aROI);
    QJsonObject getDefaultROI();
    bool modifyROI(const QJsonArray& aModification, QJsonObject& aROI, QString& aPath);
};

void roiMode::updateShowConfig(QJsonObject& aConfig) {
    aConfig.remove("text");
    aConfig.insert("id", getTaskJsonPath());
}

void roiMode::initialize(){

    //interface save
    rea::pipeline::add<QJsonArray>([this](rea::stream<QJsonArray>* aInput){
        if (getTaskID() == "")
            return;
        QString cur = getTaskJsonPath();
        QString pth = cur;
        auto roi = getROI(*m_task);
        if (modifyROI(aInput->data(), roi, pth)){
            setROI(*m_task, roi);
            aInput->out<QJsonObject>(rea::Json("visible", true, "count", getShapes(roi).size()), "updateROIGUI");
            aInput->out<stgJson>(stgJson(*m_task, pth), "deepsightwriteJson");
        }else if (pth != cur){
            aInput->cache<QJsonArray>(aInput->data())->out<stgJson>(stgJson(QJsonObject(), pth));
        }
    }, rea::Json("name", "roi_tryModifyCurrentModel"))
        ->next("updateROIGUI");

    //interface save
    rea::pipeline::add<stgJson>([this](rea::stream<stgJson>* aInput){
        auto dt = aInput->data().getData();
        auto pth = QString(aInput->data());
        auto roi = getROI(dt);
        modifyROI(aInput->cacheData<QJsonArray>(0), roi, pth);
        setROI(dt, roi);
    }, rea::Json("name", "roi_modifyRemoteModel"));

    //interface show
    rea::pipeline::add<stgCVMat>([this](rea::stream<stgCVMat>* aInput){
        auto dt = aInput->data();
        auto ch = aInput->cacheData<QHash<QString, int>>(0).value(dt);
        auto add_show = showQSGModel(ch, dt);
        if (ch == getShowCount() - 1){
            if (add_show)
                add_show();
            rea::pipeline::run<stgJson>("deepsightreadJson", stgJson(QJsonObject(), getImageResultJsonPath()), rea::Json("tag", "updateImageResult"));
        }
    }, rea::Json("name", "roi_showQSGModel"));

    //interface param
    rea::pipeline::find("startJob")
        ->next(rea::pipeline::add<QJsonObject>([this](rea::stream<QJsonObject>* aInput){
            auto prm = aInput->data().value("param").toObject();
            setROI(prm, getROI(*m_task));
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
        for (int i = shps.size() - 1; i > cnt - 1; --i)
            dels.push_back("roi_" + QString::number(i));
        if (dels.size() > 0)
            aInput->out<QJsonArray>(dels, "taskimage_gridder0_deleteShapes");
        auto id = getTaskJsonPath();
        for (int i = shps.size(); i < cnt; ++i){
            auto shp = "roi_" + QString::number(i);
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
}

roiMode::roiMode(task* aParent) : taskMode(aParent){

}

std::function<void(void)> roiMode::prepareQSGObjects(QJsonObject& aObjects){
    auto roi = getROI(*m_task);
    auto shps = getShapes(roi);
    if (shps.empty()){
        shps.insert("roi_0", getDefaultROI());
        setShapes(roi, shps);
        setROI(*m_task, roi);
    }
    for (auto i : shps.keys())
        aObjects.insert(i, shps.value(i));
    auto roi_cfg = rea::Json("count", shps.size(), "visible", true);
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
                           aInput->out<stgJson>(stgJson(QJsonObject(), pths[0].toString()), "readJson");
                       }
                   }), rea::Json("tag", "setJobParameter"))
            ->next(rea::local("readJson", rea::Json("thread", 10)))
            ->next(rea::pipeline::add<stgJson>([this](rea::stream<stgJson>* aInput){
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
