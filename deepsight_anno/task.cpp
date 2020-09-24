#include "task.h"
#include "imagePool.h"
#include "command.h"
#include "../socket/server.h"
#include "../socket/protocal.h"
#include "../util/cv.h"

bool ITaskFriend::modifyImage(const QJsonArray& aModification, QJsonObject& aImage, QString& aPath){
    return m_task->modifyImage(aModification, aImage, aPath);
}

QString ITaskFriend::getTaskJsonPath(){
    return m_task->getTaskJsonPath();
}

QString ITaskFriend::getImageJsonPath(){
    return "project/" + m_task->m_project_id + "/image/" + m_task->m_current_image + ".json";
}

QJsonObject& ITaskFriend::getImageData(){
    return m_task->m_image;
}

QString ITaskFriend::getImageID(){
    return m_task->m_current_image;
}

QString ITaskFriend::getTaskID(){
    return m_task->m_task_id;
}

QJsonObject ITaskFriend::getTaskLabels(){
    return m_task->getLabels();
}

QJsonObject ITaskFriend::getProjectLabels(){
    return m_task->m_project_labels;
}

QJsonObject ITaskFriend::getImageShow(){
    return m_task->m_image_show;
}

void taskMode::tryModifyCurrentModel(rea::stream<QJsonArray>* aInput){
    if (getImageID() == "")
        return;
    QString cur = getImageJsonPath();
    QString pth = cur;
    if (modifyImage(aInput->data(), getImageData(), pth))
        aInput->out<stgJson>(stgJson(getImageData(), pth), "deepsightwriteJson");
    else if (pth != cur){
        aInput->cache<QJsonArray>(aInput->data())->out<stgJson>(stgJson(QJsonObject(), pth));
    }
}

void taskMode::modifyRemoteModel(rea::stream<stgJson>* aInput){
    auto dt = aInput->data().getData();
    auto pth = QString(aInput->data());
    modifyImage(aInput->cacheData<QJsonArray>(0), dt, pth);
}

std::function<void(void)> taskMode::prepareQSGObjects(QJsonObject &aObjects){
    auto tsk_shp_lbls = getTaskLabels().value("shape").toObject(), prj_shp_lbls = getProjectLabels().value("shape").toObject();

    auto shps = getShapes(getImageData());
    for (auto i : shps.keys()){
        auto shp = shps.value(i).toObject();
        auto lbl = shp.value("label").toString();
        if (lbl == "" || (tsk_shp_lbls.contains(lbl) && prj_shp_lbls.contains(lbl))){
            if (shp.value("type") == "ellipse"){
                aObjects.insert(i, rea::Json("type", "ellipse",
                                             "caption", shp.value("label"),
                                             "center", shp.value("center"),
                                             "radius", rea::JArray(shp.value("xradius"), shp.value("yradius")),
                                             "angle", shp.value("angle")));
            }else if (shp.value("type") == "polyline"){
                QJsonArray pts;
                pts.push_back(shp.value("points"));
                auto holes = shp.value("holes").toArray();
                for (auto i : holes)
                    pts.push_back(i);
                aObjects.insert(i, rea::Json("type", "poly",
                                             "caption", shp.value("label"),
                                             "points", pts));
            }
        }
    }
    return nullptr;
}

std::function<void(void)> taskMode::showQSGModel(int aChannel, stgCVMat& aImage){
    auto pth = QString(aImage);
    auto img = cvMat2QImage(aImage.getData());
    rea::imagePool::cacheImage(pth, img);

    auto objs = rea::Json("img_" + getImageID(), rea::Json(
                                                        "type", "image",
                                                        "range", rea::JArray(0, 0, img.width(), img.height()),
                                                        "path", pth,
                                                        "text", QJsonObject(),
                                                        "transform", getImageShow()));
    std::function<void(void)> add_show = nullptr;
    add_show = prepareQSGObjects(objs);

    auto cfg = rea::Json("id", getImageJsonPath(),
                         "width", img.width(),
                         "height", img.height(),
                         "face", 100,
                         "text", rea::Json("visible", true,
                                           "size", rea::JArray(80, 30)),
                         "objects", objs);
    updateShowConfig(cfg);
    rea::pipeline::run<QJsonObject>("updateQSGModel_taskimage_gridder" + QString::number(aChannel), cfg);
    return add_show;
}

void roiMode::updateShowConfig(QJsonObject& aConfig) {
    aConfig.remove("text");
    aConfig.insert("id", getTaskJsonPath());
}

roiMode::roiMode(task* aParent) : taskMode(aParent){

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
      ->nextB(0, "taskimage_gridder0_deleteShapes", QJsonObject())
      ->next("addCommand");
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

void roiMode::tryModifyCurrentModel(rea::stream<QJsonArray>* aInput){
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
}

void roiMode::modifyRemoteModel(rea::stream<stgJson>* aInput){
    auto dt = aInput->data().getData();
    auto pth = QString(aInput->data());
    auto roi = getROI(dt);
    modifyROI(aInput->cacheData<QJsonArray>(0), roi, pth);
    setROI(dt, roi);
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

void task::setLabels(const QJsonObject& aLabels){
    rea::pipeline::run<QJsonObject>("taskLabelChanged", aLabels);
    insert("labels", aLabels);
}

QString task::getImageStage(const QJsonObject& aImage){
    return aImage.value("stage").toString();
}
void task::setImageStage(QJsonObject& aImage, const QString& aStage){
    aImage.insert("stage", aStage);
}
QJsonObject task::getImages(){
    return value("images").toObject();
}
void task::setImages(const QJsonObject& aImages){
    insert("images", aImages);
}
QJsonArray task::getImageList(){
    if (contains("image_list"))
        return value("image_list").toArray();
    else{
        QJsonArray ret;
        for (auto i : m_project_images->keys())
            ret.push_back(i);
        return ret;
    }
}
void task::setImageList(const QJsonArray& aList, bool aRemove){
    if (aRemove)
        remove("image_list");
    else
        insert("image_list", aList);
}
QJsonObject task::prepareLabelListGUI(const QJsonObject& aLabels){
    QJsonArray data;
    for (auto i : m_project_labels.keys())
        data.push_back(rea::Json("entry", rea::JArray(i, aLabels.contains(i))));
    return rea::Json("title", rea::JArray("group", "used"),
                     "selects", m_project_labels.size() > 0 ? rea::JArray("0") : QJsonArray(),
                     "data", data);
}
QJsonObject task::prepareImageListGUI(const QJsonObject& aImages){
    QJsonArray data;
    auto lst = getImageList();
    for (auto i : lst){
        auto img = i.toString();
        data.push_back(rea::Json("entry", rea::JArray(getImageStringName(m_project_images->value(img).toObject()),
                                                      aImages.contains(img),
                                                      getImageStage(aImages.value(img).toObject()))));
    }
    return rea::Json("title", rea::JArray("name", "used", "stage"),
                     "entrycount", 30,
                     "selects", lst.size() > 0 ? rea::JArray("0") : QJsonArray(),
                     "data", data);
}
QJsonObject task::prepareJobListGUI(){
    QJsonArray data;
    for (auto i : m_jobs){
        auto job = i.toObject();
        data.push_back(rea::Json("entry", rea::JArray(job.value("time"))));
    }
    return rea::Json("title", rea::JArray("time"),
                     "entrycount", 30,
                     "selects", m_jobs.size() > 0 ? rea::JArray("0") : QJsonArray(),
                     "data", data);
}

QString task::getJobsJsonPath(){
    return "project/" + m_project_id + "/task/" + m_task_id + "/jobs.json";
 }
QString task::getTaskJsonPath(){
    return "project/" + m_project_id + "/task/" + m_task_id + ".json";
}
void task::taskManagement(){
    //open project
    rea::pipeline::find("openProject")
        ->next(rea::pipeline::add<QJsonObject>([this](rea::stream<QJsonObject>* aInput){
            m_task_id = "";
        }));

    //open task
    rea::pipeline::add<IProjectInfo>([this](rea::stream<IProjectInfo>* aInput){
        auto dt = aInput->data();
        if (dt.value("id") != m_task_id){
            m_task_id = dt.value("id").toString();
            m_project_labels = dt.value("labels").toObject();
            m_project_images = dt.project_images;
            m_channel_count = dt.value("channel").toInt();
            m_project_id = dt.value("project").toString();
            m_image_show = dt.value("imageshow").toObject();
            aInput->out<stgJson>(stgJson(QJsonObject(), getTaskJsonPath()));
            aInput->out<stgJson>(stgJson(QJsonObject(), getJobsJsonPath()));
        }
    }, rea::Json("name", "openTask"))
        ->next(rea::local("deepsightreadJson", rea::Json("thread", 10)))
        ->next(rea::buffer<stgJson>(2))
        ->next(rea::pipeline::add<std::vector<stgJson>>([this](rea::stream<std::vector<stgJson>>* aInput){
            auto dt = aInput->data();
            dt[0].getData().swap(*this);
            m_jobs = dt[1].getData();
            setLabels(value("labels").toObject());
            m_current_job = "";
            aInput->out<QJsonObject>(prepareLabelListGUI(getLabels()), "task_label_updateListView");
            aInput->out<QJsonArray>(QJsonArray(), "task_label_listViewSelected");
            aInput->out<QJsonObject>(prepareJobListGUI(), "task_job_updateListView");  //the first updateImageResult for cacheImage will not work, but it is necessary here
            aInput->out<QJsonArray>(QJsonArray(), "task_job_listViewSelected");
            aInput->out<QJsonObject>(prepareImageListGUI(getImages()), "task_image_updateListView");
            aInput->out<QJsonObject>(rea::Json("count", 1), "scatterTaskImageShow");
            aInput->out<QJsonObject>(getFilter(), "updateTaskImageFilterGUI");
        }))
        ->nextB(0, "task_label_updateListView", QJsonObject())
        ->nextB(0, "task_label_listViewSelected", rea::Json("tag", "task_manual"))
        ->nextB(0, "task_image_updateListView", QJsonObject())
        ->nextB(0, "scatterTaskImageShow", QJsonObject())
        ->nextB(0, "task_job_updateListView", QJsonObject())
        ->nextB(0, "task_job_listViewSelected", rea::Json("tag", "manual"))
        ->nextB(0, "updateTaskImageFilterGUI", QJsonObject());
}

void task::labelManagement(){
    //update project labels
    rea::pipeline::find("projectLabelChanged")
        ->next(rea::pipeline::add<QJsonObject>([this](rea::stream<QJsonObject>* aInput){
            if (m_task_id == "")
                return;
            auto dt = aInput->data();
            if (m_project_labels.value("shape") != dt.value("shape")){
                m_current_image = "";
            }
            m_project_labels = dt;
            aInput->out<QJsonObject>(prepareLabelListGUI(getLabels()), "task_label_updateListView");
            aInput->out<QJsonArray>(QJsonArray(), "task_label_listViewSelected");
        }))
        ->nextB(0, "task_label_updateListView", QJsonObject())
        ->nextB(0, "task_label_listViewSelected", rea::Json("tag", "task_manual"));

    //select task label group
    rea::pipeline::find("task_label_listViewSelected")
        ->next("getProjectLabel", rea::Json("tag", "task_manual"))
        ->next(rea::pipeline::add<QJsonObject>([this](rea::stream<QJsonObject>* aInput){
               auto dt = aInput->data();
               auto lbls = getLabels();
               auto key = dt.value("key").toString();
               auto val = lbls.value(key).toObject();
               auto proj_val = dt.value("val").toObject();
               for (auto i : val.keys())
                   val.insert(i, proj_val.value(i));
               aInput->out<QJsonObject>(rea::Json("key", key, "val", val), "updateTaskLabelGUI");
               aInput->out<QJsonObject>(dt, "updateCandidateLabelGUI");
           }), rea::Json("tag", "task_manual"))
        ->nextB(0, "updateCandidateLabelGUI", QJsonObject())
        ->next("updateTaskLabelGUI");

    //add/remove label
    auto addLabel = rea::buffer<QJsonArray>(2);
    auto addLabel_nm = "addTaskLabel";
    auto addLabel_tag = rea::Json("tag", addLabel_nm);
    rea::pipeline::add<QJsonObject>([](rea::stream<QJsonObject>* aInput){
        auto dt = aInput->data();
        aInput->out<QJsonArray>(rea::JArray(dt.value("label"), dt.value("add")));
        aInput->out<QJsonArray>(QJsonArray(), "task_label_listViewSelected");
    }, rea::Json("name", addLabel_nm))
        ->nextB(0, addLabel, QJsonObject())
        ->next("task_label_listViewSelected", addLabel_tag)
        ->next(addLabel, addLabel_tag)
        ->next(rea::pipeline::add<std::vector<QJsonArray>>([this](rea::stream<std::vector<QJsonArray>>* aInput){
            auto dt = aInput->data();
            auto lbl = dt[0][0].toString();
            auto add = dt[0][1].toBool();
            auto sel = dt[1][0].toInt();
            auto grps = getLabels();
            auto grp = (m_project_labels.begin() + sel).key();
            auto lbls = grps.value(grp).toObject();
            bool updatelist = !grps.contains(grp);
            if (add){
                if (!lbls.contains(lbl)){
                    lbls.insert(lbl, QJsonObject());
                    grps.insert(grp, lbls);
                }else
                    return;
            }else{
                lbls.remove(lbl);
                if (lbls.size() == 0){
                    grps.remove(grp);
                    updatelist = true;
                }else
                    grps.insert(grp, lbls);
            }
            if (updatelist)
                aInput->out<QJsonObject>(rea::Json(prepareLabelListGUI(grps), "selects", rea::JArray(sel)), "task_label_updateListView");
            setLabels(grps);
            aInput->out<stgJson>(stgJson(*this, getTaskJsonPath()), "deepsightwriteJson");
            aInput->out<QJsonArray>(QJsonArray(), "task_label_listViewSelected");
            m_current_image = "";
            aInput->out<QJsonArray>(QJsonArray(), "task_image_listViewSelected");
        }))
        ->nextB(0, "task_label_updateListView", QJsonObject())
        ->nextB(0, "deepsightwriteJson", QJsonObject())
        ->nextB(0, "task_label_listViewSelected", rea::Json("tag", "task_manual"))
        ->next("task_image_listViewSelected", rea::Json("tag", "manual"));

    //modifyImageLabel
    const QString modifyImageLabel_nm = "modifyTaskImageLabel";
    auto modifyImageLabel_tag = rea::Json("tag", modifyImageLabel_nm);
    rea::pipeline::add<QJsonObject>([](rea::stream<QJsonObject>* aInput){
        aInput->cache<QJsonObject>(aInput->data())->out<QJsonArray>(QJsonArray(), "task_image_listViewSelected");
    }, rea::Json("name", modifyImageLabel_nm))
        ->next("task_image_listViewSelected", modifyImageLabel_tag)
        ->next(rea::pipeline::add<QJsonArray>([this](rea::stream<QJsonArray>* aInput){
                   auto st = aInput->cacheData<QJsonObject>(0);
                   auto sels = aInput->data();
                   auto lst = getImageList();
                   auto imgs = getImages();
                   for (auto i : sels){
                       auto img = lst[i.toInt()].toString();
                       auto abs = m_project_images->value(img).toObject();
                       auto lbls = getImageLabels(abs);
                       lbls.insert(st.value("group").toString(), st.value("label"));
                       setImageLabels(abs, lbls);
                       m_project_images->insert(img, abs);
                   }
                   aInput->out<stgJson>(stgJson(*m_project_images, "project/" + m_project_id + "/image.json"), "deepsightwriteJson");
               }), modifyImageLabel_tag)
        ->next("deepsightwriteJson");

    rea::pipeline::add<QJsonObject>([](rea::stream<QJsonObject>* aInput){
        aInput->out();
    }, rea::Json("name", "taskLabelChanged"));
}

std::shared_ptr<taskMode> task::getCurrentMode(){
    if (m_modes.size() == 0){
        m_modes.insert("", std::make_shared<taskMode>(this));
        m_modes.insert("roi", std::make_shared<roiMode>(this));
    }
    auto ret = m_modes.value(m_current_mode);
    if (!ret)
        ret = m_modes.value("");
    return ret;
}

void task::getResultShapeObjects(QJsonObject& aObjects){

}

void task::imageManagement(){
    //set Custom Mode
    rea::pipeline::find("updateQSGCtrl_taskimage_gridder0")
    ->next(rea::pipeline::add<QJsonArray>([this](rea::stream<QJsonArray>* aInput){
        if (!aInput->data().empty()){
            m_current_mode = aInput->data()[0].toObject().value("type").toString();
            m_current_image = "";
            aInput->out<QJsonArray>(QJsonArray(), "task_image_listViewSelected");
        };
    }))
    ->nextB(0, "task_image_listViewSelected", rea::Json("tag", "manual"));

    //update project images
    rea::pipeline::find("deepsightwriteJson")
    ->next(rea::pipeline::add<stgJson>([this](rea::stream<stgJson>* aInput){
        if (m_task_id == "")
            return;
        //m_project_images = aInput->data().getData();
        aInput->out<QJsonObject>(prepareImageListGUI(getImages()), "task_image_updateListView");
    }), rea::Json("tag", "updateProjectImages"))
    ->nextB(0, "task_image_updateListView", QJsonObject());

    //select image
    rea::pipeline::find("task_image_listViewSelected")
        ->next(rea::pipeline::add<QJsonArray>([this](rea::stream<QJsonArray>* aInput){
               if (m_task_id == "")
                   return;
               auto dt = aInput->data();
               if (dt.size() == 0){
                   rea::pipeline::run<QJsonArray>("updateQSGCtrl_taskimage_gridder0", QJsonArray());  //must be before "updateQSGModel_projectimage_gridder"
                   m_current_image = "";
                   for (int i = 0; i < m_show_count; ++i)
                       rea::pipeline::run<QJsonObject>("updateQSGModel_taskimage_gridder" + QString::number(i), QJsonObject());
                   aInput->out<QJsonObject>(QJsonObject(), "updateTaskImageGUI");
               }
               else{
                   auto lst = getImageList();
                   auto idx = dt[0].toInt();
                   if (idx < lst.size()){
                       auto nm = lst[idx].toString();
                       if (nm != m_current_image){
                           aInput->out<QJsonArray>(QJsonArray(), "updateQSGCtrl_taskimage_gridder0");
                           m_current_image = nm;
                           auto nms = getImageName(m_project_images->value(m_current_image).toObject());
                           aInput->out<stgJson>(stgJson(QJsonObject(), "project/" + m_project_id + "/image/" + nm + ".json"), "deepsightreadJson", selectTaskImage);
                           QHash<QString, int> show_cache;
                           for (auto i = 0; i < m_show_count; ++i){
                               auto img = "project/" + m_project_id + "/image/" + nm + "/" + nms[i].toString();
                               show_cache.insert(img, i);
                               aInput->out<stgCVMat>(stgCVMat(cv::Mat(), img));
                           }
                           aInput->cache<QHash<QString, int>>(show_cache);
                       }
                       //aInput->out<QJsonObject>(m_images.value(nm).toObject(), "updateTaskImageGUI");
                   }else{
                       aInput->out<QJsonObject>(QJsonObject(), "updateTaskImageGUI");
                   }
               }
           }), rea::Json("tag", "manual"))
        ->nextB(0, "updateQSGCtrl_taskimage_gridder0", QJsonObject())
        ->nextB(0, "deepsightreadJson", selectTaskImage, rea::pipeline::add<stgJson>([this](rea::stream<stgJson>* aInput){
            m_image = aInput->data().getData();
            auto img_lbls = getImageLabels(m_project_images->value(m_current_image).toObject());
            auto tsk_lbls = getLabels();
            std::vector<QString> dels;
            for (auto i : img_lbls.keys()){
                auto lbl0 = tsk_lbls.value(i).toObject();
                auto lbl1 = m_project_labels.value(i).toObject();
                auto lbl = img_lbls.value(i).toString();
                if (!lbl0.contains(lbl) || !lbl1.contains(lbl))
                    dels.push_back(i);
            }
            for (auto  i: dels)
                img_lbls.remove(i);
            aInput->out<QJsonObject>(rea::Json(m_image, "image_label", img_lbls), "updateTaskImageGUI");
        }), selectTaskImage, "updateTaskImageGUI", QJsonObject())
        ->nextB(0, "updateTaskImageGUI", QJsonObject())
        ->next(rea::local("deepsightreadCVMat", rea::Json("thread", 10)))
        ->next(rea::pipeline::add<stgCVMat>([this](rea::stream<stgCVMat>* aInput){
            auto dt = aInput->data();
            auto ch = aInput->cacheData<QHash<QString, int>>(0).value(dt);
            auto add_show = getCurrentMode()->showQSGModel(ch, dt);
            if (ch == m_show_count - 1){
                if (add_show)
                    add_show();
                rea::pipeline::run<stgJson>("deepsightreadJson", stgJson(QJsonObject(), "project/" + m_project_id + "/task/" + m_task_id + "/jobs/" + m_current_job + "/predictions/" + m_current_image + ".json"), rea::Json("tag", "updateImageResult"));
            }
            //aInput->out<QJsonObject>(cfg, "updateQSGModel_projectimage_gridder0");
        }));

    //use task image
    auto useTaskImage = rea::buffer<QJsonArray>(2);
    const QString useTaskImage_nm = "useTaskImage";
    rea::pipeline::add<bool>([](rea::stream<bool>* aInput){
        aInput->out<QJsonArray>(rea::JArray(aInput->data()));
        aInput->out<QJsonArray>(QJsonArray(), "task_image_listViewSelected");
    }, rea::Json("name", useTaskImage_nm))
        ->nextB(0, "task_image_listViewSelected", rea::Json("tag", useTaskImage_nm), useTaskImage, rea::Json("tag", useTaskImage_nm))
        ->next(useTaskImage)
        ->next(rea::pipeline::add<std::vector<QJsonArray>>([this](rea::stream<std::vector<QJsonArray>>* aInput){
            auto dt = aInput->data();
            auto lst = getImageList();
            auto imgs = getImages();
            auto add = dt[0][0].toBool();
            auto sels = dt[1];
            bool mdy = false;
            for (auto i : sels){
                auto img = lst[i.toInt()].toString();
                if (!imgs.contains(img) == add){
                    if (add)
                        imgs.insert(img, QJsonObject());
                    else
                        imgs.remove(img);
                    mdy = true;
                }
            }
            if (mdy){
                setImages(imgs);
                aInput->out<stgJson>(stgJson(*this, getTaskJsonPath()), "deepsightwriteJson");
                aInput->out<QJsonObject>(prepareImageListGUI(getImages()), "task_image_updateListView");
                aInput->out<QJsonArray>(QJsonArray(), "task_image_listViewSelected");
            }
        }))
        ->nextB(0, "deepsightwriteJson", QJsonObject())
        ->nextB(0, "task_image_updateListView", QJsonObject())
        ->next("task_image_listViewSelected", rea::Json("tag", "manual"));

    //filter images
    rea::pipeline::add<QJsonObject>([this](rea::stream<QJsonObject>* aInput){
        auto dt = aInput->data();
        QJsonArray lst;
        auto imgs = getImages();
        if (dt.value("type") == "all"){
            setImageList(lst, true);
        }else if (dt.value("type") == "used"){
            for (auto i : imgs.keys())
                lst.push_back(i);
            setImageList(lst);
        }else if (dt.value("type") == "stage"){
            auto stg = dt.value("value").toString();
            for (auto i : imgs.keys()){
                auto img = imgs.value(i).toObject();
                if (img.value("stage") == stg)
                    lst.push_back(i);
            }
            setImageList(lst);
        }else
            return;
        setFilter(dt);
        aInput->out<QJsonObject>(prepareImageListGUI(imgs), "task_image_updateListView");
        aInput->out<QJsonArray>(QJsonArray(), "task_image_listViewSelected");
        aInput->out<stgJson>(stgJson(*this, getTaskJsonPath()), "deepsightwriteJson");
    }, rea::Json("name", "filterTaskImages"))
        ->nextB(0, "task_image_updateListView", QJsonObject())
        ->nextB(0, "task_image_listViewSelected", rea::Json("tag", "manual"))
        ->nextB(0, "deepsightwriteJson", QJsonObject());

    //automatic set image stage
    if (!rea::pipeline::find("automaticSetImageStage", false))
        rea::pipeline::add<QJsonObject>([](rea::stream<QJsonObject>* aInput){
            auto dt = aInput->data();
            auto imgs = dt.value("images").toObject();
            std::cout << dt.value("train").toString().toInt() << std::endl;
            std::cout << dt.value("test").toString().toInt() << std::endl;
            std::cout << dt.value("validation").toString().toInt() << std::endl;
            for (auto i : imgs.keys()){
                auto img = imgs.value(i).toObject();
                img.insert("stage", "test");
                imgs.insert(i, img);
            }
            aInput->setData(rea::Json(dt, "images", imgs))->out();
        }, rea::Json("name", "automaticSetImageStage"));

    const QString setImageStage_nm = "setImageStage";
    auto setImageStage_tag = rea::Json("tag", setImageStage_nm);
    rea::pipeline::find("_newObject")
    ->next(rea::pipeline::add<QJsonObject>([this](rea::stream<QJsonObject>* aInput){
        auto dt = aInput->data();
        aInput->setData(rea::Json(dt, "images", getImages()))->out();
    }), setImageStage_tag)
    ->next("automaticSetImageStage")
    ->next(rea::pipeline::add<QJsonObject>([this](rea::stream<QJsonObject>* aInput){
        auto dt = aInput->data();
        auto imgs = dt.value("images").toObject();
        setImages(imgs);
        aInput->out<QJsonObject>(prepareImageListGUI(imgs), "task_image_updateListView");
        aInput->out<QJsonArray>(QJsonArray(), "task_image_listViewSelected");
        aInput->out<stgJson>(stgJson(*this, getTaskJsonPath()), "deepsightwriteJson");
    }))
    ->nextB(0, "task_image_updateListView", QJsonObject())
    ->nextB(0, "task_image_listViewSelected", rea::Json("tag", "manual"))
    ->nextB(0, "deepsightwriteJson", QJsonObject());

    //manual set image stage
    auto setImageStage = rea::buffer<QJsonArray>(2);
    rea::pipeline::add<QString>([](rea::stream<QString>* aInput){
        aInput->out<QJsonArray>(rea::JArray(aInput->data()));
        aInput->out<QJsonArray>(QJsonArray(), "task_image_listViewSelected");
    }, rea::Json("name", setImageStage_nm))
    ->nextB(0, "task_image_listViewSelected", setImageStage_tag, setImageStage, setImageStage_tag)
    ->next(setImageStage)
    ->next(rea::pipeline::add<std::vector<QJsonArray>>([this](rea::stream<std::vector<QJsonArray>>* aInput){
        auto dt = aInput->data();
        auto stg = dt[0][0].toString();
        auto sels = dt[1];
        auto lst = getImageList();
        auto imgs = getImages();
        bool mdy = false;
        for (auto i : sels){
            auto nm = lst[i.toInt()].toString();
            if (imgs.contains(nm)){
                auto img = imgs.value(nm).toObject();
                this->setImageStage(img, stg);
                imgs.insert(nm, img);
                mdy = true;
            }
        }
        if (mdy){
            setImages(imgs);
            aInput->out<QJsonObject>(prepareImageListGUI(imgs), "task_image_updateListView");
            aInput->out<QJsonArray>(QJsonArray(), "task_image_listViewSelected");
            aInput->out<stgJson>(stgJson(*this, getTaskJsonPath()), "deepsightwriteJson");
        }
    }))
    ->nextB(0, "task_image_updateListView", QJsonObject())
    ->nextB(0, "task_image_listViewSelected", rea::Json("tag", "manual"))
    ->nextB(0, "deepsightwriteJson", QJsonObject());

    //modify image
    rea::pipeline::find("QSGAttrUpdated_taskimage_gridder0")
        ->next(rea::pipeline::add<QJsonArray>([this](rea::stream<QJsonArray>* aInput){
            getCurrentMode()->tryModifyCurrentModel(aInput);
        }))
        ->nextB(0, "updateROIGUI", QJsonObject())
        ->nextB(0, "deepsightwriteJson", QJsonObject())
        ->next(rea::local("deepsightreadJson", rea::Json("thread", 10)))
        ->next(rea::pipeline::add<stgJson>([this](rea::stream<stgJson>* aInput){
            auto dt = aInput->data().getData();
            auto pth = QString(aInput->data());
            getCurrentMode()->modifyRemoteModel(aInput);
            aInput->out<stgJson>(stgJson(dt, pth), "deepsightwriteJson");
        }))
        ->next("deepsightwriteJson");

    //get task current image info
    rea::pipeline::add<QJsonObject, rea::pipePartial>([this](rea::stream<QJsonObject>* aInput){
        if (m_current_image != ""){
            auto nms = getImageName(m_project_images->value(m_current_image).toObject());
            QJsonArray imgs;
            for (auto i : nms)
                imgs.push_back("project/" + m_project_id + "/image/" + m_current_image + "/" + i.toString());
            aInput->setData(rea::Json("images", imgs, "show", m_image_show))->out();
        }
    }, rea::Json("name", "getTaskCurrentImage"));
}

void task::guiManagement(){
    //the qsgnode will be destroyed by qt after switch page
    rea::pipeline::find("title_updateStatus")
        ->next(rea::pipeline::add<QJsonArray>([this](rea::stream<QJsonArray>* aInput){
            if (aInput->data().size() == 3)
                aInput->out<QJsonArray>(QJsonArray(), "task_image_listViewSelected");
            else
                m_current_image = "";
        }))
        ->next("task_image_listViewSelected", rea::Json("tag", "manual"));

    //switch scatter mode
    rea::pipeline::add<QJsonObject>([this](rea::stream<QJsonObject>* aInput){
        auto dt = aInput->data();
        if (dt.contains("count"))
            m_show_count = dt.value("count").toInt();
        else
            m_show_count = m_show_count == m_channel_count ? 1 : m_channel_count;
        aInput->out<QJsonObject>(rea::Json("size", m_show_count), "taskimage_updateViewCount");
        m_current_image = "";
        aInput->out<QJsonArray>(QJsonArray(), "task_image_listViewSelected");
    }, rea::Json("name", "scatterTaskImageShow"))
        ->nextB(0, "taskimage_updateViewCount", QJsonObject())
        ->next("task_image_listViewSelected", rea::Json("tag", "manual"));

    //modify image show
    rea::pipeline::find("_newObject")
    ->next(rea::pipeline::add<QJsonObject>([this](rea::stream<QJsonObject>* aInput){
           if (m_task_id == "")
               return;
           auto dt = aInput->data();
           bool mdy = false;
           for (auto i : dt.keys()){
               if (dt.value(i) != m_image_show.value(i)){
                   m_image_show.insert(i, dt.value(i));
                   mdy = true;
               }
           }
           if (mdy){
               if (m_current_image != ""){
                   for (int i = 0; i < m_show_count; ++i)
                       rea::pipeline::run<QJsonObject>("updateQSGAttr_taskimage_gridder" + QString::number(i),
                                                       rea::Json("obj", "img_" + m_current_image, "key", rea::JArray("transform"), "val", m_image_show));
               }
           }
    }), rea::Json("tag", "setImageShow"));
}

void task::serverManagement(){
    //initialize socket
    rea::pipeline::find("tryLinkServer")->previous(rea::pipeline::add<stgJson>([](rea::stream<stgJson>* aInput){
        auto dt = aInput->data();
        if (dt.getData().value("test_server").toBool()){
            static normalServer server;
            static QHash<QString, int> job_states;
            aInput->out<QJsonObject>(rea::Json("ip", "127.0.0.1",
                                               "port", "8081",
                                               "id", "hello"), "tryLinkServer");

            rea::pipeline::find("receiveFromClient")  //server end
                ->nextB(0, rea::pipeline::add<clientMessage>([](rea::stream<clientMessage>* aInput){
                        auto dt = aInput->data();
                        auto ret = protocal.value(protocal_connect).toObject().value("res").toObject();
                        aInput->setData(ret)->out();
                        }), rea::Json("tag", protocal_connect), "callClient", QJsonObject())
                ->nextB(0, rea::pipeline::add<clientMessage>([](rea::stream<clientMessage>* aInput){
                            auto dt = aInput->data();
                            auto ret = protocal.value(protocal_stop_job).toObject().value("res").toObject();
                            job_states.insert(dt.value("stop_job_id").toString(), - 1);
                            aInput->setData(ret)->out();
                        }), rea::Json("tag", protocal_stop_job), "callClient", QJsonObject())
                ->nextB(0, rea::pipeline::add<clientMessage>([](rea::stream<clientMessage>* aInput){
                        auto dt = aInput->data();
                        auto ret = protocal.value(protocal_training).toObject().value("res").toObject();

                        auto id = dt.value("params").toObject().value("train_from_model").toString();
                        if (id != ""){
                            ret.insert("job_id", id);
                        }else{
                            auto tm0 = QDateTime::currentDateTime();
                            id = rea::generateUUID();
                            ret.insert("job_id", QString::number(tm0.toTime_t()) + "_" + id);
                        }

                        aInput->setData(ret)->out();
                    }), rea::Json("tag", protocal_training), "callClient", QJsonObject())
                ->nextB(0, rea::pipeline::add<clientMessage>([](rea::stream<clientMessage>* aInput){
                        auto dt = aInput->data();
                        auto ret = protocal.value(protocal_task_state).toObject().value("res").toObject();
                        auto id = dt.value("id").toString();
                        ret.insert("id", id);
                        auto job_id = id.mid(id.indexOf("_") + 1, id.length());
                        auto cnt = job_states.value(job_id) + 1;
                        job_states.insert(job_id, cnt);
                        if (!cnt)
                            ret.insert("state", "fail");
                        else if (cnt == 1000)
                            ret.insert("state", "process_finish");
                        else if (cnt < 2000)
                            ret.insert("state", "running");
                        else
                            ret.insert("state", "upload_finish");
                        auto res = clientMessage(ret);
                        res.client_socket = aInput->data().client_socket;
                        aInput->out<clientMessage>(res, "callClient");

                        if (cnt){
                            res = clientMessage(rea::Json(protocal.value(protocal_progress_push).toObject().value("res").toObject(),
                                                          "job_id", dt.value("job_id"),
                                                          "progress", cnt * 100.0 / 2000,
                                                          "progress_msg", "..."));
                            res.client_socket = aInput->data().client_socket;
                            aInput->out<clientMessage>(res, "callClient");

                            res = clientMessage(rea::Json(protocal.value(protocal_log_push).toObject().value("res").toObject(),
                                                          "job_id", dt.value("job_id"),
                                                          "log_level", cnt % 2 == 0 ? "info" : "warning",
                                                          "log_msg", "..."));
                            res.client_socket = aInput->data().client_socket;
                            aInput->out<clientMessage>(res, "callClient");
                        }

                    }), rea::Json("tag", protocal_task_state), "callClient", QJsonObject());
        }else
            aInput->out<QJsonObject>(QJsonObject(), "tryLinkServer");
    }))->previous(rea::local("readJson"))->execute(std::make_shared<rea::stream<stgJson>>((stgJson(QJsonObject(), "config_.json"))));

    //manually link server
    rea::pipeline::find("_newObject")
        ->next(rea::pipeline::add<QJsonObject>([](rea::stream<QJsonObject>* aInput){
                   auto dt = aInput->data();
                   if (dt.value("auto").toBool())
                       aInput->out<QJsonObject>(QJsonObject(), "tryLinkServer");
                   else
                       aInput->out<QJsonObject>(rea::Json("ip", dt.value("ip"),
                                                          "port", dt.value("port"),
                                                          "id", "hello"));
        }), rea::Json("tag", "setServer"))
        ->next("tryLinkServer");
}

void task::insertJob(const QString& aID){
    auto tm0 = QDateTime::currentDateTime();
    auto tm = tm0.toString(Qt::DateFormat::ISODate);
    auto tms = tm.split("T");
    m_jobs.insert(aID, rea::Json("time", tms[0] + " " + tms[1], "state", "running"));
}

QJsonObject task::prepareTrainingData(){
    QJsonArray uuid_list, image_label_list, image_dataset_list;
    auto imgs = getImages();
    auto tsk_lbls = getLabels();
    tsk_lbls.remove("shape");
    for (auto i : imgs.keys()){
        auto img = imgs.value(i).toObject();
        uuid_list.push_back(i);
        image_dataset_list.push_back(getImageStage(img));

        auto img_lbls = getImageLabels(m_project_images->value(i).toObject());
        QJsonArray lbls;
        for (auto j : tsk_lbls.keys())
            lbls.push_back(img_lbls.value(j).toString());
        image_label_list.push_back(lbls);
    }
    return rea::Json("s3_bucket_name", s3_bucket_name,
                     "image_root", "project/" + m_project_id + "/image",
                     "json_root", "project/" + m_project_id + "/image",
                     "uuid_list", uuid_list,
                     "image_label_list", image_label_list,
                     "image_dataset_list", image_dataset_list);
}

QString task::getJobState(const QJsonObject& aJob){
    return aJob.value("state").toString();
}

void task::setJobState(QJsonObject& aJob, const QString& aState){
    aJob.insert("state", aState);
}

QJsonArray task::getLossList(const QJsonObject& aStatistics){
    return aStatistics.value("loss_list").toArray();
}

QJsonObject task::getHistogramData(const QJsonObject& aStatistics){
    return aStatistics.value("score_hist").toObject();
}

QJsonObject task::prepareTrainParam(){
    return m_current_param;
}

QJsonArray task::getPredictShapes(const QJsonObject& aImageResult){
    return aImageResult.value("predict_shapes").toArray();
}

QJsonArray task::updateResultObjects(const QJsonObject& aImageResult, int aIndex){
    QJsonArray ret;
    auto shps = getPredictShapes(m_image_result);
    for (int i = 0; i < shps.size(); ++i)
        ret.push_back(rea::Json("key", rea::JArray("objects"),
                                "type", "del",
                                "tar", "result_" + QString::number(i) + "_" + QString::number(aIndex)));

    if (getCurrentMode()->showResult()){
        shps = getPredictShapes(aImageResult);
        auto basis = aImageResult.value("predict_polygon_objs_basis").toArray();
        double rx = 1, ry = 1;
        if (basis.size() == 2){
            rx = aImageResult.value("width").toInt() * 1.0 / basis[1].toInt();
            ry = aImageResult.value("height").toInt() * 1.0 / basis[0].toInt();
        }else
            basis = rea::JArray(aImageResult.value("width"), aImageResult.value("height"));

        for (int i = 0; i < shps.size(); ++i){
            auto shp = shps[i].toObject();
            auto nm = "result_" + QString::number(i) + "_" + QString::number(aIndex);
            if (shp.value("type") == "rectangle"){
                if (shp.value("score").toDouble() > m_min_threshold){
                    auto arr = shp.value("points").toArray();
                    std::vector<double> pts{arr[0].toDouble() * rx, arr[1].toDouble() * ry, arr[2].toDouble() * rx, arr[3].toDouble() * ry};
                    ret.push_back(rea::Json("key", rea::JArray("objects"),
                                            "type", "add",
                                            "tar", nm,
                                            "val", rea::Json("type", "poly",
                                                             "caption", shp.value("label"),
                                                             "color", "green",
                                                             "text", rea::Json("visible", true,
                                                                               "size", rea::JArray(80, 30),
                                                                               "location", "bottom"),
                                                             "tag", "result",
                                                             "points", rea::JArray(QJsonArray(), rea::JArray(pts[0], pts[1], pts[2], pts[1], pts[2], pts[3], pts[0], pts[3], pts[0], pts[1])))));
                }
            }else if (shp.value("type") == "score_map"){
                QImage img(basis[0].toInt(), basis[1].toInt(), QImage::Format_ARGB32);
                img.fill(QColor("transparent"));
                auto arr = shp.value("points").toArray(), scs = shp.value("scores").toArray();
                int bnd[4];
                auto clr = QColor("green");
                for (int i = 0; i < scs.size(); ++i){
                    auto x = arr[i * 2].toInt(), y = arr[i * 2 + 1].toInt();
                    if (scs[i].toDouble() > m_min_threshold)
                        img.setPixelColor(x, y, QColor(clr.red(), clr.green(), clr.blue(), 125));
                    if (i == 0){
                        bnd[0] = x; bnd[1] = y; bnd[2] = x; bnd[3] = y;
                    }else{
                        bnd[0] = std::min(x, bnd[0]); bnd[1] = std::min(y, bnd[1]); bnd[2] = std::max(x, bnd[2]); bnd[3] = std::max(y, bnd[3]);
                    }
                }
                img = img.copy(bnd[0], bnd[1], bnd[2] - bnd[0], bnd[3] - bnd[1]).scaled((bnd[2] - bnd[0]) * rx, (bnd[3] - bnd[1]) * ry);
                rea::imagePool::cacheImage(nm, img);
                ret.push_back(rea::Json("key", rea::JArray("objects"),
                                        "type", "add",
                                        "tar", nm,
                                        "val", rea::Json("type", "image",
                                                         "caption", shp.value("label"),
                                                         "color", "green",
                                                         "range", rea::JArray(bnd[0] * rx, bnd[1] * ry, bnd[2] * rx, bnd[3] * ry),
                                                         "text", rea::Json("visible", true,
                                                                           "size", rea::JArray(80, 30),
                                                                           "location", "bottom"),
                                                         "tag", "result",
                                                         "path", nm)));
            }
        }
    }
    m_image_result = aImageResult;
    return ret;
}

void task::jobManagement(){
    //start Job
    rea::pipeline::add<QJsonObject>([this](rea::stream<QJsonObject>* aInput){
        auto dt = aInput->data();
        auto train_prm = prepareTrainParam();
        auto prm = rea::Json(protocal.value(protocal_training).toObject().value("req").toObject(),
                             "project_id", m_project_id,
                             "task_id", m_task_id,
                             "data", prepareTrainingData(),
                             "task_type", value("type"),
                             "params", train_prm);
        auto mdls = dt.value("train_from_model").toArray();
        if (mdls.size() == 0)
            aInput->out<QJsonObject>(prm, "callServer");
        else
            for (auto i : mdls)
                aInput->out<QJsonObject>(rea::Json(prm,
                                                   "params", rea::Json(train_prm,
                                                                       "train_from_model", (m_jobs.begin() + i.toInt()).key())),
                                         "callServer");
    }, rea::Json("name", "startJob"))
        ->next("callServer")
        ->next(rea::pipeline::add<QJsonObject>([this](rea::stream<QJsonObject>* aInput){
            auto res = aInput->data();
            if (res.value("err_code").toInt()){
                aInput->out<QJsonObject>(rea::Json("title", "error", "text", res.value("msg")), "popMessage");
            }else{
                insertJob(res.value("job_id").toString());
                m_current_job = "";
                aInput->out<QJsonObject>(prepareJobListGUI(), "task_job_updateListView");
                aInput->out<stgJson>(stgJson(m_jobs, getJobsJsonPath()), "deepsightwriteJson");
                aInput->out<QJsonArray>(QJsonArray(), "task_job_listViewSelected");
            }
        }), rea::Json("tag", protocal_training))
        ->nextB(0, "task_job_updateListView", QJsonObject())
        ->nextB(0, "deepsightwriteJson", QJsonObject())
        ->nextB(0, "task_job_listViewSelected", rea::Json("tag", "manual"))
        ->next("popMessage");

    //update image result
    rea::pipeline::find("deepsightreadJson")
        ->next(rea::pipeline::add<stgJson>([this](rea::stream<stgJson>* aInput){
           for (int i = 0; i < m_show_count; ++i)
               rea::pipeline::run<QJsonArray>("updateQSGAttrs_taskimage_gridder" + QString::number(i), updateResultObjects(aInput->data().getData(), i));
               }), rea::Json("tag", "updateImageResult"))
        ->previous(rea::pipeline::add<QJsonArray>([this](rea::stream<QJsonArray>* aInput){
               auto dt = aInput->data();
               m_min_threshold = dt[0].toDouble();
               m_max_threshold = dt[1].toDouble();
               aInput->out<stgJson>(stgJson(m_image_result, ""));
        }, rea::Json("name", "thresholdChanged")));

    //select Job
    rea::pipeline::find("task_job_listViewSelected")
        ->next(rea::pipeline::add<QJsonArray>([this](rea::stream<QJsonArray>* aInput){
               auto dt = aInput->data();
               do{
                   if (dt.size() == 0){
                       aInput->out<QJsonObject>(QJsonObject(), "updateTaskJobGUI");
                       aInput->out<QJsonObject>(QJsonObject(), "updateTaskJobProgress");
                       aInput->out<QJsonObject>(rea::Json("log", QJsonArray()), "updateTaskJobLog");
                       break;
                   }
                   auto cur = (m_jobs.begin() + dt[0].toInt()).key();
                   if (m_current_job != cur){
                       m_current_job = cur;
                       m_current_request = QString::number(QDateTime::currentDateTime().toTime_t()) + "_" + m_current_job;
                       aInput->out<QJsonObject>(rea::Json("log", *rea::tryFind(&m_log_cache, m_current_job)), "updateTaskJobLog");
                       aInput->out<QString>(m_current_request, "requestJobState");
                       break;
                   }else{
                       return;
                   }
               }while(0);
               aInput->out<QJsonObject>(rea::Json("content", rea::JArray(QJsonArray(), QJsonArray(), QJsonArray())), "result_abstract_updateMatrix");
               aInput->out<QJsonArray>(QJsonArray(), "_updateLineChart");
               aInput->out<QJsonObject>(QJsonObject(), "_updateTHistogramGUI");
           }), rea::Json("tag", "manual"))
        ->nextB(0, "result_abstract_updateMatrix", QJsonObject())
        ->nextB(0, "_updateLineChart", QJsonObject())
        ->nextB(0, "_updateTHistogramGUI", QJsonObject())
        ->nextB(0, "updateTaskJobProgress", QJsonObject())
        ->nextB(0, "updateTaskJobGUI", QJsonObject())
        ->nextB(0, "updateTaskJobLog", QJsonObject())
        ->next(rea::pipeline::add<QString>([this](rea::stream<QString>* aInput){
            if (aInput->data() != m_current_request)
                return;
            auto job = m_jobs.value(m_current_job).toObject();
            aInput->out<QJsonObject>(job, "updateTaskJobGUI");
            if (getJobState(job) == "running"){
                aInput->out<QJsonObject>(rea::Json(protocal.value(protocal_task_state).toObject().value("req").toObject(),
                                                   "project_id", m_project_id,
                                                   "task_id", m_task_id,
                                                   "job_id", m_current_job,
                                                   "id", aInput->data()),
                                         "callServer");
            }else if (getJobState(job) == "process_finish"){
                aInput->out<QJsonObject>(rea::Json(protocal.value(protocal_upload).toObject().value("req").toObject(),
                                                   "job_id", m_current_job,
                                                   "s3_bucket_name", s3_bucket_name,
                                                   "data_root", "project/" + m_project_id + "/task/" + m_task_id + "/jobs/" + m_current_job),
                                         "callServer");
            }else //if (getJobState(job) == "upload_finish")
            {
                aInput->out<QJsonObject>(QJsonObject(), "updateTaskJobProgress");
                aInput->out<stgJson>(stgJson(QJsonObject(), "project/" + m_project_id + "/task/" + m_task_id + "/jobs/" + m_current_job + "/result_summary.json"));
                aInput->out<stgJson>(stgJson(QJsonObject(), "project/" + m_project_id + "/task/" + m_task_id + "/jobs/" + m_current_job + "/result_statistics.json"));
                aInput->out<stgJson>(stgJson(QJsonObject(), "project/" + m_project_id + "/task/" + m_task_id + "/jobs/" + m_current_job + "/predictions/" + m_current_image + ".json"), "deepsightreadJson");
            }
        }, rea::Json("name", "requestJobState", "thread", 5)))
        ->nextB(0, "deepsightreadJson", rea::Json("tag", "updateImageResult"))
        ->nextB(0, rea::local("deepsightreadJson", rea::Json("thread", 10)), QJsonObject(), rea::buffer<stgJson>(2), QJsonObject(),
                rea::pipeline::add<std::vector<stgJson>>([this](rea::stream<std::vector<stgJson>>* aInput){
                    auto summary = aInput->data()[0].getData();
                    QJsonArray title, content;
                    for (auto i : summary.keys()){
                        title.push_back(i);
                        content.push_back(summary.value(i));
                    }
                    rea::pipeline::run<QJsonObject>("result_abstract_updateMatrix", rea::Json("content", rea::JArray(QJsonArray(), title, content)));

                    auto statistics = aInput->data()[1].getData();
                    rea::pipeline::run<QJsonArray>("_updateLineChart", getLossList(statistics));
                    rea::pipeline::run<QJsonObject>("_updateTHistogramGUI", rea::Json("histogram", getHistogramData(statistics),
                                                                                      "threshold", rea::JArray(m_min_threshold, m_max_threshold)));
                }), QJsonObject())
        ->nextB(0, "updateTaskJobProgress", QJsonObject())
        ->nextB(0, "updateTaskJobGUI", QJsonObject())
        ->next("callServer")
        ->next(rea::pipeline::add<QJsonObject>([this](rea::stream<QJsonObject>* aInput){
               auto res = aInput->data();
               auto id = res.value("id").toString();
               if (m_current_job == "" || id.indexOf(m_current_job) < 0)
                   return;
               auto job = m_jobs.value(m_current_job).toObject();
               auto st = getJobState(res);
               if (st != getJobState(job)){
                   setJobState(job, st);
                   if (st == "process_finish"){
                       setJobState(job, "running");
                       m_jobs.insert(m_current_job, job);
                   }
                   else
                       m_jobs.insert(m_current_job, job);
                   aInput->out<stgJson>(stgJson(m_jobs, getJobsJsonPath()), "deepsightwriteJson");
                   if (st == "upload_finish"){
                       m_current_image = "";
                       aInput->out<QJsonArray>(QJsonArray(), "task_image_listViewSelected");
                   }
               }
               std::this_thread::sleep_for(std::chrono::microseconds(500));
               aInput->out<QString>(id, "requestJobState");
        }, rea::Json("thread", 5)), rea::Json("tag", protocal_task_state))
        ->nextB(0, "deepsightwriteJson", QJsonObject())
        ->nextB(0, "task_image_listViewSelected", rea::Json("tag", "manual"))
        ->next("requestJobState");

    //update job progress
    rea::pipeline::find("receiveFromServer")
        ->next(rea::pipeline::add<QJsonObject>([this](rea::stream<QJsonObject>* aInput){
            auto dt = aInput->data();
            if (dt.value("job_id") != m_current_job)
                return;
            aInput->out();
        }), rea::Json("tag", protocal_progress_push))
        ->next("updateTaskJobProgress");

    //update job log
    rea::pipeline::find("receiveFromServer")
        ->next(rea::pipeline::add<QJsonObject>([this](rea::stream<QJsonObject>* aInput){
            auto dt = aInput->data();
            auto job = dt.value("job_id").toString();
            auto entry = dt.value("log_level").toString() + ": " + dt.value("log_msg").toString();
            rea::tryFind(&m_log_cache, job)->push_back(entry);
            if (job == m_current_job)
                aInput->out<QJsonObject>(rea::Json("log_new", entry), "updateTaskJobLog");
        }), rea::Json("tag", protocal_log_push))
        ->next("updateTaskJobLog");

    //stop job
    rea::pipeline::find("task_job_listViewSelected")
        ->next(rea::pipeline::add<QJsonArray>([this](rea::stream<QJsonArray>* aInput){
                   auto dt = aInput->data();
                   for (auto i : dt){
                       aInput->out<QJsonObject>(rea::Json(protocal.value(protocal_stop_job).toObject().value("req").toObject(),
                                                          "stop_job_id", (m_jobs.begin() + i.toInt()).key()), "callServer");
                   }
               }), rea::Json("tag", "stopJob"))
        ->next("callServer");

    //continue job
    rea::pipeline::find("task_job_listViewSelected")
        ->next(rea::pipeline::add<QJsonArray>([](rea::stream<QJsonArray>* aInput){
            aInput->out<QJsonObject>(rea::Json("train_from_model", aInput->data()), "startJob");
        }), rea::Json("tag", "continueJob"))
        ->next("startJob");

    //delete Job
    rea::pipeline::find("task_job_listViewSelected")
        ->next(rea::pipeline::add<QJsonArray>([this](rea::stream<QJsonArray>* aInput){
            auto dt = aInput->data();
            if (dt.size() > 0){
                std::vector<int> idxes;
                for (auto i : dt)
                    idxes.push_back(i.toInt());
                std::sort(idxes.begin(), idxes.end(), std::greater<int>());
                for (auto i : idxes){
                    auto job = (m_jobs.begin() + i).key();
                    aInput->out<QJsonObject>(rea::Json(protocal.value(protocal_stop_job).toObject().value("req").toObject(),
                                                       "stop_job_id", job), "callServer");
                    aInput->out<QString>("project/" + m_project_id + "/task/" + m_task_id + "/jobs/" + job, "deepsightdeletePath");
                    m_jobs.erase(m_jobs.begin() + i);
                }
                aInput->out<QJsonObject>(prepareJobListGUI(), "task_job_updateListView");
                aInput->out<stgJson>(stgJson(m_jobs, getJobsJsonPath()), "deepsightwriteJson");
                aInput->out<QJsonArray>(QJsonArray(), "task_job_listViewSelected");
            }
        }), rea::Json("tag", "deleteJob"))
        ->nextB(0, "deepsightdeletePath", QJsonObject())
        ->nextB(0, "callServer", QJsonObject())
        ->nextB(0, "task_job_updateListView", QJsonObject())
        ->nextB(0, "deepsightwriteJson", QJsonObject())
        ->nextB(0, "task_job_listViewSelected", rea::Json("tag", "manual"));

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

task::task(){
    taskManagement();
    labelManagement();
    imageManagement();
    guiManagement();
    serverManagement();
    jobManagement();
}

static rea::regPip<QQmlApplicationEngine*> init_task([](rea::stream<QQmlApplicationEngine*>* aInput){
    static task cur_task;
    aInput->out();
}, rea::Json("name", "install2_task"), "regQML");
