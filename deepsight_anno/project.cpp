#include "reactive2.h"
#include "command.h"
#include "../storage/storage.h"
#include "model.h"
#include "imagePool.h"
#include "../util/cv.h"
#include <QRandomGenerator>
#include <QQueue>
#include <QFileInfo>

QJsonObject parseProjectV3Shapes(const QJsonObject& aShapes){
    QJsonObject ret;
    for (auto i : aShapes){
        auto shp = i.toObject();
        auto tp = shp.value("type");
        auto id = "shp_" + rea::generateUUID();
        if (tp == "polyline"){
            ret.insert(id, rea::Json("type", "polyline",
                                     "points", shp.value("points"),
                                     "label", shp.value("label")));
        }else if (tp == "rectangle"){
            auto rect = shp.value("points").toArray();
            auto pts = QJsonArray({rect[0], rect[1], rect[0], rect[3], rect[2], rect[3], rect[2], rect[1], rect[0], rect[1]});
            ret.insert(id, rea::Json("type", "polyline",
                                     "points", pts,
                                     "label", shp.value("label")));
        }else if (tp == "ellipse"){
            shp.remove("line_color");
            ret.insert(id, shp);
        }
    }
    /*for (auto i : aShapes){
        auto shp = i.toObject();
        auto tp = shp.value("shape_type");
        auto id = "shp_" + rea::generateUUID();
        QJsonArray pts;
        auto pts0 = shp.value("points").toArray();
        if (tp == "polygon"){
            for (auto i : pts0){
                auto pt = i.toArray();
                pts.push_back(pt[0]);
                pts.push_back(pt[1]);
            }
            pts.push_back(pts[0]);
            pts.push_back(pts[1]);
            ret.insert(id, rea::Json("type", "polyline",
                                     "points", pts,
                                     "label", shp.value("label")));
        }else if (tp == "rectangle"){
            auto lt = pts0[0].toArray(), rb = pts0[1].toArray();
            ret.insert(id, rea::Json("type", "polyline",
                                     "points", rea::JArray(lt[0], lt[1], lt[0], rb[1], rb[0], rb[1], rb[0], lt[1], lt[0], lt[1]),
                                     "label", shp.value("label")));
        }else if (tp == "circle"){
            auto ct0 = pts0[0].toArray(), pt0 = pts0[1].toArray();
            auto ct = QPointF(ct0[0].toDouble(), ct0[1].toDouble()), pt = QPointF(pt0[0].toDouble(), pt0[1].toDouble());
            auto del = (ct - pt);
            auto r = sqrt(QPointF::dotProduct(del, del));
            ret.insert(id, rea::Json("type", "ellipse",
                                     "center", ct0,
                                     "xradius", r,
                                     "yradius", r,
                                     "label", shp.value("label")));
        }
    }*/
    return ret;
}

class project : public imageModel{
protected:
   // ABSTRACT(Task)
    QJsonObject getImageAbstracts() override{
        return m_images;
    }
    int getChannelCount() override{
        if (m_project_abstract.value("channel").isString())
            return m_project_abstract.value("channel").toString().toInt();
        else
            return m_project_abstract.value("channel").toInt();
    }
private:
    const QString importImage = "importImage";
    const QString openTask = "openTask";
private:
    QJsonObject m_project_abstract;
    QJsonObject m_tasks;
    QJsonObject m_images;
    QJsonObject m_image_filter;
    QString m_current_task;
    int m_project_image_index;
    int m_task_image_index;
    int m_last_state = 0;
private:
    QJsonArray getImageList(){
        return value("images").toArray();
    }
    void setImageList(QJsonArray& aImageList){
        sortImagesByTime(aImageList, m_images);
        insert("images", aImageList);
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

    QJsonArray getImageFilter(){
        return m_image_filter.value("operators").toArray();
    }

    QJsonObject prepareImageListGUI(const QJsonObject& aImageAbstract, const QJsonArray& aImageList, int aProjectImageIndex = 0){
        QJsonArray data;
        int idx = 0;
        for (auto i : aImageList){
            auto img = aImageAbstract.value(i.toString()).toObject();
            data.push_back(rea::Json("entry", rea::JArray(//getImageStringName(img),
                                                          ++idx,
                                                          getImageTime(img))));
        }
        auto ret = rea::Json("title", rea::JArray("no", "time"),
                             "entrycount", 30,
                             "pageindex", std::floor(aProjectImageIndex / 30),
                             "selects", aImageList.size() > 0 ? rea::JArray(aProjectImageIndex) : QJsonArray(),
                             "data", data);
        return ret;
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
                    m_current_task = dt.value("current_task").toString();
                    m_project_image_index = dt.value("project_image_index").toInt();
                    m_task_image_index = dt.value("task_image_index").toInt();
                    aInput->out<rea::stgJson>(rea::stgJson(QJsonObject(), "project/" + m_project_id + "/task.json"));
                    aInput->out<rea::stgJson>(rea::stgJson(QJsonObject(), "project/" + m_project_id + "/image.json"));
                    aInput->out<rea::stgJson>(rea::stgJson(QJsonObject(), "project/" + m_project_id + ".json"));
                    aInput->out<rea::stgJson>(rea::stgJson(QJsonObject(), "project/" + m_project_id + "/image_filter.json"));
                }
            }))
            ->next(rea::local(s3_bucket_name + "readJson", rea::Json("thread", 10)))
            ->next(rea::buffer<rea::stgJson>(4))
            ->next(rea::pipeline::add<std::vector<rea::stgJson>>([this](rea::stream<std::vector<rea::stgJson>>* aInput){
                auto dt = aInput->data();
                m_first_image_index = 0;
                m_transform = QJsonArray();
                m_tasks = dt[0].getData();
                m_images = dt[1].getData();
                dt[2].getData().swap(*this);
                m_image_filter = dt[3].getData();

                auto tsks = getTasks();
                auto imgs = getImageList();
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
                aInput->out<QJsonObject>(prepareImageListGUI(m_images, imgs, m_project_image_index), "project_image_updateListView");
                aInput->out<QJsonObject>(prepareLabelListGUI(lbls), "project_label_updateListView");
                //aInput->out<QJsonArray>(QJsonArray(), "project_image_listViewSelected", rea::Json("tag", "manual"));
                aInput->out<QJsonArray>(QJsonArray(), "project_task_listViewSelected", rea::Json("tag", "manual"));
                aInput->out<QJsonObject>(rea::Json("count", 1), "scatterprojectImageShow");
                aInput->out<QJsonArray>(QJsonArray(), "project_label_listViewSelected", rea::Json("tag", "project_manual"));
                aInput->out<QJsonObject>(getFilter(), "updateProjectImageFilterGUI");
                aInput->out<double>(0, "updateProjectChannelCountGUI");
                aInput->out<QJsonObject>(QJsonObject(), "switchprojectFirstImageIndex");
                aInput->out<QJsonArray>(QJsonArray({rea::GetMachineFingerPrint(), getProjectName(m_project_abstract)}), "title_updateNavigation", rea::Json("tag", "manual"));
                if (m_current_task != ""){
                    aInput->out<QJsonArray>(QJsonArray({rea::GetMachineFingerPrint(), getProjectName(m_project_abstract), getTaskName(m_tasks.value(m_current_task).toObject())}), "title_updateNavigation", rea::Json("tag", "manual"));
                    aInput->out<IProjectInfo>(IProjectInfo(&m_images, rea::Json("id", m_current_task,
                                                                                "labels", getLabels(),
                                                                                "channel", getChannelCount(),
                                                                                "project", m_project_id,
                                                                                "project_name", getProjectName(m_project_abstract),
                                                                                "task_name", getTaskName(m_tasks.value(m_current_task).toObject()),
                                                                                "imageshow", getImageShow(),
                                                                                "task_image_index", m_task_image_index,
                                                                                "type", getTaskType(m_tasks.value(m_current_task).toObject()))), openTask);
                    m_current_task = "";
                }
            }));
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
                           aInput->out<rea::stgJson>(rea::stgJson(*this, "project/" + m_project_id + ".json"), s3_bucket_name + "writeJson");
                           aInput->out<rea::stgJson>(rea::stgJson(m_tasks, "project/" + m_project_id + "/task.json"), s3_bucket_name + "writeJson");
                           aInput->out<QJsonArray>(QJsonArray(), "project_task_listViewSelected", rea::Json("tag", "manual"));
                       }
                   }), rea::Json("tag", "newTask"));

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
                   }), rea::Json("tag", "manual"));

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
                    aInput->out<rea::stgJson>(rea::stgJson(*this, "project/" + m_project_id + ".json"), s3_bucket_name + "writeJson");

                    if (dels.size() > 0){
                        for (auto i : dels){
                            m_tasks.remove(i);
                            aInput->out<QString>("project/" + m_project_id + "/task/" + i + ".json", s3_bucket_name + "deletePath");
                            aInput->out<QString>("project/" + m_project_id + "/task/" + i, s3_bucket_name + "deletePath");
                        }
                        aInput->out<rea::stgJson>(rea::stgJson(m_tasks, "project/" + m_project_id + "/task.json"), s3_bucket_name + "writeJson");
                    }

                    aInput->out<QJsonObject>(prepareTaskListGUI(getTasks()), "project_task_updateListView");
                    aInput->out<QJsonArray>(QJsonArray(), "project_task_listViewSelected", rea::Json("tag", "manual"));
                }
            }));

        //open task
        rea::pipeline::find("project_task_listViewSelected")
            ->next(rea::pipeline::add<QJsonArray>([this](rea::stream<QJsonArray>* aInput){
                       auto dt = aInput->data();
                       if (dt.size() > 0){
                           auto tsks = getTasks();
                           auto id = tsks[dt[0].toInt()].toString();
                           aInput->out<QJsonArray>(QJsonArray({rea::GetMachineFingerPrint(), getProjectName(m_project_abstract), getTaskName(m_tasks.value(id).toObject())}), "title_updateNavigation", rea::Json("tag", "manual"));
                           aInput->out<IProjectInfo>(IProjectInfo(&m_images, rea::Json("id", id,
                                                              "labels", getLabels(),
                                                              "channel", getChannelCount(),
                                                              "project", m_project_id,
                                                              "project_name", getProjectName(m_project_abstract),
                                                              "task_name", getTaskName(m_tasks.value(id).toObject()),
                                                              "imageshow", getImageShow(),
                                                              "type", getTaskType(m_tasks.value(id).toObject()))), openTask);
                       }
                   }), rea::Json("tag", openTask));
    }

    void labelManagement(){
        //label statistics
        serviceLabelStatistics("Project");

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
                           aInput->out<rea::stgJson>(rea::stgJson(*this, "project/" + m_project_id + ".json"), s3_bucket_name + "writeJson");
                           aInput->out<QJsonArray>(QJsonArray(), "project_label_listViewSelected", rea::Json("tag", "project_manual"));
                       }
                   }), rea::Json("tag", "newLabelGroup"));

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
                aInput->out<rea::stgJson>(rea::stgJson(*this, "project/" + m_project_id + ".json"), s3_bucket_name + "writeJson");
                aInput->out<QJsonObject>(rea::Json("key", key, "val", lbls), "updateProjectLabelGUI");
            }));

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
                        aInput->out<QJsonArray>(QJsonArray(), "project_label_listViewSelected", rea::Json("tag", "project_manual"));
                        aInput->out<rea::stgJson>(rea::stgJson(*this, "project/" + m_project_id + ".json"), s3_bucket_name + "writeJson");
                    }
                }
            }));

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
                aInput->out<rea::stgJson>(rea::stgJson(*this, "project/" + m_project_id + ".json"), s3_bucket_name + "writeJson");
                aInput->out<QJsonObject>(rea::Json("key", key, "val", lbls), "updateProjectLabelGUI");
            }));

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
                aInput->out<rea::stgJson>(rea::stgJson(*this, "project/" + m_project_id + ".json"), s3_bucket_name + "writeJson");
                aInput->out<QJsonObject>(rea::Json("key", key, "val", lbls), "updateProjectLabelGUI");
            }));

        //modifyShapeLabel
        rea::pipeline::add<QJsonObject>([this](rea::stream<QJsonObject>* aInput){
            auto dt = aInput->data();
            auto shps = dt.value("shapes").toObject();
            auto lbl = dt.value("label").toString();
            auto lbls = getShapeLabels(getLabels());
            for (auto i : shps.keys()){
                for (auto j = 0; j < m_show_count; ++j){
                    aInput->out<QJsonArray>(rea::JArray(
                                                rea::Json("obj", i,
                                                          "key", rea::JArray("caption"),
                                                          "val", lbl,
                                                          "cmd", true),
                                                rea::Json("obj", i,
                                                          "key", rea::JArray("color"),
                                                          "val", lbls.value(lbl).toObject().value("color"))
                                                    ), "updateQSGAttrs_projectimage_gridder" + QString::number(j));
                }
            }
        }, rea::Json("name", "updateProjectShapeLabel"));

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
            auto imgs = getImageList();
            for (auto i : sels){
                auto img = imgs[i.toInt()].toString();
                auto abs = m_images.value(img).toObject();
                auto lbls = getImageLabels(abs);
                lbls.insert(st.value("group").toString(), st.value("label"));
                setImageLabels(abs, lbls);
                m_images.insert(img, abs);
            }
            aInput->out<rea::stgJson>(rea::stgJson(m_images, "project/" + m_project_id + "/image.json"), s3_bucket_name + "writeJson");
        }), modifyImageLabel_tag);

        rea::pipeline::add<QJsonObject>([](rea::stream<QJsonObject>* aInput){
            aInput->out();
        }, rea::Json("name", "projectLabelChanged"));
    }
protected:
    QJsonObject getImageShow() override{
        return value("imageShow").toObject();
    }
private:
    const QJsonObject selectProjectImage = rea::Json("tag", "selectProjectImage");
    bool m_selecting = false;

    void setImageShow(const QJsonObject& aImageShow){
        insert("imageShow", aImageShow);
    }

    void imageManagement(){
        //switch first image index
        serviceSelectFirstImageIndex("project");

        //select image
        rea::pipeline::find("project_image_listViewSelected")
            ->next(rea::pipeline::add<QJsonArray, rea::pipeThrottle2>([this](rea::stream<QJsonArray>* aInput){
                if (m_selecting)
                    aInput->cache<bool>(true, 0);
                else
                    aInput->cache<bool>(false, 0);
            }), rea::Json("tag", "manual"))
            ->next(rea::pipeline::add<QJsonArray>([this](rea::stream<QJsonArray>* aInput){
                       auto dt = aInput->data();
                       if (dt.size() == 0){
                           aInput->out<QJsonArray>(QJsonArray(), "updateQSGCtrl_projectimage_gridder0");  //must be before "updateQSGModel_projectimage_gridder"
                           m_current_image = "";
                           for (int i = 0; i < m_show_count; ++i)
                               aInput->out<QJsonObject>(QJsonObject(), "updateQSGModel_projectimage_gridder" + QString::number(i));
                           aInput->out<QJsonObject>(QJsonObject(), "updateProjectImageGUI");
                       }
                       else{
                           auto idx = dt[0].toInt();
                           auto imgs = getImageList();
                           if (idx < imgs.size()){
                               auto nm = imgs[idx].toString();
                               if (nm != m_current_image){
                                   m_selecting = true;
                                   aInput->out<QJsonObject>(rea::Json("type", "project", "index", idx), "imageIndexChanged");
                                   aInput->out<QJsonArray>(QJsonArray(), "updateQSGCtrl_projectimage_gridder0");
                                   m_current_image = nm;
                                   auto nms = getImageName(m_images.value(m_current_image).toObject());
                                   aInput->out<rea::stgJson>(rea::stgJson(QJsonObject(), "project/" + m_project_id + "/image/" + nm + ".json"), s3_bucket_name + "readJson", selectProjectImage);
                                   aInput->cache<std::vector<stgCVMat>>(std::vector<stgCVMat>());
                                   for (auto i = 0; i < m_show_count; ++i){
                                       auto img_nm = nms[i].toString();
                                       if (i == 0)
                                           img_nm = nms[m_first_image_index].toString();
                                       auto img = "project/" + m_project_id + "/image/" + nm + "/" + img_nm;
                                       aInput->out<stgCVMat>(stgCVMat(img));
                                   }
                               }
                               //aInput->out<QJsonObject>(m_images.value(nm).toObject(), "updateProjectImageGUI");
                           }else{
                               aInput->out<QJsonObject>(QJsonObject(), "updateProjectImageGUI");
                           }
                       }
                   }))
            ->nextB(rea::pipeline::find(s3_bucket_name + "readJson")
                           ->nextB(rea::pipeline::add<rea::stgJson>([this](rea::stream<rea::stgJson>* aInput){
                                       m_image = aInput->data().getData();
                                       aInput->out<QJsonObject>(rea::Json(m_image, "image_label", getImageLabels(m_images.value(m_current_image).toObject()),
                                                                          "id", m_current_image), "updateProjectImageGUI");
                                      }),
                                selectProjectImage),
                    selectProjectImage)
            ->next(rea::local(s3_bucket_name + "readCVMat", rea::Json("thread", 10)))
            ->next(rea::pipeline::add<stgCVMat>([this](rea::stream<stgCVMat>* aInput){
                auto imgs = aInput->cacheData<std::vector<stgCVMat>>(0);
                imgs.push_back(stgCVMat(aInput->data().getData(), QString(aInput->data())));
                if (int(imgs.size()) == m_show_count){
                    aInput->out<std::vector<stgCVMat>>(imgs, "imageShowFilter", QJsonObject(), false)
                        ->cache<QJsonObject>(m_image)
                        ->cache<QJsonArray>(getImageFilter());
                }else
                    aInput->cache<std::vector<stgCVMat>>(imgs, 0);
            }))
            ->next("imageShowFilter", rea::Json("tag", "project"))
            ->nextF<std::vector<stgCVMat>>([this](rea::stream<std::vector<stgCVMat>>* aInput){
                auto imgs = aInput->data();

                auto page_count = int(imgs.size()) / m_show_count;
                m_show_page = std::min(page_count - 1, m_show_page);
                aInput->out<QJsonObject>(rea::Json("all", page_count, "index", m_show_page), "updateImageShowPages");

                for (auto i = 0; i < m_show_count; ++i){
                    auto dt = imgs[m_show_page * m_show_count + i];
                    auto pth = QString(dt);
                    auto img = cvMat2QImage(dt.getData());
                    rea::imagePool::cacheImage(pth, img);

                    auto img_show = getImageShow();
                    if (aInput->cacheData<QJsonArray>(1).size() > 0)
                        img_show.insert("disp_image_format", "none");

                    auto objs = rea::Json("img_" + m_current_image, rea::Json(
                                                                        "type", "imagebak",
                                                                        "range", rea::JArray(0, 0, img.width(), img.height()),
                                                                        "path", pth,
                                                                        "text", QJsonObject(),
                                                                        "transform", img_show));
                    if (m_show_label) {
                        auto new_annos = aInput->cacheData<QJsonArray>(2);
                        auto act_anno = new_annos.size() == 0 ? aInput->cacheData<QJsonObject>(0) : new_annos[m_show_page].toObject();
                        auto shps = getShapes(act_anno);
                        auto lbls = getShapeLabels(getLabels());
                        for (auto i : shps.keys()){
                            auto shp = shps.value(i).toObject();
                            if (shp.value("type") == "ellipse"){
                                shp = rea::Json("type", "ellipse",
                                                "caption", shp.value("label"),
                                                "color", lbls.value(shp.value("label").toString()).toObject().value("color"),
                                                "center", shp.value("center"),
                                                "radius", rea::JArray(shp.value("xradius"), shp.value("yradius")),
                                                "angle", shp.value("angle"),
                                                "text", shp.value("text"),
                                                "tag", shp.value("tag"));
                            }else if (shp.value("type") == "polyline"){
                                QJsonArray pts;
                                pts.push_back(shp.value("points"));
                                auto holes = shp.value("holes").toArray();
                                for (auto i : holes)
                                    pts.push_back(i);
                                shp = rea::Json("type", "poly",
                                                "caption", shp.value("label"),
                                                "color", lbls.value(shp.value("label").toString()).toObject().value("color"),
                                                "points", pts,
                                                "text", shp.value("text"),
                                                "tag", shp.value("tag"));
                            }else if (shp.value("type") == "image"){

                            }else
                                shp = QJsonObject();
                            if (!shp.empty()){
                                objs.insert(i, shp);
                            }
                        }
                    }

                    auto cfg = rea::Json("id", "project/" + m_project_id + "/image/" + m_current_image + ".json",
                                         "width", img.width(),
                                         "height", img.height(),
                                         "transform", m_transform,
                                         "face", 100,
                                         "text", rea::Json("visible", true,
                                                           "size", rea::JArray(80, 30)
                                                           //"location", "bottom"
                                                                                                                              ),
                                         "objects", objs);
                    auto ch = QString::number(i);
                    aInput->out<QJsonObject>(cfg, "updateQSGModel_projectimage_gridder" + ch);
                    serviceShowImageStatus("project", ch, img, pth);
                }
                m_selecting = false;
            }, rea::Json("tag", "project"));

        //get selected images
#define extractSelectedImages(aTitle) \
        next(rea::local("project_image_listViewSelected")) \
            ->nextF<QJsonArray>([this](rea::stream<QJsonArray>* aInput){ \
                auto dt = aInput->data(); \
                auto imgs = getImageList(); \
                aInput->out<QJsonObject>(rea::Json("title", aTitle, "sum", dt.size() * getChannelCount()), "updateProgress"); \
                for (auto i : dt){ \
                    auto img = (imgs.begin() + i.toInt())->toString(); \
                    aInput->out<rea::stgJson>(rea::stgJson(QJsonObject(), "project/" + m_project_id + "/image/" + img + ".json"), s3_bucket_name + "readJson", QJsonObject(), false)->cache<QString>(aInput->cacheData<QString>(0)); \
                } \
            }) \
            ->next(rea::local(s3_bucket_name + "readJson", rea::Json("thread", 10))) \
            ->nextF<rea::stgJson>([this](rea::stream<rea::stgJson>* aInput){ \
                auto dt = aInput->data(); \
                auto pth = QFileInfo(dt).baseName(); \
                auto imgs = dt.getData().value("source").toArray(); \
                auto anno = dt.getData(); \
                auto img_abs = m_images.value(pth).toObject(); \
                setImageLabels(anno, getImageLabels(img_abs)); \
                aInput->cache<std::vector<stgCVMat>>(std::vector<stgCVMat>())->cache<QJsonObject>(anno); \
                for (auto i : imgs) \
                    aInput->out<stgCVMat>(stgCVMat(cv::Mat(), "project/" + m_project_id + "/image/" + pth + "/" + i.toString())); \
            }) \
            ->next(rea::local(s3_bucket_name + "readCVMat", rea::Json("thread", 10))) \

        //export images
        rea::pipeline::find("_selectFile")
            ->nextF<QJsonArray>([](rea::stream<QJsonArray>* aInput){
                if (aInput->data().size() == 0)
                    return;
                aInput->cache<QString>(aInput->data()[0].toString())->out();
            }, rea::Json("tag", "exportLabelme"))
            ->extractSelectedImages("export labelme")
            ->nextF<stgCVMat>([this](rea::stream<stgCVMat>* aInput){
                auto dt = aInput->data();
                auto imgs = aInput->cacheData<std::vector<stgCVMat>>(1);
                imgs.push_back(dt);
                if (int(imgs.size()) == getChannelCount()){
                    aInput->out<std::vector<stgCVMat>>(imgs, "exportLabelme", QJsonObject(), false)
                        ->cache<QJsonObject>(aInput->cacheData<QJsonObject>(2))
                        ->cache<QString>(aInput->cacheData<QString>(0));
                }else
                    aInput->cache<std::vector<stgCVMat>>(imgs, 1);
            });

        //apply transform
        rea::pipeline::find("_newObject")
            ->next(rea::pipeline::add<QJsonObject>([](rea::stream<QJsonObject>* aInput){
               if (aInput->data().value("suffix").toString() == "")
                   aInput->out<QJsonObject>(rea::Json("title", "warning", "text", "Invalid suffix!"), "popMessage");
               aInput->cache<QString>(aInput->data().value("suffix").toString())->out<QJsonArray>(QJsonArray(), "project_image_listViewSelected");
                   }), rea::Json("tag", "applyTransform"))
            ->extractSelectedImages("apply transform")
            ->nextF<stgCVMat>([this](rea::stream<stgCVMat>* aInput){
                auto dt = aInput->data();
                auto imgs = aInput->cacheData<std::vector<stgCVMat>>(1);
                imgs.push_back(dt);
                if (int(imgs.size()) == getChannelCount()){
                    aInput->out<std::vector<stgCVMat>>(imgs, "applyTransform", QJsonObject(), false)
                        ->cache<QJsonObject>(aInput->cacheData<QJsonObject>(2))
                        ->cache<QJsonArray>(getImageFilter())
                        ->cache<QString>(aInput->cacheData<QString>(0));
                }else
                    aInput->cache<std::vector<stgCVMat>>(imgs, 1);
            })
            ->next("applyTransform")
            ->nextF<std::vector<stgCVMat>>([this](rea::stream<std::vector<stgCVMat>>* aInput){
                auto anno = aInput->cacheData<QJsonObject>(0);
                auto suffix = "_" + aInput->cacheData<QString>(2) + "_";
                auto annos = aInput->cacheData<QJsonArray>(3);
                auto dt = aInput->data();

                auto ch = getChannelCount();
                auto ex = int(dt.size() / ch - 1);
                if (ex)
                    aInput->out<QJsonObject>(rea::Json("step", ex * (- ch)), "updateProgress");
                for (auto i = 0; i < dt.size(); i += ch){
                    std::vector<stgCVMat> imgs;
                    for (auto j = 0; j < ch - 1; ++j)
                        imgs.push_back(dt[size_t(i + j)]);
                    auto idx = i / ch;
                    if (idx < annos.size())
                        anno = annos[idx].toObject();
                    auto srcs = anno.value("source").toArray();
                    for (int k = 0; k < srcs.size(); ++k){
                        auto src = srcs[k].toString();
                        src.insert(src.lastIndexOf("."), suffix + QString::number(idx));
                        srcs[k] = src;
                    }
                    anno.insert("source", srcs);
                    aInput->out<stgCVMat>(dt[i + ch - 1], "importImage", QJsonObject(), false)
                        ->cache<std::vector<stgCVMat>>(imgs)
                        ->cache<QJsonObject>(anno);
                }
            });

        // import project v3 data
        rea::pipeline::find("_selectFile")
            ->next(rea::pipeline::add<QJsonArray>([](rea::stream<QJsonArray>* aInput) {
                auto pths = aInput->data();
                if (pths.size() != 1)
                    return;
                auto pth = pths[0].toString();
                auto dir = QFileInfo(pth).path();
                dir = dir.mid(0, dir.lastIndexOf("/"));
                aInput->out<rea::stgJson>(rea::stgJson(QJsonObject(), pth), "readJson")->cache<QString>(dir);
            }), rea::Json("tag", "importProjectV3"))
            ->next(rea::local("readJson", rea::Json("thread", 10)))
            ->nextF<rea::stgJson>([this](rea::stream<rea::stgJson>* aInput){
                auto dir = aInput->cacheData<QString>(0);
                auto dt = aInput->data().getData();
                auto imgs = dt.value("image_abstract").toObject();
                auto ch = getChannelCount();
                if (imgs.size() > 0 && imgs.begin().value().toObject().value("captions").toArray().size() == ch){
                    aInput->out<QJsonObject>(rea::Json("title", "import image with labelme", "sum", imgs.size() * ch), "updateProgress");
                    for (auto i : imgs.keys())
                        aInput->out<rea::stgJson>(rea::stgJson(QJsonObject(), dir + "/imageInfo/" + i + ".json"), "readJson", QJsonObject(), false)
                            ->cache<QString>(dir)->cache<QString>(i)->cache<QJsonObject>(imgs.value(i).toObject().value("labels").toObject())
                            ->cache<QJsonObject>(QJsonObject())->cache<std::vector<stgCVMat>>(std::vector<stgCVMat>());
                }
            })
            ->next(rea::local("readJson", rea::Json("thread", 10)))
            ->nextF<rea::stgJson>([](rea::stream<rea::stgJson>* aInput){
                auto dir = aInput->cacheData<QString>(0);
                auto id = aInput->cacheData<QString>(1);
                auto img = aInput->data().getData();
                img.insert("image_label", aInput->cacheData<QJsonObject>(2));
                auto src = img.value("source").toArray();
                for (auto i : src){
                    aInput->out<stgCVMat>(stgCVMat(cv::Mat(), dir + "/image/" + id + "/" + i.toString()), "readCVMat")->cache<QJsonObject>(img, 3);
                }
                auto dt = aInput->data().getData();
            })
            ->next(rea::local("readCVMat", rea::Json("thread", 10)))
            ->nextF<stgCVMat>([this](rea::stream<stgCVMat>* aInput){
                auto imgs = aInput->cacheData<std::vector<stgCVMat>>(4);
                imgs.push_back(aInput->data());
                auto ch = getChannelCount();
                if (int(imgs.size()) == ch){
                    auto img = aInput->cacheData<QJsonObject>(3);
                    std::vector<cv::Mat> images;
                    QJsonArray channels;
                    QJsonArray sources;
                    QJsonArray locals;
                    QJsonObject shape_data;
                    for (int i = 0; i < ch; ++i){
                        auto path = imgs[i];
                        auto image = path.getData();

                        images.emplace_back(image);

                        auto fileinfo = QFileInfo(path);
                        auto basename = fileinfo.completeBaseName();
                        auto dirname = fileinfo.path();
                        auto suffix = fileinfo.suffix();

                        channels.append(image.channels());
                        sources.append(basename + "." + suffix);
                        locals.append(path);
                    }

                    std::vector<stgCVMat> ret_imgs;
                    for (int i = 0; i < ch - 1; ++i)
                        ret_imgs.push_back(stgCVMat(images[i], imgs[i]));  // push other

                    aInput->out<stgCVMat>(stgCVMat(images[ch - 1], imgs[ch - 1]), "importImage", QJsonObject(), false)
                        ->cache<std::vector<stgCVMat>>(ret_imgs)
                        ->cache<QJsonObject>(rea::Json("width", images[0].cols,
                                                       "height", images[0].rows,
                                                       "channel", channels,
                                                       "source", sources,
                                                       "local", locals,
                                                       "image_label", img.value("image_label"),
                                                       "shapes", parseProjectV3Shapes(img.value("shapes").toObject())));
                }else
                    aInput->cache<std::vector<stgCVMat>>(imgs, 4);
            });

           /* ->next(rea::local("readCVMat", rea::Json("thread", 10)))
            ->nextF<stgCVMat>([](rea::stream<stgCVMat>* aInput) {
                auto imgs = aInput->cacheData<std::vector<stgCVMat>>(0);
                imgs.emplace_back(aInput->data());
                aInput->cache<std::vector<stgCVMat>>(imgs, 0);

                if (int(imgs.size()) < channel_count)
                    return;

                std::vector<cv::Mat> images;
                QJsonArray channels;
                QJsonArray sources;
                QJsonArray locals;
                QJsonObject shape_data;

                for (int i = 0; i < channel_count; ++i) {
                    auto path = imgs[i];
                    auto image = path.getData();

                    if (image.empty()) {
                        LOG << "can not open image: " << imgs[channel_count - 1];
                        return;
                    }

                    images.emplace_back(image);

                    auto fileinfo = QFileInfo(path);
                    auto basename = fileinfo.completeBaseName();
                    auto dirname = fileinfo.path();
                    auto suffix = fileinfo.suffix();

                    auto labelme_path = dirname + "/" + basename + ".json";
                    if (QFileInfo::exists(labelme_path)) {
                        QJsonObject json_data;
                        auto ret = loadJson(labelme_path, json_data);
                        if (ret)
                            shape_data = parseLabelmeShapes(json_data.value("shapes").toArray());
                    }

                    channels.append(image.channels());
                    sources.append(basename + "." + suffix);
                    locals.append(path);
                }

                std::vector<stgCVMat> ret_imgs;
                for (int i = 0; i < channel_count - 1; ++i)
                    ret_imgs.push_back(stgCVMat(images[i], imgs[i]));  // push other

                auto img = images[0];
                aInput->out<stgCVMat>(stgCVMat(images[channel_count - 1], imgs[channel_count - 1]), "importImage", QJsonObject(), false)
                    ->cache<std::vector<stgCVMat>>(ret_imgs)
                    ->cache<QJsonObject>(rea::Json("width", img.cols,
                                                   "height", img.rows,
                                                   "channel", channels,
                                                   "source", sources,
                                                   "local", locals,
                                                   "shapes", shape_data));

                imgs.clear();
                aInput->cache<std::vector<stgCVMat>>(imgs, 0);
            }, rea::Json("thread", 8));*/

        //import image
        rea::pipeline::find("_selectFile")
            ->next(rea::pipeline::add<QJsonArray>([this](rea::stream<QJsonArray>* aInput){
                auto pths = aInput->data();
                QStringList img_pths, anno_pths;
                for (auto i : pths){
                    auto pth = i.toString();
                    if (pth.endsWith(".json"))
                        anno_pths.push_back(pth);
                    else
                        img_pths.push_back(pth);
                }

                auto ch_cnt = getChannelCount();
                auto remain = img_pths.size() % ch_cnt;
                while (remain--)
                    img_pths.pop_back();
                aInput->out<QJsonObject>(rea::Json("title", importImage, "sum", img_pths.size()), "updateProgress");
                aInput->cache<std::vector<stgCVMat>>(std::vector<stgCVMat>())->cache<QJsonObject>(QJsonObject());
                for (int i = 0; i < img_pths.size(); ++i){
                    if (i % ch_cnt == 0){
                        auto idx = int(std::floor(i / ch_cnt));
                        if (idx < anno_pths.size())
                            aInput->out<rea::stgJson>(rea::stgJson(QJsonObject(), anno_pths[idx]), "readJson");
                    }
                    aInput->out<stgCVMat>(stgCVMat(cv::Mat(), img_pths[i]), "readCVMat");
                }
            }), rea::Json("tag", importImage))
            ->nextB(rea::local("readJson", rea::Json("thread", 10))
                        ->nextB(rea::pipeline::add<rea::stgJson>([](rea::stream<rea::stgJson>* aInput){
                            aInput->cache<QJsonObject>(aInput->data().getData(), 1);
                        })))
            ->next(rea::local("readCVMat", rea::Json("thread", 10)))
            ->next(rea::pipeline::add<stgCVMat>([this](rea::stream<stgCVMat>* aInput){
                auto imgs_cache = aInput->cacheData<std::vector<stgCVMat>>(0);
                imgs_cache.push_back(aInput->data());
                int sz = int(imgs_cache.size());
                if (sz == getChannelCount()){
                    auto w = imgs_cache[0].getData().cols, h = imgs_cache[0].getData().rows;
                    for (auto i : imgs_cache){ //make sure the same width and height
                        if (i.getData().cols != w || i.getData().rows != h){
                            aInput->out<QJsonObject>(rea::Json("step", sz), "updateProgress");
                            imgs_cache.clear();
                            aInput->cache<std::vector<stgCVMat>>(imgs_cache, 0);
                            aInput->cache<QJsonObject>(QJsonObject(), 1);
                            return;
                        }
                    }

                    auto id = rea::generateUUID();

                    auto anno = aInput->cacheData<QJsonObject>(1);
                    if (anno.empty()){
                        QJsonArray nms;
                        QJsonArray localpth;
                        QJsonArray channels;
                        for (auto i : imgs_cache){
                            localpth.push_back(i);
                            channels.push_back(i.getData().channels());
                            auto nm = i.mid(i.lastIndexOf("/") + 1, i.length());
                            nms.push_back(nm);
                            aInput->out<stgCVMat>(stgCVMat(i.getData(), "project/" + m_project_id + "/image/" + id + "/" + nm));
                        }
                        auto tm = QDateTime::currentDateTime().toString(Qt::DateFormat::ISODate);
                        auto tms = tm.split("T");
                        m_images.insert(id, rea::Json("name", nms, "time", tms[0] + " " + tms[1]));  //update image.json
                        aInput->out<rea::stgJson>(rea::stgJson(rea::Json("width", w,  //update image_id.json
                                                               "height", h,
                                                               "channel", channels,
                                                               "source", nms,
                                                               "local", localpth),
                                                     "project/" + m_project_id + "/image/" + id + ".json"), s3_bucket_name + "writeJson");
                    }else{
                        auto nms = anno.value("source").toArray();
                        for (int i = 0; i < int(imgs_cache.size()); ++i)
                            aInput->out<stgCVMat>(stgCVMat(imgs_cache[i].getData(), "project/" + m_project_id + "/image/" + id + "/" + nms[i].toString()));
                        auto tm = QDateTime::currentDateTime().toString(Qt::DateFormat::ISODate);
                        auto tms = tm.split("T");

                        auto img_abs = rea::Json("name", nms, "time", tms[0] + " " + tms[1]);
                        auto img_lbls = getImageLabels(anno);
                        setImageLabels(img_abs, img_lbls);
                        m_images.insert(id, img_abs);  //update image.json
                        aInput->out<rea::stgJson>(rea::stgJson(anno, "project/" + m_project_id + "/image/" + id + ".json"), s3_bucket_name + "writeJson");

                        auto grps = getLabels();
                        auto lbls = getShapeLabels(grps);
                        auto shps = anno.value("shapes").toObject();
                        for (auto i : shps){
                            auto lbl = i.toObject().value("label").toString();
                            if (lbl != "" && !lbls.contains(lbl)){
                                lbls.insert(lbl, rea::Json("color", QColor::fromRgb(QRandomGenerator::global()->generate()).name()));
                                setShapeLabels(grps, lbls);
                                setLabels(grps);
                                aInput->out<QJsonObject>(prepareLabelListGUI(grps), "project_label_updateListView");
                                aInput->out<QJsonArray>(QJsonArray(), "project_label_listViewSelected", rea::Json("tag", "project_manual"));
                            }
                        }

                        for (auto i : img_lbls.keys()){
                            auto lbl = img_lbls.value(i).toString();
                            lbls = grps.value(i).toObject();
                            if (lbl != "" && !lbls.contains(lbl)){
                                lbls.insert(lbl, rea::Json("color", QColor::fromRgb(QRandomGenerator::global()->generate()).name()));
                                grps.insert(i, lbls);
                                setLabels(grps);
                                aInput->out<QJsonObject>(prepareLabelListGUI(grps), "project_label_updateListView");
                                aInput->out<QJsonArray>(QJsonArray(), "project_label_listViewSelected", rea::Json("tag", "project_manual"));
                            }
                        }
                    }

                    auto imgs = getImageList();
                    imgs.push_back(id);
                    setImageList(imgs);   //update project_id.json

                    imgs_cache.clear();
                }
                aInput->cache<std::vector<stgCVMat>>(imgs_cache, 0);
                aInput->cache<QJsonObject>(QJsonObject(), 1);
            }, rea::Json("name", "importImage")))
            ->next(rea::local(s3_bucket_name + "writeCVMat", rea::Json("thread", 11)))
            ->next(rea::pipeline::add<stgCVMat>([](rea::stream<stgCVMat>* aInput){
                aInput->out<QJsonObject>(QJsonObject());
            }))
            ->next(rea::local("updateProgress"))
            ->next(rea::pipeline::add<double>([this](rea::stream<double>* aInput){
                if (aInput->data() == 1.0){
                    aInput->out<rea::stgJson>(rea::stgJson(m_images, "project/" + m_project_id + "/image.json"), s3_bucket_name + "writeJson");
                    aInput->out<rea::stgJson>(rea::stgJson(*this, "project/" + m_project_id + ".json"), s3_bucket_name + "writeJson", rea::Json("tag", "updateProjectImages"));
                    auto imgs = getImageList();
                    aInput->out<QJsonObject>(prepareImageListGUI(m_images, imgs, imgs.size() - 1), "project_image_updateListView");
                    aInput->out<QJsonArray>(QJsonArray(), "project_image_listViewSelected", rea::Json("tag", "manual"), false);
                }
            }));

        //delete image
        rea::pipeline::find("_makeSure")
            ->next(rea::local("project_image_listViewSelected"), rea::Json("tag", "deleteImage"))
            ->next(rea::pipeline::add<QJsonArray>([this](rea::stream<QJsonArray>* aInput){
                auto dt = aInput->data();
                auto imgs = getImageList();

                std::vector<int> idxes;
                for (auto i : dt)
                    idxes.push_back(i.toInt());
                std::sort(idxes.begin(), idxes.end(), std::greater<int>());

                QJsonArray dels;
                for (auto i : idxes){
                    auto img = (imgs.begin() + i)->toString();
                    imgs.removeAt(i);
                    dels.push_back(img);
                    m_images.remove(img);
                }
                setImageList(imgs);
                aInput->out<rea::stgJson>(rea::stgJson(*this, "project/" + m_project_id + ".json"), s3_bucket_name + "writeJson");
                aInput->out<rea::stgJson>(rea::stgJson(m_images, "project/" + m_project_id + "/image.json"), s3_bucket_name + "writeJson");
                for (auto i : dels){
                    auto img = i.toString();
                    aInput->out<QString>("project/" + m_project_id + "/image/" + img, s3_bucket_name + "deletePath");
                    aInput->out<QString>("project/" + m_project_id + "/image/" + img + ".json", s3_bucket_name + "deletePath");
                }

                auto tsks = getTasks();
                aInput->cache<int>(tsks.size());
                aInput->cache<QJsonArray>(dels);
                if (tsks.size() == 0){
                    aInput->out<QJsonObject>(prepareImageListGUI(m_images, getImageList()), "project_image_updateListView");
                    aInput->out<QJsonArray>(QJsonArray(), "project_image_listViewSelected", rea::Json("tag", "manual"), false);
                }else
                    for (auto i : tsks)
                        aInput->out<rea::stgJson>(rea::stgJson(*this, "project/" + m_project_id + "/task/" + i.toString() + ".json"));
            }))
            ->next(rea::local(s3_bucket_name + "readJson", rea::Json("thread", 10)))
            ->next(rea::pipeline::add<rea::stgJson>([](rea::stream<rea::stgJson>* aInput){
                auto dt = aInput->data().getData();
                auto imgs = dt.value("images").toObject();
                auto dels = aInput->cacheData<QJsonArray>(1);
                for (auto i : dels)
                    imgs.remove(i.toString());
                dt.insert("images", imgs);
                aInput->setData(rea::stgJson(dt, QString(aInput->data())))->out();
            }))
            ->next(rea::local(s3_bucket_name + "writeJson", rea::Json("thread", 11)))
            ->next(rea::pipeline::add<rea::stgJson>([this](rea::stream<rea::stgJson>* aInput){
                auto cnt = aInput->cacheData<int>(0);
                cnt = cnt - 1;
                if (!cnt){
                    aInput->out<QJsonObject>(prepareImageListGUI(m_images, getImageList()), "project_image_updateListView");
                    aInput->out<QJsonArray>(QJsonArray(), "project_image_listViewSelected", rea::Json("tag", "manual"), false);
                }else
                    aInput->cache<int>(cnt, 0);
            }));

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
            }else if (dt.value("type") == "label"){
                imgs = dt.value("value").toArray();
            }else
                return;
            setImageList(imgs);
            setFilter(dt);
            aInput->out<QJsonObject>(prepareImageListGUI(m_images, imgs), "project_image_updateListView");
            aInput->out<QJsonArray>(QJsonArray(), "project_image_listViewSelected", rea::Json("tag", "manual"), false);
            aInput->out<rea::stgJson>(rea::stgJson(*this, "project/" + m_project_id + ".json"), s3_bucket_name + "writeJson");
        }, rea::Json("name", "filterProjectImages"));

        //modify task image//try update show
        rea::pipeline::find("QSGAttrUpdated_taskimage_gridder0")
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
                        aInput->out<QJsonArray>(mdys, "updateQSGAttrs_projectimage_gridder" + QString::number(i));
            }));

        //modify image
        rea::pipeline::find("QSGAttrUpdated_projectimage_gridder0")
            ->next(rea::pipeline::add<QJsonArray>([this](rea::stream<QJsonArray>* aInput){
                QString cur = "project/" + m_project_id + "/image/" + m_current_image + ".json";
                QString pth = cur;
                if (modifyImage(aInput->data(), m_image, pth)){
                    if (m_current_image != "")
                        aInput->out<rea::stgJson>(rea::stgJson(m_image, pth), s3_bucket_name + "writeJson");
                }else if (pth != cur){
                    aInput->cache<QJsonArray>(aInput->data())->out<rea::stgJson>(rea::stgJson(QJsonObject(), pth));
                }
            }))
            ->next(rea::local(s3_bucket_name + "readJson", rea::Json("thread", 10)))
            ->next(rea::pipeline::add<rea::stgJson>([this](rea::stream<rea::stgJson>* aInput){
                auto dt = aInput->data().getData();
                auto pth = QString(aInput->data());
                modifyImage(aInput->cacheData<QJsonArray>(0), dt, pth);
                aInput->out<rea::stgJson>(rea::stgJson(dt, pth), s3_bucket_name + "writeJson");
            }));

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
    int m_show_page = 0;
    int m_show_count = 1;
    bool m_show_label = true;

    const QString setImageShow0 = "setImageShow";
    void guiManagement(){
        // set image filter
        rea::pipeline::find("setImageFilter")
            ->nextF<QJsonArray>([this](rea::stream<QJsonArray>* aInput){
                m_image_filter.insert("operators", aInput->data());
                aInput->out<rea::stgJson>(rea::stgJson(m_image_filter, "project/" + m_project_id + "/" + "image_filter.json"), s3_bucket_name + "writeJson");
            }, QJsonObject(), rea::Json("name", "saveImageFilter"));

        // update image
        rea::pipeline::add<QJsonObject>([this](rea::stream<QJsonObject>* aInput){
            //if (m_last_state == 1) //avoid free parent qsgnode crash
            //    return;
            auto dt = aInput->data();
            m_current_image = "";
            if (dt.contains("page"))
                m_show_page = dt.value("page").toInt();
            aInput->out<QJsonArray>(QJsonArray(), "project_image_listViewSelected", rea::Json("tag", "manual"), false);
            if (m_selects_cache.contains("shapes"))
                aInput->out<QJsonObject>(rea::Json(m_selects_cache, "invisible", true), "updateQSGSelects_projectimage_gridder0");
        }, rea::Json("name", "refreshProjectImage"));

        //modify image show
        rea::pipeline::add<double>([this](rea::stream<double>* aInput){
            m_show_label = !m_show_label;
            m_current_image = "";
            aInput->out<QJsonArray>(QJsonArray(), "project_image_listViewSelected", rea::Json("tag", "manual"), false);
        }, rea::Json("name", "modifyProjectLabelShow"));

        //update channel count
        rea::pipeline::add<double>([this](rea::stream<double>* aInput){
            aInput->setData(getChannelCount())->out();
        }, rea::Json("name", "updateProjectChannelCountGUI"));

        //the qsgnode will be destroyed by qt after switch page
        rea::pipeline::find("title_updateNavigation")
            ->next(rea::pipeline::add<QJsonArray>([this](rea::stream<QJsonArray>* aInput){
                if (aInput->data().size() == 2){
                    if (m_last_state == 3)
                        aInput->out<QJsonArray>(QJsonArray(), "project_image_listViewSelected", rea::Json("tag", "manual"), false);
                }else{
                    if (aInput->data().size() == 1)
                        m_project_id = "";
                    m_current_image = "";
                }
                m_last_state = aInput->data().size();
            }), rea::Json("tag", "manual"));

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
            aInput->out<QJsonArray>(QJsonArray(), "project_image_listViewSelected", rea::Json("tag", "manual"), false);
        }, rea::Json("name", "scatterprojectImageShow"));

        //modify image show
        rea::pipeline::find(setImageShow0)
        ->next(rea::pipeline::add<QJsonObject>([this](rea::stream<QJsonObject>* aInput){
            auto show_cfg = aInput->data();
            auto show = getImageShow();
            QJsonObject show_content;
            for (auto i : show_cfg.keys()){
                auto cnds = show_cfg.value(i).toArray();
                auto val = show.value(i).toString();
                int sel = 0;
                for (int j = 0; j < cnds.size(); ++j)
                    if (val == cnds[j].toString()){
                        sel = j;
                        break;
                    }
                show_content.insert(i, rea::Json("model", cnds, "index", sel));
            }
            aInput->out<QJsonObject>(rea::Json("title", "image show",
                                               "content", show_content,
                                               "tag", rea::Json("tag", setImageShow0)), "_newObject");
        }))
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
                aInput->out<rea::stgJson>(rea::stgJson(*this, "project/" + m_project_id + ".json"), s3_bucket_name + "writeJson");
                if (m_current_image != ""){
                    for (int i = 0; i < m_show_count; ++i)
                        aInput->out<QJsonObject>(rea::Json("obj", "img_" + m_current_image, "key", rea::JArray("transform"), "val", show),
                                                 "updateQSGAttr_projectimage_gridder" + QString::number(i));
                }
            }
        }), rea::Json("tag", setImageShow0));
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
}, rea::Json("name", "install2_project"), "regQML");
