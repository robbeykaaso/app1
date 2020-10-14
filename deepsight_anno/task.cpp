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

QString ITaskFriend::getImageResultJsonPath(){
    return m_task->getImageResultJsonPath();
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

QString ITaskFriend::getProjectID(){
    return m_task->m_project_id;
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

bool ITaskFriend::isCurrentMode(const QString& aMode){
    return m_task->isCurrentMode(aMode);
}

bool ITaskFriend::belongThisMode(const QString& aMode, const QString& aPath){
    if (!m_paths.contains(aPath))
        m_paths.insert(aPath, isCurrentMode(aMode));
        return m_paths.value(aPath);
}

int ITaskFriend::getShowCount(){
    return m_task->m_show_count;
}

void ITaskFriend::updateCurrentImage(){
    m_task->m_current_image = "";
}

void taskMode::initialize(){
    rea::pipeline::add<QJsonArray>([this](rea::stream<QJsonArray>* aInput){
        //if (getImageID() == "")
        //    return;
        QString cur = getImageJsonPath();
        QString pth = cur;
        if (modifyImage(aInput->data(), getImageData(), pth)){
            belongThisMode("task", pth);
            if (getImageID() != ""){
                aInput->out<stgJson>(stgJson(getImageData(), pth), "deepsightwriteJson");
            }
        }else if (pth != cur && belongThisMode("task", pth)){
            aInput->cache<QJsonArray>(aInput->data())->out<stgJson>(stgJson(QJsonObject(), pth));
        }
    }, rea::Json("name", "task_tryModifyCurrentModel"));

    rea::pipeline::add<stgJson>([this](rea::stream<stgJson>* aInput){
        auto dt = aInput->data().getData();
        auto pth = QString(aInput->data());
        modifyImage(aInput->cacheData<QJsonArray>(0), dt, pth);
        aInput->out<stgJson>(stgJson(dt, pth), "deepsightwriteJson");
    }, rea::Json("name", "task_modifyRemoteModel"));

    rea::pipeline::add<stgCVMat>([this](rea::stream<stgCVMat>* aInput){
            auto dt = aInput->data();
            auto ch = aInput->cacheData<QHash<QString, int>>(0).value(dt);
            auto add_show = showQSGModel(ch, dt);
            if (ch == getShowCount() - 1){
                if (add_show)
                    add_show();
                rea::pipeline::run<stgJson>("deepsightreadJson", stgJson(QJsonObject(), getImageResultJsonPath()), rea::Json("tag", "updateImageResult"));
            }
    }, rea::Json("name", "task_showQSGModel"));
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
                                                        "color", "green",
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
                     "selects", m_project_labels.size() > 0 ? rea::JArray(0) : QJsonArray(),
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
                     "selects", lst.size() > 0 ? rea::JArray(0) : QJsonArray(),
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
                     "selects", m_jobs.size() > 0 ? rea::JArray(0) : QJsonArray(),
                     "data", data);
}

QString task::getJobsJsonPath(){
    return "project/" + m_project_id + "/task/" + m_task_id + "/jobs.json";
}

QString task::getTaskJsonPath(){
    return "project/" + m_project_id + "/task/" + m_task_id + ".json";
}

QString task::getImageResultJsonPath(){
    return "project/" + m_project_id + "/task/" + m_task_id + "/jobs/" + m_current_job + "/predictions/" + m_current_image + ".json";
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
            m_task_type = dt.value("type").toString();
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
            aInput->out<QJsonObject>(prepareJobListGUI(), "task_job_updateListView");
            aInput->out<QJsonArray>(QJsonArray(), "task_job_listViewSelected");
            aInput->out<QJsonObject>(prepareImageListGUI(getImages()), "task_image_updateListView");
            aInput->out<QJsonObject>(rea::Json("count", 1), "scatterTaskImageShow");
            aInput->out<QJsonObject>(getFilter(), "updateTaskImageFilterGUI");
        }))
        ->nextB("task_label_updateListView")
        ->nextB("task_label_listViewSelected", rea::Json("tag", "task_manual"))
        ->nextB("task_image_updateListView")
        ->nextB("scatterTaskImageShow")
        ->nextB("task_job_updateListView")
        ->nextB("task_job_listViewSelected", rea::Json("tag", "manual"))
        ->nextB("updateTaskImageFilterGUI");
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
        ->nextB("task_label_updateListView")
        ->nextB("task_label_listViewSelected", rea::Json("tag", "task_manual"));

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
        ->nextB("updateCandidateLabelGUI")
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
        ->nextB(addLabel)
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
        ->nextB("task_label_updateListView")
        ->nextB("deepsightwriteJson")
        ->nextB("task_label_listViewSelected", rea::Json("tag", "task_manual"))
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

bool task::isCurrentMode(const QString& aMode){
    return (m_current_mode == aMode || (!m_custom_modes.contains(m_current_mode) && aMode == "task"));
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
    ->nextB("task_image_listViewSelected", rea::Json("tag", "manual"));

    //update project images
    rea::pipeline::find("deepsightwriteJson")
    ->next(rea::pipeline::add<stgJson>([this](rea::stream<stgJson>* aInput){
        if (m_task_id == "")
            return;
        //m_project_images = aInput->data().getData();
        aInput->out<QJsonObject>(prepareImageListGUI(getImages()), "task_image_updateListView");
    }), rea::Json("tag", "updateProjectImages"))
    ->nextB("task_image_updateListView");

    //select image
    auto select_image =
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
        ->nextB("updateQSGCtrl_taskimage_gridder0")
        ->nextB(rea::pipeline::find("deepsightreadJson")
                       ->nextB(rea::pipeline::add<stgJson>([this](rea::stream<stgJson>* aInput){
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
                                      })->nextB("updateTaskImageGUI"),
                                   selectTaskImage),
                selectTaskImage)
        ->nextB("updateTaskImageGUI")
        ->next(rea::local("deepsightreadCVMat", rea::Json("thread", 10)));
    auto show_qsg_model = [select_image, this](QString aMode){
        select_image->next(rea::pipeline::add<stgCVMat>([this, aMode](rea::stream<stgCVMat>* aInput){
            if (isCurrentMode(aMode))
                aInput->out();
        }))
        ->next(aMode + "_showQSGModel");
    };
    show_qsg_model("task");
    for (auto i : m_custom_modes)
        show_qsg_model(i);

    //use task image
    auto useTaskImage = rea::buffer<QJsonArray>(2);
    const QString useTaskImage_nm = "useTaskImage";
    rea::pipeline::add<bool>([](rea::stream<bool>* aInput){
        aInput->out<QJsonArray>(rea::JArray(aInput->data()));
        aInput->out<QJsonArray>(QJsonArray(), "task_image_listViewSelected");
    }, rea::Json("name", useTaskImage_nm))
        ->nextB(rea::pipeline::find("task_image_listViewSelected")
                       ->nextB(useTaskImage, rea::Json("tag", useTaskImage_nm)),
                rea::Json("tag", useTaskImage_nm))
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
        ->nextB("deepsightwriteJson")
        ->nextB("task_image_updateListView")
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
                if (img.value("stage") == stg || (!img.contains("stage") && stg == "none"))
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
        ->nextB("task_image_updateListView")
        ->nextB("task_image_listViewSelected", rea::Json("tag", "manual"))
        ->nextB("deepsightwriteJson");

    //automatic set image stage
    if (!rea::pipeline::find("automaticSetImageStage", false))
        rea::pipeline::add<QJsonObject>([](rea::stream<QJsonObject>* aInput){
            auto dt = aInput->data();
            auto imgs = dt.value("images").toObject();
            std::cout << dt.value("type").toString().toStdString() << std::endl;
            auto used_labels = dt.value("used_labels").toObject();
            std::cout << dt.value("train").toInt() << std::endl;
            std::cout << dt.value("test").toInt() << std::endl;
            std::cout << dt.value("validation").toInt() << std::endl;
            for (auto i : imgs.keys()){
                auto img = imgs.value(i).toObject();
                auto img_lbls = img.value("image_labels").toObject();
                auto shp_lbls = img.value("shape_labels").toArray();
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
        auto imgs = getImages();
        aInput->cache<QJsonObject>(QJsonObject());
        aInput->cache<QJsonObject>(dt);
        for (auto i : imgs.keys())
            aInput->out<stgJson>(stgJson(QJsonObject(), "project/" + m_project_id + "/image/" + i + ".json"));
    }), setImageStage_tag)
    ->next(rea::local("deepsightreadJson", rea::Json("thread", 10)))
    ->next(rea::pipeline::add<stgJson>([this](rea::stream<stgJson>* aInput){
        auto dt = aInput->data();
        auto pth = dt.mid(dt.lastIndexOf("/") + 1, dt.length());
        pth = pth.mid(0, pth.indexOf("."));
        auto img = aInput->data().getData();
        auto shps = img.value("shapes").toObject();
        QJsonArray lbls;
        for (auto i : shps){
            auto shp = i.toObject();
            if (shp.contains("label"))
                lbls.push_back(shp.value("label"));
        }
        auto img_cfg = m_project_images->value(pth).toObject();

        auto imgs = getImages();
        auto ret = aInput->cacheData<QJsonObject>(0);
        ret.insert(pth, rea::Json(imgs.value(pth).toObject(), "image_labels", getImageLabels(img_cfg), "shape_labels", lbls));
        if (ret.size() == imgs.size()){
            aInput->out<QJsonObject>(rea::Json(aInput->cacheData<QJsonObject>(1), "images", ret, "type", m_task_type, "used_labels", getLabels()), "automaticSetImageStage");
        }else
            aInput->cache<QJsonObject>(ret, 0);
    }))
    ->next("automaticSetImageStage")
    ->next(rea::pipeline::add<QJsonObject>([this](rea::stream<QJsonObject>* aInput){
        auto dt = aInput->data();
        auto ret = dt.value("images").toObject();
        QJsonObject imgs;
        for (auto i : ret.keys()){
            imgs.insert(i, rea::Json("stage", ret.value(i).toObject().value("stage")));
        }
        setImages(imgs);
        aInput->out<QJsonObject>(prepareImageListGUI(imgs), "task_image_updateListView");
        aInput->out<QJsonArray>(QJsonArray(), "task_image_listViewSelected");
        aInput->out<stgJson>(stgJson(*this, getTaskJsonPath()), "deepsightwriteJson");
    }))
    ->nextB("task_image_updateListView")
    ->nextB("task_image_listViewSelected", rea::Json("tag", "manual"))
    ->nextB("deepsightwriteJson");

    //manual set image stage
    auto setImageStage = rea::buffer<QJsonArray>(2);
    rea::pipeline::add<QString>([](rea::stream<QString>* aInput){
        aInput->out<QJsonArray>(rea::JArray(aInput->data()));
        aInput->out<QJsonArray>(QJsonArray(), "task_image_listViewSelected");
    }, rea::Json("name", setImageStage_nm))
    ->nextB(rea::pipeline::find("task_image_listViewSelected")
                       ->nextB(setImageStage, setImageStage_tag),
                setImageStage_tag)
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
    ->nextB("task_image_updateListView")
    ->nextB("task_image_listViewSelected", rea::Json("tag", "manual"))
    ->nextB("deepsightwriteJson");

    //modify project image//try update show
    rea::pipeline::find("QSGAttrUpdated_projectimage_gridder0")
        ->next(rea::pipeline::add<QJsonArray>([this](rea::stream<QJsonArray>* aInput){
            auto dt = aInput->data();
            QJsonArray mdys;
            for (int i = 0; i < dt.size(); ++i){
                auto mdy = dt[i].toObject();
                if (mdy.contains("cmd")){
                    mdy.remove("cmd");
                    mdys.push_back(mdy);
                }
            }
            if (mdys.size() > 0)
                for (int i = 0; i < m_show_count; ++i)
                    rea::pipeline::run<QJsonArray>("updateQSGAttrs_taskimage_gridder" + QString::number(i), mdys);
        }));

    //modify image
    auto modify_image = [this](QString aMode){
        rea::pipeline::find("QSGAttrUpdated_taskimage_gridder0")
            //->next(rea::pipeline::add<QJsonArray>([this, aMode](rea::stream<QJsonArray>* aInput){
            //    if (isCurrentMode(aMode))
            //        aInput->out();
            //}))
            ->next(aMode + "_tryModifyCurrentModel")
            ->nextB("deepsightwriteJson")
            ->next(rea::local("deepsightreadJson", rea::Json("thread", 10)))
            ->next(aMode + "_modifyRemoteModel")
            ->next("deepsightwriteJson");
    };
    modify_image("task");
    for (auto i : m_custom_modes)
        modify_image(i);

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
    rea::pipeline::find("title_updateNavigation")
        ->next(rea::pipeline::add<QJsonArray>([this](rea::stream<QJsonArray>* aInput){
            if (aInput->data().size() == 3)
                aInput->out<QJsonArray>(QJsonArray(), "task_image_listViewSelected");
            else
                m_current_image = "";
        }), rea::Json("tag", "manual"))
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
        ->nextB("taskimage_updateViewCount")
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
                ->nextB(rea::pipeline::add<clientMessage>([](rea::stream<clientMessage>* aInput){
                               auto dt = aInput->data();
                               auto ret = protocal.value(protocal_connect).toObject().value("res").toObject();
                               aInput->setData(ret)->out();
                           })->nextB("callClient"),
                        rea::Json("tag", protocal_connect))
                ->nextB(rea::pipeline::add<clientMessage>([](rea::stream<clientMessage>* aInput){
                               auto dt = aInput->data();
                               auto ret = protocal.value(protocal_stop_job).toObject().value("res").toObject();
                               job_states.insert(dt.value("stop_job_id").toString(), - 1);
                               aInput->setData(ret)->out();
                           })->nextB("callClient"),
                        rea::Json("tag", protocal_stop_job))
                ->nextB(rea::pipeline::add<clientMessage>([](rea::stream<clientMessage>* aInput){
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
                        }, rea::Json("name", "startServerTraining"))->nextB("callClient"),
                        rea::Json("tag", protocal_training))
                ->nextB(rea::pipeline::add<clientMessage>([](rea::stream<clientMessage>* aInput){
                            auto dt = aInput->data();
                            auto ret = protocal.value(protocal_inference).toObject().value("res").toObject();
                            auto tm0 = QDateTime::currentDateTime();
                            auto id = rea::generateUUID();
                            ret.insert("type", "inference");
                            ret.insert("job_id", QString::number(tm0.toTime_t()) + "_" + id);
                            aInput->setData(ret)->out();
                        })->nextB("callClient"),
                        rea::Json("tag", "inference"))
                ->nextB(rea::pipeline::add<clientMessage>([](rea::stream<clientMessage>* aInput){
                        auto dt = aInput->data();
                        auto ret = protocal.value(protocal_task_state).toObject().value("res").toObject();
                        auto id = dt.value("id").toString();
                        ret.insert("id", id);
                        auto job_id = dt.value("job_id").toString();
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

                        }, rea::Json("name", "pollingTrainingState"))->nextB("callClient"),
                        rea::Json("tag", protocal_task_state));
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

QJsonObject task::prepareTrainingData(const QJsonArray& aImages){
    QJsonArray uuid_list, image_label_list, image_dataset_list;
    auto imgs = getImages();
    if (aImages.size() > 0){
        auto lst = getImageList();
        QJsonObject sel_imgs;
        for (auto i : aImages){
            auto img = (lst.begin() + i.toInt())->toString();
            sel_imgs.insert(img, imgs.value(img));
        }
        imgs = sel_imgs;
    }
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

    if (!m_custom_modes.contains(m_current_mode)){
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

int task::calcThresholdIndex(){
    int idx1 = getThresholdIndex(m_min_threshold), idx2 = getThresholdIndex(m_max_threshold);
    return idx2 * m_threshold_list.size() + idx1;
}

int task::getThresholdIndex(double aThreshold){
    int ret = 0;
    for (size_t i = 0; i < m_threshold_list.size(); ++i)
        if (m_threshold_list.at(i) >= aThreshold){
            ret = int(i);
            break;
        }
    return ret;
}

int task::getIOUIndex(double aIOU){
    int ret = 0;
    for (size_t i = 0; i < m_iou_list.size(); ++i)
        if (m_iou_list.at(i) >= aIOU){
            ret = int(i);
            break;
        }
    return ret;
}

void task::updateStatisticsModel(const QJsonObject& aStatistics){
    auto double_metric_data = aStatistics.value("double_metric_data").toObject();
    auto image_level_statistics = double_metric_data.value("image_level_statistics").toObject();
    auto instance_level_statistics = double_metric_data.value("instance_level_statistics").toObject();

    m_image_level_statistics.clear();
    for (auto i : image_level_statistics.keys())
        if (i.contains("#")){
            auto lbls = i.split("#");
            rea::tryFind(&m_image_level_statistics, lbls[1].split(":")[1])->insert(lbls[0].split(":")[1], image_level_statistics.value(i).toArray());
        }

    m_instance_level_statistics.clear();
    int idx1 = 1, idx2 = 0;
    if (aStatistics.value("object_table_type") == "statistics"){
        idx1 = 0;
        idx2 = 1;
    }
    for (auto i : instance_level_statistics.keys())
        if (i.contains("#")){
            auto lbls = i.split("#");
            auto ret = instance_level_statistics.value(i);
            if (ret.isArray())
                rea::tryFind(&m_instance_level_statistics, lbls[idx1].split(":")[1])->insert(lbls[idx2].split(":")[1], ret.toArray());
            else if (ret.isDouble())
                rea::tryFind(&m_instance_level_statistics, lbls[idx1].split(":")[1])->insert(lbls[idx2].split(":")[1], rea::JArray(ret));
            else
                throw "dsResult deserialize error!";
        }

    m_threshold_list.clear();
    auto thresholds = image_level_statistics.value("score_threshold_list").toArray();
    for (auto i : thresholds)
        m_threshold_list.push_back(i.toDouble());

    m_iou_list.clear();
    thresholds = instance_level_statistics.value("iou_threshold_list").toArray();
    for (auto i : thresholds)
        m_iou_list.push_back(i.toDouble());

    m_images_statistics.clear();
    m_instances_statistics.clear();
    auto every_image_statistics = double_metric_data.value("every_image_statistics").toObject();
    for (auto i : every_image_statistics.keys()){
        auto statistics = every_image_statistics.value(i).toObject();
        m_images_statistics.insert(i, statistics.value("image_level_statistics").toObject());
        m_instances_statistics.insert(i, statistics.value("instance_level_statistics").toObject());
    }
}

QString task::getImagePredict(const QString& aImageID, const QJsonObject& aImageResult){
    QString ret = "";
    if (aImageResult.value("stage") != "test"){
        if (m_statistics.value("image_table_type").toString().contains("binary")){
            auto lbls = m_statistics.value("image_label_list").toArray();
            if (aImageResult.contains("predict_score")){
                auto score = aImageResult.value("predict_score").toDouble();
                if (score < m_max_threshold)
                    ret = lbls[0].toString();
                else if (score > m_min_threshold)
                    ret = lbls[1].toString();
                else
                    ret = "inter";
            };
        }else
            ret = aImageResult.value("predict_label").toString();
        if (ret != "")
            ret = "pred: " + ret;
    }
    else{
        if (m_images_statistics.contains(aImageID)){
            auto idx = calcThresholdIndex();
            ret = m_images_statistics.value(aImageID).value("image_value").toArray()[idx].toString();
        }
    }
    return ret;
}

void task::prepareTrainParam(QJsonObject& aParam){
    aParam.insert("num_of_image_in_group", m_channel_count);

    auto lbls = getLabels();
    QJsonArray image_label_groups, shape_labels;
    for (auto i : lbls.keys())
        if (i == "shape"){
            auto shp_lbls = lbls.value(i).toObject();
            for (auto j : shp_lbls.keys())
                shape_labels.push_back(j);
        }else
            image_label_groups.push_back(i);
    aParam.insert("image_label_groups", image_label_groups);
    aParam.insert("shape_labels", shape_labels);
}


void task::jobManagement(){
    //start Job
    auto insert_job = rea::pipeline::add<QJsonObject>([this](rea::stream<QJsonObject>* aInput){
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
    });

    rea::pipeline::add<QJsonObject>([](rea::stream<QJsonObject>* aInput){
        aInput->out();
    }, rea::Json("name", "startJob"))
    ->next(rea::pipeline::add<QJsonObject>([this](rea::stream<QJsonObject>* aInput){
        auto dt = aInput->data();
        auto prm = rea::Json(protocal.value(protocal_training).toObject().value("req").toObject(),
                             "project_id", m_project_id,
                             "task_id", m_task_id,
                             "data", prepareTrainingData(dt.value("images").toArray()));
        if (dt.value("infer").toBool()){
            auto jobs = dt.value("jobs").toArray();
            for (auto i : jobs){
                prm = rea::Json(prm, "type", "inference", "statistics", true, "model_id", (m_jobs.begin() + i.toInt()).key());
                aInput->out<QJsonObject>(prm, "callServer");
            }
        }else{
            auto train_prm = dt.value("param").toObject();
            prepareTrainParam(train_prm);
            prm = rea::Json(prm, "type", "training", "task_type", m_task_type, "params", train_prm);
            auto jobs = dt.value("jobs").toArray();
            if (jobs.size() == 0)
                aInput->out<QJsonObject>(prm, "callServer");
            else
                for (auto i : jobs)
                    aInput->out<QJsonObject>(rea::Json(prm,
                                                       "params", rea::Json(train_prm,
                                                                           "train_from_model", (m_jobs.begin() + i.toInt()).key())),
                                             "callServer");
        }
    }, rea::Json("name", "doStartJob")))
        ->next("callServer")
        ->nextB(insert_job, rea::Json("tag", protocal_training))
        ->next(insert_job, rea::Json("tag", protocal_inference))
        ->nextB("task_job_updateListView")
        ->nextB("deepsightwriteJson")
        ->nextB("task_job_listViewSelected", rea::Json("tag", "manual"))
        ->next("popMessage");

    //update image result
    rea::pipeline::find("deepsightreadJson")
        ->next(rea::pipeline::add<stgJson>([this](rea::stream<stgJson>* aInput){
           for (int i = 0; i < m_show_count; ++i)
               rea::pipeline::run<QJsonArray>("updateQSGAttrs_taskimage_gridder" + QString::number(i), updateResultObjects(aInput->data().getData(), i));
           auto img_predict = getImagePredict(m_current_image, m_image_result);
           aInput->out<QString>(img_predict, "updateImagePredictGUI");
        }), rea::Json("tag", "updateImageResult"))
        ->nextB("updateImagePredictGUI")
        ->previous(rea::pipeline::add<QJsonObject>([this](rea::stream<QJsonObject>* aInput){
               auto dt = aInput->data();
               if (dt.contains("min"))
                   m_min_threshold = dt.value("min").toDouble();
               if (dt.contains("max"))
                   m_max_threshold = dt.value("max").toDouble();
               if (dt.contains("iou"))
                   m_iou = dt.value("iou").toDouble();
               aInput->out<stgJson>(stgJson(m_image_result, ""));
               aInput->out<QJsonObject>(QJsonObject(), "updateConfuseMatrix");
        }, rea::Json("name", "thresholdChanged")))
        ->next("updateConfuseMatrix");

    //update confuse matrix
    rea::pipeline::add<QJsonObject>([this](rea::stream<QJsonObject>* aInput){
        auto dt = aInput->data();
        aInput->out<QJsonObject>(aInput->data());
        if (dt.contains("for_image"))
            m_for_image = dt.value("for_image").toBool();
        auto idx = calcThresholdIndex();
        QJsonArray lbls;
        if (m_for_image){
            lbls = m_statistics.value("image_label_list").toArray();
            if (m_statistics.value("image_table_type") == "binary_without_inter"){
                if (lbls.size() == 2){
                    auto lbl0 = lbls[0].toString(), lbl1 = lbls[1].toString();
                    auto m00 = rea::tryFind(&m_image_level_statistics, lbl0)->value(lbl0)[idx].toInt(),
                         m01 = rea::tryFind(&m_image_level_statistics, lbl0)->value(lbl1)[idx].toInt(),
                         m10 = rea::tryFind(&m_image_level_statistics, lbl1)->value(lbl0)[idx].toInt(),
                         m11 = rea::tryFind(&m_image_level_statistics, lbl1)->value(lbl1)[idx].toInt();
                    aInput->out<QJsonObject>(rea::Json("rowcap", "predict",
                                                       "colcap", "actual",
                                                       "content", rea::JArray(rea::JArray("", lbls[0].toString(), lbls[1].toString(), "total"),
                                                                              rea::JArray(lbls[0].toString(), m00, m01, m00 + m01),
                                                                              rea::JArray(lbls[1].toString(), m10, m11, m10 + m11))), "result_confuse_updateMatrix");
                    return;
                }
            }else if (m_statistics.value("image_table_type") == "multi"){
                QJsonArray content, titles;
                titles.push_back("");
                for (auto i : m_image_level_statistics.keys())
                    titles.push_back(i);
                content.push_back(titles);
                for (auto i : m_image_level_statistics.keys()){
                    QJsonArray ret;
                    ret.push_back(i);
                    auto i2 = m_image_level_statistics.value(i);
                    for (auto j : i2.keys())
                        ret.push_back(i2.value(j)[idx]);
                    content.push_back(ret);
                }
                aInput->out<QJsonObject>(rea::Json("rowcap", "attribute",
                                                   "colcap", "target",
                                                   "content", content), "result_confuse_updateMatrix");
                return;
            }else if (m_statistics.value("image_table_type") == "statistics"){

            }else //if (m_statistics.value("image_table_type") == "binary_with_inter")
            {
                if (lbls.size() == 2){
                    auto lbl0 = lbls[0].toString(), lbl1 = lbls[1].toString();
                    auto m00 = rea::tryFind(&m_image_level_statistics, lbl0)->value(lbl0)[idx].toInt(),
                         m01 = rea::tryFind(&m_image_level_statistics, lbl0)->value(lbl1)[idx].toInt(),
                         m02 = rea::tryFind(&m_image_level_statistics, lbl0)->value(QString("inter"))[idx].toInt(),
                         m10 = rea::tryFind(&m_image_level_statistics, lbl1)->value(lbl0)[idx].toInt(),
                         m11 = rea::tryFind(&m_image_level_statistics, lbl1)->value(lbl1)[idx].toInt(),
                         m12 = rea::tryFind(&m_image_level_statistics, lbl1)->value(QString("inter"))[idx].toInt();
                    aInput->out<QJsonObject>(rea::Json("rowcap", "predict",
                                                       "colcap", "actual",
                                                       "content", rea::JArray(rea::JArray("", lbls[0].toString(), lbls[1].toString(), "inter", "total"),
                                                                              rea::JArray(lbls[0].toString(), m00, m01, m02, m00 + m01 + m02),
                                                                              rea::JArray(lbls[1].toString(), m10, m11, m12, m10 + m11 + m12))), "result_confuse_updateMatrix");
                    return;
                }
            }
        }else{
            lbls = m_statistics.value("object_label_list").toArray();
            if (m_statistics.value("object_table_type") == "binary_without_inter"){

            }else if (m_statistics.value("object_table_type") == "multi"){
                QJsonArray content, titles;
                titles.push_back("");
                for (auto i : m_instance_level_statistics.keys())
                    titles.push_back(i);
                content.push_back(titles);
                for (auto i : m_instance_level_statistics.keys()){
                    QJsonArray ret;
                    ret.push_back(i);
                    auto i2 = m_instance_level_statistics.value(i);
                    for (auto j : i2.keys())
                        ret.push_back(i2.value(j)[idx]);
                    content.push_back(ret);
                }
                aInput->out<QJsonObject>(rea::Json("rowcap", "attribute",
                                                   "colcap", "target",
                                                   "content", content), "result_confuse_updateMatrix");
                return;
            }else if (m_statistics.value("object_table_type") == "statistics"){
                QJsonArray content, titles;
                idx += getIOUIndex(m_iou) * m_threshold_list.size() * m_threshold_list.size();
                for (auto i : m_instance_level_statistics.keys()){
                    QJsonArray ret;
                    ret.push_back(i);
                    auto stat = m_instance_level_statistics.value(i);
                    for (auto j : stat.keys()){
                        if (idx >= 0 && idx < stat.value(j).size())
                            ret.push_back(stat.value(j)[idx]);
                        else
                            ret.push_back(stat.value(j)[0]);
                    }

                    if (titles.size() < stat.size()){
                        titles.push_back("");
                        for (auto j : stat.keys())
                            titles.push_back(j);
                        content.push_back(titles);
                    }
                    content.push_back(ret);
                }
                aInput->out<QJsonObject>(rea::Json("rowcap", "attribute",
                                                   "colcap", "target",
                                                   "content", content), "result_confuse_updateMatrix");
                return;
            }else //if (m_statistics.value("object_table_type") == "binary_with_inter")
            {
                if (lbls.size() == 2){
                    auto lbl0 = lbls[0].toString(), lbl1 = lbls[1].toString();
                    auto m00 = rea::tryFind(&m_instance_level_statistics, lbl0)->value(lbl0)[idx].toInt(),
                         m01 = rea::tryFind(&m_instance_level_statistics, lbl1)->value(lbl0)[idx].toInt(),
                         m02 = rea::tryFind(&m_instance_level_statistics, QString("inter"))->value(lbl0)[idx].toInt(),
                         m10 = rea::tryFind(&m_instance_level_statistics, lbl0)->value(lbl1)[idx].toInt(),
                         m11 = rea::tryFind(&m_instance_level_statistics, lbl1)->value(lbl1)[idx].toInt(),
                         m12 = rea::tryFind(&m_instance_level_statistics, QString("inter"))->value(lbl1)[idx].toInt();
                    aInput->out<QJsonObject>(rea::Json("rowcap", "predict",
                                                       "colcap", "actual",
                                                       "content", rea::JArray(rea::JArray("", lbls[0].toString(), lbls[1].toString(), "inter", "total"),
                                                                              rea::JArray(lbls[0].toString(), m00, m01, m02, m00 + m01 + m02),
                                                                              rea::JArray(lbls[1].toString(), m10, m11, m12, m10 + m11 + m12))), "result_confuse_updateMatrix");
                    return;
                }
            }
        }
        aInput->out<QJsonObject>(rea::Json("content", rea::JArray(QJsonArray(), QJsonArray())), "result_confuse_updateMatrix");
    }, rea::Json("name", "updateConfuseMatrix"))
        ->next("result_confuse_updateMatrix");

    //select Job
    rea::pipeline::find("task_job_listViewSelected")
        ->next(rea::pipeline::add<QJsonArray>([this](rea::stream<QJsonArray>* aInput){
               auto dt = aInput->data();
               do{
                   aInput->out<QJsonObject>(QJsonObject(), "updateTaskJobProgress");
                   if (dt.size() == 0){
                       aInput->out<QJsonObject>(QJsonObject(), "updateTaskJobGUI");
                       aInput->out<QJsonObject>(rea::Json("log", QJsonArray()), "updateTaskJobLog");
                       break;
                   }
                   auto cur = (m_jobs.begin() + dt[0].toInt()).key();
                   aInput->out<QJsonObject>(m_jobs.value(cur).toObject(), "updateTaskJobGUI");
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
               aInput->out<QJsonObject>(rea::Json("content", rea::JArray(QJsonArray(), QJsonArray())), "result_confuse_updateMatrix");
           }), rea::Json("tag", "manual"))
        ->nextB("result_abstract_updateMatrix")
        ->nextB("_updateLineChart")
        ->nextB("_updateTHistogramGUI")
        ->nextB("result_confuse_updateMatrix")
        ->nextB("updateTaskJobProgress")
        ->nextB("updateTaskJobGUI")
        ->nextB("updateTaskJobLog")
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
                assert(0);
            }else //if (getJobState(job) == "upload_finish")
            {
                aInput->out<QJsonObject>(QJsonObject(), "updateTaskJobProgress");
                aInput->out<stgJson>(stgJson(QJsonObject(), "project/" + m_project_id + "/task/" + m_task_id + "/jobs/" + m_current_job + "/result_summary.json"));
                aInput->out<stgJson>(stgJson(QJsonObject(), "project/" + m_project_id + "/task/" + m_task_id + "/jobs/" + m_current_job + "/result_statistics.json"));
                aInput->out<stgJson>(stgJson(QJsonObject(), getImageResultJsonPath()), "deepsightreadJson");
            }
        }, rea::Json("name", "requestJobState", "thread", 5)))
        ->nextB("deepsightreadJson", rea::Json("tag", "updateImageResult"))
        ->nextB(rea::local("deepsightreadJson", rea::Json("thread", 10))
                       ->nextB(rea::buffer<stgJson>(2)->nextB(rea::pipeline::add<std::vector<stgJson>>([this](rea::stream<std::vector<stgJson>>* aInput){
                                                               auto summary = aInput->data()[0].getData();
                                                                 QJsonArray title, content;
                                                                 for (auto i : summary.keys()){
                                                                     title.push_back(i);
                                                                     content.push_back(summary.value(i));
                                                                 }
                                                                 aInput->out<QJsonObject>(rea::Json("content", rea::JArray(QJsonArray(), title, content)), "result_abstract_updateMatrix");

                                                                 m_statistics = aInput->data()[1].getData();
                                                                 updateStatisticsModel(m_statistics);
                                                                 aInput->out<QJsonArray>(getLossList(m_statistics), "_updateLineChart");
                                                                 aInput->out<QJsonObject>(rea::Json("histogram", getHistogramData(m_statistics),
                                                                                                    "threshold", rea::JArray(m_min_threshold, m_max_threshold),
                                                                                                    "show", m_threshold_list.size() > 1), "_updateTHistogramGUI");
                                                                 m_for_image = true;
                                                                 aInput->out<QJsonObject>(rea::Json("has_object", m_statistics.value("has_object")), "updateConfuseMatrix");
                                                             })->nextB("result_abstract_updateMatrix")
                                                                 ->nextB("_updateLineChart")
                                                                 ->nextB("_updateTHistogramGUI")
                                                                 ->nextB("updateConfuseMatrix"))))

        ->nextB("updateTaskJobProgress")
        ->nextB("updateTaskJobGUI")
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
                       aInput->out<QJsonObject>(rea::Json(protocal.value(protocal_upload).toObject().value("req").toObject(),
                                                          "job_id", m_current_job,
                                                          "s3_bucket_name", s3_bucket_name,
                                                          "data_root", "project/" + m_project_id + "/task/" + m_task_id + "/jobs/" + m_current_job),
                                                "callServer");
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
        ->nextB("callServer")
        ->nextB("deepsightwriteJson")
        ->nextB("task_image_listViewSelected", rea::Json("tag", "manual"))
        ->next("requestJobState");

    //update job progress
    rea::pipeline::find("receiveFromServer")
        ->next(rea::pipeline::add<QJsonObject>([this](rea::stream<QJsonObject>* aInput){
            auto dt = aInput->data();
            if (dt.value("job_id") != m_current_job)
                return;
            auto tmp = m_current_job;
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

    //inference partial images
    auto infer_tag = rea::Json("tag", "inference");
    rea::pipeline::find("task_image_listViewSelected")
        ->next(rea::pipeline::add<QJsonArray>([](rea::stream<QJsonArray>* aInput){
           if (aInput->data().size() > 0){
               aInput->cache<QJsonArray>(aInput->data())->out(aInput->data(), "task_job_listViewSelected");
           }
    }), infer_tag)
        ->next("task_job_listViewSelected", infer_tag)
        ->next(rea::pipeline::add<QJsonArray>([](rea::stream<QJsonArray>* aInput){
           if (aInput->data().size() > 0){
               aInput->out<QJsonObject>(rea::Json("jobs", aInput->data(), "infer", true, "images", aInput->cacheData<QJsonArray>(0)), "startJob");
           }
        }), infer_tag)
        ->next("startJob");

    //continue job
    rea::pipeline::find("task_job_listViewSelected")
        ->nextB(rea::pipeline::add<QJsonArray>([](rea::stream<QJsonArray>* aInput){
            aInput->out<QJsonObject>(rea::Json("jobs", aInput->data()), "startJob");
        })->nextB("startJob"), rea::Json("tag", "continueJob"))
        ->nextB(rea::pipeline::add<QJsonArray>([](rea::stream<QJsonArray>* aInput){
            aInput->out<QJsonObject>(rea::Json("jobs", aInput->data(), "infer", true), "startJob");
        })->nextB("startJob"), rea::Json("tag", "startJob"));

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
        ->nextB("deepsightdeletePath")
        ->nextB("callServer")
        ->nextB("task_job_updateListView")
        ->nextB("deepsightwriteJson")
        ->nextB("task_job_listViewSelected", rea::Json("tag", "manual"));
}

task::task(){
    auto md = new taskMode(this);
    md->initialize();
    rea::pipeline::add<task*>([](rea::stream<task*>* aInput){
        aInput->out();
    }, rea::Json("name", "loadCustomModes"));

    rea::pipeline::add<QString>([this](rea::stream<QString>* aInput){
        m_custom_modes.insert(aInput->data());
    }, rea::Json("name", "customModesLoaded"));

    rea::pipeline::run<task*>("loadCustomModes", this);

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
