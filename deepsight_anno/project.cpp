#include "reactive2.h"
#include "command.h"
#include "../storage/storage.h"
#include "model.h"
#include "imagePool.h"
#include "../util/cv.h"
#include <QRandomGenerator>
#include <QQueue>

class project : public imageModel{
protected:
   // ABSTRACT(Task)
private:
    const QString openTask = "openTask";
    const QString importImage = "importImage";
private:
    QJsonObject m_project_abstract;
    QJsonObject m_tasks;
    QJsonObject m_images;
private:
    int getChannelCount(){
        return m_project_abstract.value("channel").toString("1").toInt();
    }
    QJsonArray getImages(){
        return value("images").toArray();
    }
    void setImages(const QJsonArray& aImages){
        insert("images", aImages);
    }
    QJsonArray getTasks(){
        return value("tasks").toArray();
    }
    void setTasks(const QJsonArray& aTasks){
        insert("tasks", aTasks);
    }
    void setLabels(const QJsonObject& aLabels){
        insert("labels", aLabels);
        rea::pipeline::run<QJsonObject>("projectLabelChanged", aLabels);
    }
    QString getTaskName(const QJsonObject& aTask){
        return aTask.value("name").toString();
    }
    QString getTaskType(const QJsonObject& aTask){
        return aTask.value("type").toString();
    }
    QString getImageTime(const QJsonObject& aImage){
        return aImage.value("time").toString();
    }

    QJsonObject prepareImageListGUI(const QJsonArray& aImages){
        QJsonArray data;
        for (auto i : aImages){
            auto img = m_images.value(i.toString()).toObject();
            data.push_back(rea::Json("entry", rea::JArray(getImageStringName(img), getImageTime(img))));
        }
        return rea::Json("title", rea::JArray("name", "time"),
                         "entrycount", 30,
                         "selects", aImages.size() > 0 ? rea::JArray(0) : QJsonArray(),
                         "data", data);
    }
    QJsonObject prepareLabelListGUI(const QJsonObject& aLabels){
        QJsonArray data;
        for (auto i : aLabels.keys())
            data.push_back(rea::Json("entry", rea::JArray(i)));
        return rea::Json("title", rea::JArray("group"),
                         "selects", aLabels.size() > 0 ? rea::JArray(0) : QJsonArray(),
                         "data", data);
    }
    QJsonObject prepareTaskListGUI(const QJsonArray& aTasks){
        QJsonArray data;
        for (auto i : aTasks)
            data.push_back(rea::Json("entry", rea::JArray(getTaskName(m_tasks.value(i.toString()).toObject()))));
        return rea::Json("title", rea::JArray("name"),
                         "selects", aTasks.size() > 0 ? rea::JArray(0) : QJsonArray(),
                         "data", data);
    }
    void insertTask(QJsonObject& aTask){
        auto mdls = getTasks();
        auto tm = QDateTime::currentDateTime().toString(Qt::DateFormat::ISODate);
        auto tms = tm.split("T");
        auto id = rea::generateUUID();
        mdls.push_back(id);
        m_tasks.insert(id, rea::Json(aTask, "time", tms[0] + " " + tms[1]));
        setTasks(mdls);
    }
private:
    void projectManagement(){
        //open project
        rea::pipeline::add<QJsonObject>([](rea::stream<QJsonObject>* aInput){
            aInput->out();
        }, rea::Json("name", "openProject"))
            ->next(rea::pipeline::add<QJsonObject>([this](rea::stream<QJsonObject>* aInput){
                auto dt = aInput->data();
                if (dt.value("id") != m_project_id){
                    m_project_id = dt.value("id").toString();
                    m_project_abstract = dt.value("abstract").toObject();
                    aInput->out<stgJson>(stgJson(QJsonObject(), "project/" + m_project_id + "/task.json"));
                    aInput->out<stgJson>(stgJson(QJsonObject(), "project/" + m_project_id + "/image.json"));
                    aInput->out<stgJson>(stgJson(QJsonObject(), "project/" + m_project_id + ".json"));
                }
            }))
            ->next(rea::local("deepsightreadJson", rea::Json("thread", 10)))
            ->next(rea::buffer<stgJson>(3))
            ->next(rea::pipeline::add<std::vector<stgJson>>([this](rea::stream<std::vector<stgJson>>* aInput){
                auto dt = aInput->data();
                m_tasks = dt[0].getData();
                m_images = dt[1].getData();
                dt[2].getData().swap(*this);

                auto tsks = getTasks();
                auto imgs = getImages();
                auto lbls = getLabels();
                bool dflt = false;
                if (!lbls.contains("shape")){
                    lbls.insert("shape", QJsonObject());
                    dflt = true;
                }
                if (!lbls.contains("default")){
                    lbls.insert("default", rea::Json("NG", rea::Json("color", "red"),
                                                     "OK", rea::Json("color", "green")));
                    dflt = true;
                }
                if (dflt)
                    setLabels(lbls);
                else
                    setLabels(value("labels").toObject());

                aInput->out<QJsonObject>(prepareTaskListGUI(tsks), "project_task_updateListView");
                aInput->out<QJsonObject>(prepareImageListGUI(imgs), "project_image_updateListView");
                aInput->out<QJsonObject>(prepareLabelListGUI(lbls), "project_label_updateListView");
                aInput->out<QJsonArray>(QJsonArray(), "project_task_listViewSelected");
                //aInput->out<QJsonArray>(QJsonArray(), "project_image_listViewSelected");
                aInput->out<QJsonObject>(rea::Json("count", 1), "scatterProjectImageShow");
                aInput->out<QJsonArray>(QJsonArray(), "project_label_listViewSelected");
                aInput->out<QJsonObject>(getFilter(), "updateProjectImageFilterGUI");
            }))
            ->nextB("updateTaskGUI")
            ->nextB("project_task_updateListView")
            ->nextB("project_image_updateListView")
            ->nextB("project_label_updateListView")
            ->nextB("project_task_listViewSelected", rea::Json("tag", "manual"))
            ->nextB("scatterProjectImageShow")
            ->nextB("updateProjectImageFilterGUI")
            //->nextB("project_image_listViewSelected", rea::Json("tag", "manual"))
            ->next("project_label_listViewSelected", rea::Json("tag", "project_manual"));
    }
    void taskManagement(){
        //new task
        rea::pipeline::find("_newObject")
            ->next(rea::pipeline::add<QJsonObject>([this](rea::stream<QJsonObject>* aInput){
                       auto tsk = aInput->data();
                       auto nm = tsk.value("name").toString();
                       if (nm == "")
                           aInput->out<QJsonObject>(rea::Json("title", "warning", "text", "Invalid name!"), "popMessage");
                       else{
                           insertTask(tsk);
                           auto tsks = getTasks();
                           aInput->out<QJsonObject>(prepareTaskListGUI(tsks), "project_task_updateListView");
                           aInput->out<stgJson>(stgJson(*this, "project/" + m_project_id + ".json"), "deepsightwriteJson");
                           aInput->out<stgJson>(stgJson(m_tasks, "project/" + m_project_id + "/task.json"), "deepsightwriteJson");
                           aInput->out<QJsonArray>(QJsonArray(), "project_task_listViewSelected");
                       }
                   }), rea::Json("tag", "newTask"))
            ->nextB("popMessage")
            ->nextB("project_task_updateListView")
            ->nextB("project_task_listViewSelected", rea::Json("tag", "manual"))
            ->next("deepsightwriteJson");

        //select task
        rea::pipeline::find("project_task_listViewSelected")
            ->next(rea::pipeline::add<QJsonArray>([this](rea::stream<QJsonArray>* aInput){
                       auto dt = aInput->data();
                       if (dt.size() == 0)
                           aInput->out<QJsonObject>(QJsonObject(), "updateTaskGUI");
                       else{
                           auto idx = dt[0].toInt();
                           auto tsks = getTasks();
                           if (idx < tsks.size()){
                               auto nm = tsks[idx].toString();
                               aInput->out<QJsonObject>(m_tasks.value(nm).toObject(), "updateTaskGUI");
                           }
                           else
                               aInput->out<QJsonObject>(QJsonObject(), "updateTaskGUI");
                       }
                   }), rea::Json("tag", "manual"))
            ->next("updateTaskGUI");

        //delete task
        rea::pipeline::find("_makeSure")
            ->next(rea::local("project_task_listViewSelected"), rea::Json("tag", "deleteTask"))
            ->next(rea::pipeline::add<QJsonArray>([this](rea::stream<QJsonArray>* aInput){
                auto dt = aInput->data();

                if (dt.size() > 0){
                    std::vector<int> idxes;
                    for (auto i : dt)
                        idxes.push_back(i.toInt());
                    std::sort(idxes.begin(), idxes.end(), std::greater<int>());

                    QStringList dels;
                    auto tsks = getTasks();
                    for (auto i : idxes){
                        auto nm = tsks[i].toString();
                        dels.push_back(nm);
                        tsks.erase(tsks.begin() + i);
                    }
                    setTasks(tsks);
                    aInput->out<stgJson>(stgJson(*this, "project/" + m_project_id + ".json"), "deepsightwriteJson");

                    if (dels.size() > 0){
                        for (auto i : dels){
                            m_tasks.remove(i);
                            aInput->out<QString>("project/" + m_project_id + "/task/" + i + ".json", "deepsightdeletePath");
                            aInput->out<QString>("project/" + m_project_id + "/task/" + i, "deepsightdeletePath");
                        }
                        aInput->out<stgJson>(stgJson(m_tasks, "project/" + m_project_id + "/task.json"), "deepsightwriteJson");
                    }

                    aInput->out<QJsonObject>(prepareTaskListGUI(getTasks()), "project_task_updateListView");
                    aInput->out<QJsonArray>(QJsonArray(), "project_task_listViewSelected");
                }
            }))
            ->nextB("project_task_updateListView")
            ->nextB("project_task_listViewSelected", rea::Json("tag", "manual"))
            ->nextB("deepsightdeletePath")
            ->next("deepsightwriteJson");

        //open task
        rea::pipeline::find("project_task_listViewSelected")
            ->next(rea::pipeline::add<QJsonArray>([this](rea::stream<QJsonArray>* aInput){
                       auto dt = aInput->data();
                       if (dt.size() > 0){
                           auto tsks = getTasks();
                           auto id = tsks[dt[0].toInt()].toString();
                           aInput->out<QJsonArray>(QJsonArray({rea::GetMachineFingerPrint(), getProjectName(m_project_abstract), getTaskName(m_tasks.value(id).toObject())}), "title_updateStatus");
                           aInput->out<IProjectInfo>(IProjectInfo(&m_images, rea::Json("id", id,
                                                              "labels", getLabels(),
                                                              "channel", getChannelCount(),
                                                              "project", m_project_id,
                                                              "imageshow", getImageShow(),
                                                              "type", getTaskType(m_tasks.value(id).toObject()))), openTask);
                       }
                   }), rea::Json("tag", openTask))
            ->nextB("title_updateStatus")
            ->next(openTask);
    }
    void labelManagement(){
        //select label group
        rea::pipeline::find("project_label_listViewSelected")
            ->next(rea::pipeline::add<QJsonArray, rea::pipePartial>([this](rea::stream<QJsonArray>* aInput){
                       auto dt = aInput->data();
                       if (dt.size() == 0)
                           aInput->out<QJsonObject>(QJsonObject());
                       else{
                           auto idx = dt[0].toInt();
                           auto lbls = getLabels();
                           if (idx < lbls.size()){
                               aInput->out<QJsonObject>(rea::Json("key", (lbls.begin() + idx).key(), "val", (lbls.begin() + idx)->toObject()));
                           }
                           else
                               aInput->out<QJsonObject>(QJsonObject());
                       }
                   }, rea::Json("name", "getProjectLabel")), rea::Json("tag", "project_manual"))
            ->next(rea::local("updateProjectLabelGUI"), rea::Json("tag", "project_manual"));

        //new label group
        rea::pipeline::find("_newObject")
            ->next(rea::pipeline::add<QJsonObject>([this](rea::stream<QJsonObject>* aInput){
                       auto grp = aInput->data().value("group").toString();
                       auto lbls = getLabels();
                       if (lbls.contains(grp))
                           aInput->out<QJsonObject>(rea::Json("title", "warning", "text", "Existed group!"), "popMessage");
                       else{
                           setLabels(rea::Json(lbls, grp, QJsonObject()));
                           aInput->out<QJsonObject>(prepareLabelListGUI(lbls), "project_label_updateListView");
                           aInput->out<stgJson>(stgJson(*this, "project/" + m_project_id + ".json"), "deepsightwriteJson");
                           aInput->out<QJsonArray>(QJsonArray(), "project_label_listViewSelected");
                       }
                   }), rea::Json("tag", "newLabelGroup"))
            ->nextB("popMessage")
            ->nextB("project_label_updateListView")
            ->nextB("project_label_listViewSelected", rea::Json("tag", "project_manual"))
            ->next("deepsightwriteJson");

        //new label
        auto newLabel = rea::buffer<QJsonArray>(2);
        auto newLabel_tag = rea::Json("tag", "newLabel");
        rea::pipeline::find("_newObject")
            ->next(rea::pipeline::add<QJsonObject>([](rea::stream<QJsonObject>* aInput){
                       aInput->out<QJsonArray>(rea::JArray(aInput->data().value("label")));
                       aInput->out<QJsonArray>(QJsonArray(), "project_label_listViewSelected");
                   }), newLabel_tag)
            ->nextB(rea::pipeline::find("project_label_listViewSelected")->nextB(newLabel, newLabel_tag), newLabel_tag)
            ->next(newLabel)
            ->next(rea::pipeline::add<std::vector<QJsonArray>>([this](rea::stream<std::vector<QJsonArray>>* aInput){
                auto dt = aInput->data();
                auto lbl = dt[0][0].toString();
                if (lbl == ""){
                    aInput->out<QJsonObject>(rea::Json("title", "warning", "text", "Invalid label!"), "popMessage");
                    return;
                }
                auto idx = dt[1][0].toInt();
                auto grps = getLabels();
                auto lbls = (grps.begin() + idx)->toObject();
                if (lbls.contains(lbl)){
                    aInput->out<QJsonObject>(rea::Json("title", "warning", "text", "Existed label!"), "popMessage");
                    return;
                }
                lbls.insert(lbl, rea::Json("color", QColor::fromRgb(QRandomGenerator::global()->generate()).name()));
                auto key = (grps.begin() + idx).key();
                grps.insert(key, lbls);
                setLabels(grps);
                aInput->out<stgJson>(stgJson(*this, "project/" + m_project_id + ".json"), "deepsightwriteJson");
                aInput->out<QJsonObject>(rea::Json("key", key, "val", lbls), "updateProjectLabelGUI");
            }))
            ->nextB("popMessage")
            ->nextB("updateProjectLabelGUI")
            ->next("deepsightwriteJson");

        //delete label group
        rea::pipeline::find("_makeSure")
            ->next(rea::local("project_label_listViewSelected"), rea::Json("tag", "deleteLabelGroup"))
            ->next(rea::pipeline::add<QJsonArray>([this](rea::stream<QJsonArray>* aInput){
                auto dt = aInput->data();

                if (dt.size() > 0){
                    std::vector<int> idxes;
                    for (auto i : dt)
                        idxes.push_back(i.toInt());
                    std::sort(idxes.begin(), idxes.end(), std::greater<int>());

                    bool erased = false;
                    auto lbls = getLabels();
                    for (auto i : idxes){
                        auto lbl = lbls.begin() + i;
                        if (lbl.key() == "shape" || lbl.key() == "default")
                            continue;
                        erased = true;
                        lbls.erase(lbl);
                    }
                    if (erased){
                        setLabels(lbls);
                        aInput->out<QJsonObject>(prepareLabelListGUI(lbls), "project_label_updateListView");
                        aInput->out<QJsonArray>(QJsonArray(), "project_label_listViewSelected");
                        aInput->out<stgJson>(stgJson(*this, "project/" + m_project_id + ".json"), "deepsightwriteJson");
                    }
                }
            }))
            ->nextB("project_label_updateListView")
            ->nextB("project_label_listViewSelected", rea::Json("tag", "project_manual"))
            ->next("deepsightwriteJson");

        //delete label
        auto deleteLabel = rea::buffer<QJsonArray>(2);
        auto deleteLabel_tag = "deleteLabel";
        rea::pipeline::add<QString>([](rea::stream<QString>* aInput){
            aInput->out<QJsonArray>(rea::JArray(aInput->data()));
            aInput->out<QJsonArray>(QJsonArray(), "project_label_listViewSelected");
        }, rea::Json("name", deleteLabel_tag))
            ->nextB(rea::pipeline::find("project_label_listViewSelected")->nextB(deleteLabel, rea::Json("tag", deleteLabel_tag)), rea::Json("tag", deleteLabel_tag))
            ->next(deleteLabel)
            ->next(rea::pipeline::add<std::vector<QJsonArray>>([this](rea::stream<std::vector<QJsonArray>>* aInput){
                auto dt = aInput->data();
                auto lbl = dt[0][0].toString();

                auto idx = dt[1][0].toInt();
                auto grps = getLabels();
                auto lbls = (grps.begin() + idx)->toObject();

                lbls.remove(lbl);
                auto key = (grps.begin() + idx).key();
                grps.insert(key, lbls);
                setLabels(grps);
                aInput->out<stgJson>(stgJson(*this, "project/" + m_project_id + ".json"), "deepsightwriteJson");
                aInput->out<QJsonObject>(rea::Json("key", key, "val", lbls), "updateProjectLabelGUI");
            }))
            ->nextB("updateProjectLabelGUI")
            ->next("deepsightwriteJson");

        // modify label color
        const QString modifyLabel = "modifyLabelColor";
        rea::pipeline::find("project_label_listViewSelected")
            ->next(rea::buffer<QJsonArray>(2, modifyLabel), rea::Json("tag", modifyLabel))
            ->next(rea::pipeline::add<std::vector<QJsonArray>>([this](rea::stream<std::vector<QJsonArray>>* aInput){
                auto dt = aInput->data();
                auto lbl = dt[0][0].toString();
                auto clr = dt[0][1].toString();
                auto idx = dt[1][0].toInt();
                auto grps = getLabels();
                auto lbls = (grps.begin() + idx)->toObject();

                auto cfg = lbls.value(lbl).toObject();
                cfg.insert("color", clr);
                lbls.insert(lbl, cfg);

                auto key = (grps.begin() + idx).key();
                grps.insert(key, lbls);
                setLabels(grps);
                aInput->out<stgJson>(stgJson(*this, "project/" + m_project_id + ".json"), "deepsightwriteJson");
                aInput->out<QJsonObject>(rea::Json("key", key, "val", lbls), "updateProjectLabelGUI");
            }))
            ->nextB("updateProjectLabelGUI")
            ->next("deepsightwriteJson");

        //modifyImageLabel
        const QString modifyImageLabel_nm = "modifyProjectImageLabel";
        auto modifyImageLabel_tag = rea::Json("tag", modifyImageLabel_nm);
        rea::pipeline::add<QJsonObject>([](rea::stream<QJsonObject>* aInput){
            aInput->cache<QJsonObject>(aInput->data())->out<QJsonArray>(QJsonArray(), "project_image_listViewSelected");
        }, rea::Json("name", modifyImageLabel_nm))
        ->next("project_image_listViewSelected", modifyImageLabel_tag)
        ->next(rea::pipeline::add<QJsonArray>([this](rea::stream<QJsonArray>* aInput){
            auto st = aInput->cacheData<QJsonObject>(0);
            auto sels = aInput->data();
            auto imgs = getImages();
            for (auto i : sels){
                auto img = imgs[i.toInt()].toString();
                auto abs = m_images.value(img).toObject();
                auto lbls = getImageLabels(abs);
                lbls.insert(st.value("group").toString(), st.value("label"));
                setImageLabels(abs, lbls);
                m_images.insert(img, abs);
            }
            aInput->out<stgJson>(stgJson(m_images, "project/" + m_project_id + "/image.json"), "deepsightwriteJson");
        }), modifyImageLabel_tag)
        ->next("deepsightwriteJson");

        rea::pipeline::add<QJsonObject>([](rea::stream<QJsonObject>* aInput){
            aInput->out();
        }, rea::Json("name", "projectLabelChanged"));
    }
private:
    std::vector<stgCVMat> m_cvmat_cache;
    QHash<QString, int> m_show_cache;
    const QJsonObject selectProjectImage = rea::Json("tag", "selectProjectImage");

    QJsonObject getImageShow(){
        return value("imageShow").toObject();
    }

    void setImageShow(const QJsonObject& aImageShow){
        insert("imageShow", aImageShow);
    }

    QString getResizeMode(const QJsonObject& aImageShow){
        return aImageShow.value("resizeMode").toString("linear");
    }

    void setResizeMode(QJsonObject& aImageShow, const QString& aResizeMode){
        aImageShow.insert("resizeMode", aResizeMode);
    }

    QString getColorFormat(const QJsonObject& aImageShow){
        return aImageShow.value("colorFormat").toString("None");
    }

    void setColorFormat(QJsonObject& aImageShow, const QString& aColorFormat){
        aImageShow.insert("colorFormat", aColorFormat);
    }

    void imageManagement(){
        //select image
        rea::pipeline::find("project_image_listViewSelected")
            ->next(rea::pipeline::add<QJsonArray>([this](rea::stream<QJsonArray>* aInput){
                       auto dt = aInput->data();
                       if (dt.size() == 0){
                           rea::pipeline::run<QJsonArray>("updateQSGCtrl_projectimage_gridder0", QJsonArray());  //must be before "updateQSGModel_projectimage_gridder"
                           m_current_image = "";
                           for (int i = 0; i < m_show_count; ++i)
                               rea::pipeline::run<QJsonObject>("updateQSGModel_projectimage_gridder" + QString::number(i), QJsonObject());
                           aInput->out<QJsonObject>(QJsonObject(), "updateProjectImageGUI");
                       }
                       else{
                           auto idx = dt[0].toInt();
                           auto imgs = getImages();
                           if (idx < imgs.size()){
                               auto nm = imgs[idx].toString();
                               if (nm != m_current_image){
                                   aInput->out<QJsonArray>(QJsonArray(), "updateQSGCtrl_projectimage_gridder0");
                                   m_current_image = nm;
                                   auto nms = getImageName(m_images.value(m_current_image).toObject());
                                   aInput->out<stgJson>(stgJson(QJsonObject(), "project/" + m_project_id + "/image/" + nm + ".json"), "deepsightreadJson", selectProjectImage);
                                   for (auto i = 0; i < m_show_count; ++i){
                                       auto img = "project/" + m_project_id + "/image/" + nm + "/" + nms[i].toString();
                                       m_show_cache.insert(img, i);
                                       aInput->out<stgCVMat>(stgCVMat(cv::Mat(), img));
                                   }
                               }
                               //aInput->out<QJsonObject>(m_images.value(nm).toObject(), "updateProjectImageGUI");
                           }else{
                               aInput->out<QJsonObject>(QJsonObject(), "updateProjectImageGUI");
                           }
                       }
                   }), rea::Json("tag", "manual"))
            ->nextB("updateQSGCtrl_projectimage_gridder0")
            ->nextB(rea::pipeline::find("deepsightreadJson")
                           ->nextB(rea::pipeline::add<stgJson>([this](rea::stream<stgJson>* aInput){
                                       m_image = aInput->data().getData();
                                       aInput->out<QJsonObject>(rea::Json(m_image, "image_label", getImageLabels(m_images.value(m_current_image).toObject())), "updateProjectImageGUI");
                                      })->nextB("updateProjectImageGUI"),
                                   selectProjectImage),
                    selectProjectImage)
            ->nextB("updateProjectImageGUI")
            ->next(rea::local("deepsightreadCVMat", rea::Json("thread", 10)))
            ->next(rea::pipeline::add<stgCVMat>([this](rea::stream<stgCVMat>* aInput){
                auto dt = aInput->data();
                auto ch = m_show_cache.value(dt);
                auto pth = QString(dt);
                auto img = cvMat2QImage(dt.getData());
                rea::imagePool::cacheImage(pth, img);

                auto objs = rea::Json("img_" + m_current_image, rea::Json(
                                 "type", "image",
                                 "range", rea::JArray(0, 0, img.width(), img.height()),
                                 "path", pth,
                                 "text", QJsonObject(),
                                 "transform", getImageShow()));
                auto shps = getShapes(m_image);
                for (auto i : shps.keys()){
                    auto shp = shps.value(i).toObject();
                    if (shp.value("type") == "ellipse"){
                        objs.insert(i, rea::Json("type", "ellipse",
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
                        objs.insert(i, rea::Json("type", "poly",
                                                 "caption", shp.value("label"),
                                                 "points", pts));
                    }
                }

                auto cfg = rea::Json("id", "project/" + m_project_id + "/image/" + m_current_image + ".json",
                                     "width", img.width(),
                                     "height", img.height(),
                                     "face", 100,
                                     "text", rea::Json("visible", true,
                                                       "size", rea::JArray(80, 30)
                                                       //"location", "bottom"
                                                       ),
                                     "objects", objs);
                rea::pipeline::run<QJsonObject>("updateQSGModel_projectimage_gridder" + QString::number(ch), cfg);
                //aInput->out<QJsonObject>(cfg, "updateQSGModel_projectimage_gridder0");
            }));

        //import image
        rea::pipeline::find("_selectFile")
            ->next(rea::pipeline::add<QJsonArray>([this](rea::stream<QJsonArray>* aInput){
                       m_cvmat_cache.clear();
                       auto pths = aInput->data();
                       auto remain = pths.size() % getChannelCount();
                       while (remain--)
                           pths.pop_back();
                       aInput->out<QJsonObject>(rea::Json("title", importImage, "sum", pths.size()), "updateProgress");
                       for (auto i : pths)
                           aInput->out<stgCVMat>(stgCVMat(cv::Mat(), i.toString()));
                   }), rea::Json("tag", importImage))
            ->nextB("updateProgress")
            ->next(rea::local("readCVMat", rea::Json("thread", 10)))
            ->next(rea::pipeline::add<stgCVMat>([this](rea::stream<stgCVMat>* aInput){
                m_cvmat_cache.push_back(aInput->data());
                int sz = int(m_cvmat_cache.size());
                if (sz == getChannelCount()){
                    auto w = m_cvmat_cache[0].getData().cols, h = m_cvmat_cache[0].getData().rows;
                    for (auto i : m_cvmat_cache){ //make sure the same width and height
                        if (i.getData().cols != w || i.getData().rows != h){
                            aInput->out<QJsonObject>(rea::Json("step", sz), "updateProgress");
                            return;
                        }
                    }

                    auto id = rea::generateUUID();
                    QJsonArray nms;
                    QJsonArray localpth;
                    QJsonArray channels;
                    for (auto i : m_cvmat_cache){
                        localpth.push_back(i);
                        channels.push_back(i.getData().channels());
                        auto nm = i.mid(i.lastIndexOf("/") + 1, i.length());
                        nms.push_back(nm);
                        aInput->out<stgCVMat>(stgCVMat(i.getData(), "project/" + m_project_id + "/image/" + id + "/" + nm));
                    }
                    auto tm = QDateTime::currentDateTime().toString(Qt::DateFormat::ISODate);
                    auto tms = tm.split("T");
                    m_images.insert(id, rea::Json("name", nms, "time", tms[0] + " " + tms[1]));  //update image.json
                    aInput->out<stgJson>(stgJson(rea::Json("width", w,  //update image_id.json
                                                           "height", h,
                                                           "channel", channels,
                                                           "source", nms,
                                                           "local", localpth),
                                                 "project/" + m_project_id + "/image/" + id + ".json"), "deepsightwriteJson");

                    auto imgs = getImages();
                    imgs.push_back(id);
                    setImages(imgs);   //update project_id.json

                    m_cvmat_cache.clear();
                }
            }))
            ->nextB("updateProgress")
            ->nextB("deepsightwriteJson")
            ->next(rea::local("deepsightwriteCVMat", rea::Json("thread", 11)))
            ->next(rea::pipeline::add<stgCVMat>([](rea::stream<stgCVMat>* aInput){
                aInput->out<QJsonObject>(QJsonObject());
            }))
            ->next(rea::local("updateProgress"))
            ->next(rea::pipeline::add<double>([this](rea::stream<double>* aInput){
                if (aInput->data() == 1.0){
                    aInput->out<stgJson>(stgJson(m_images, "project/" + m_project_id + "/image.json"), "deepsightwriteJson");
                    aInput->out<stgJson>(stgJson(*this, "project/" + m_project_id + ".json"));
                    aInput->out<QJsonObject>(prepareImageListGUI(getImages()), "project_image_updateListView");
                }
            }))
            ->nextB("project_image_updateListView")
            ->nextB(rea::local("deepsightwriteJson", rea::Json("thread", 11)))
            ->next("deepsightwriteJson", rea::Json("tag", "updateProjectImages"));

        //filter images
        rea::pipeline::add<QJsonObject>([this](rea::stream<QJsonObject>* aInput){
            auto dt = aInput->data();
            QJsonArray imgs;
            if (dt.value("type") == "all"){
                for (auto i : m_images.keys())
                    imgs.push_back(i);
            }else if (dt.value("type") == "time"){
                auto tm = dt.value("value").toString();
                if (tm != "")
                    for (auto i : m_images.keys()){
                        auto img = m_images.value(i).toObject();
                        if (img.value("time").toString().indexOf(tm) >= 0)
                            imgs.push_back(i);
                    }
            }else if (dt.value("type") == "name"){
                auto nm = dt.value("value").toString();
                if (nm != "")
                    for (auto i : m_images.keys()){
                        auto nms = getImageName(m_images.value(i).toObject());
                        for (auto j : nms)
                            if (j.toString().indexOf(nm) >= 0){
                                imgs.push_back(i);
                                break;
                            }
                    }
            }else
                return;
            setImages(imgs);
            setFilter(dt);
            aInput->out<QJsonObject>(prepareImageListGUI(imgs), "project_image_updateListView");
            aInput->out<QJsonArray>(QJsonArray(), "project_image_listViewSelected");
            aInput->out<stgJson>(stgJson(*this, "project/" + m_project_id + ".json"), "deepsightwriteJson");
        }, rea::Json("name", "filterProjectImages"))
        ->nextB("project_image_updateListView")
        ->nextB("project_image_listViewSelected", rea::Json("tag", "manual"))
        ->nextB("deepsightwriteJson");

        //modify image
        rea::pipeline::find("QSGAttrUpdated_projectimage_gridder0")
            ->next(rea::pipeline::add<QJsonArray>([this](rea::stream<QJsonArray>* aInput){
                if (m_current_image == "")
                    return;
                QString cur = "project/" + m_project_id + "/image/" + m_current_image + ".json";
                QString pth = cur;
                if (modifyImage(aInput->data(), m_image, pth))
                    aInput->out<stgJson>(stgJson(m_image, pth), "deepsightwriteJson");
                else if (pth != cur){
                    aInput->cache<QJsonArray>(aInput->data())->out<stgJson>(stgJson(QJsonObject(), pth));
                }
            }))
            ->nextB("deepsightwriteJson")
            ->next(rea::local("deepsightreadJson", rea::Json("thread", 10)))
            ->next(rea::pipeline::add<stgJson>([this](rea::stream<stgJson>* aInput){
                auto dt = aInput->data().getData();
                auto pth = QString(aInput->data());
                modifyImage(aInput->cacheData<QJsonArray>(0), dt, pth);
                aInput->out<stgJson>(stgJson(dt, pth), "deepsightwriteJson");
            }))
            ->next("deepsightwriteJson");

        //get project current image info
        rea::pipeline::add<QJsonObject, rea::pipePartial>([this](rea::stream<QJsonObject>* aInput){
            if (m_current_image != ""){
                auto nms = getImageName(m_images.value(m_current_image).toObject());
                QJsonArray imgs;
                for (auto i : nms)
                    imgs.push_back("project/" + m_project_id + "/image/" + m_current_image + "/" + i.toString());
                aInput->setData(rea::Json("images", imgs, "show", getImageShow()))->out();
            }
        }, rea::Json("name", "getProjectCurrentImage"));
    }
private:
    int m_show_count = 1;
    const QString setImageShow0 = "setImageShow";
    void guiManagement(){
        //the qsgnode will be destroyed by qt after switch page
        rea::pipeline::find("title_updateStatus")
            ->next(rea::pipeline::add<QJsonArray>([this](rea::stream<QJsonArray>* aInput){
                if (aInput->data().size() == 2)
                    aInput->out<QJsonArray>(QJsonArray(), "project_image_listViewSelected");
                else
                    m_current_image = "";
            }))
            ->next("project_image_listViewSelected", rea::Json("tag", "manual"));

        //switch scatter mode
        rea::pipeline::add<QJsonObject>([this](rea::stream<QJsonObject>* aInput){
            auto dt = aInput->data();
            auto ch = getChannelCount();
            if (dt.contains("count"))
                m_show_count = dt.value("count").toInt();
            else
                m_show_count = m_show_count == ch ? 1 : ch;
            aInput->out<QJsonObject>(rea::Json("size", m_show_count), "projectimage_updateViewCount");
            m_current_image = "";
            aInput->out<QJsonArray>(QJsonArray(), "project_image_listViewSelected");
        }, rea::Json("name", "scatterProjectImageShow"))
            ->nextB("projectimage_updateViewCount")
            ->next("project_image_listViewSelected", rea::Json("tag", "manual"));

        //modify image show
        rea::pipeline::add<QJsonObject>([this](rea::stream<QJsonObject>* aInput){
            auto show = getImageShow();
            auto md = getResizeMode(show);
            auto fmt = getColorFormat(show);
            QVector<QString> mds{"linear", "nearest", "cubic", "area", "lanczos4"}, fmts{"None", "BayerRG2RGB", "BayerRG2Gray", "RGB2Gray"};
            aInput->out<QJsonObject>(rea::Json("title", "image show",
                                               "content", rea::Json("resizeMode", rea::Json("model", rea::JArray("linear", "nearest", "cubic", "area", "lanczos4"), "index", mds.indexOf(md)),
                                                                    "colorFormat", rea::Json("model", rea::JArray("None", "BayerRG2RGB", "BayerRG2Gray", "RGB2Gray"), "index", fmts.indexOf(fmt))),
                                               "tag", rea::Json("tag", setImageShow0)), "_newObject");
        }, rea::Json("name", setImageShow0))
        ->next("_newObject")
        ->next(rea::pipeline::add<QJsonObject>([this](rea::stream<QJsonObject>* aInput){
            auto dt = aInput->data();
            auto show = getImageShow();
            bool mdy = false;
            for (auto i : dt.keys()){
                if (dt.value(i) != show.value(i)){
                    show.insert(i, dt.value(i));
                    mdy = true;
                }
            }
            if (mdy){
                setImageShow(show);
                aInput->out<stgJson>(stgJson(*this, "project/" + m_project_id + ".json"), "deepsightwriteJson");
                if (m_current_image != ""){
                    for (int i = 0; i < m_show_count; ++i)
                        rea::pipeline::run<QJsonObject>("updateQSGAttr_projectimage_gridder" + QString::number(i),
                                                        rea::Json("obj", "img_" + m_current_image, "key", rea::JArray("transform"), "val", show));
                }
            }
        }), rea::Json("tag", setImageShow0))
            ->nextB("deepsightwriteJson");
    }
public:
    project(){
        projectManagement();
        taskManagement();
        labelManagement();
        imageManagement();
        guiManagement();
    }
};

static rea::regPip<QQmlApplicationEngine*> init_proj([](rea::stream<QQmlApplicationEngine*>* aInput){
    static project cur_proj;
    aInput->out();
}, QJsonObject(), "regQML");
