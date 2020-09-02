#include "reactive2.h"
#include "../storage/storage.h"
#include "model.h"
#include <QRandomGenerator>

class project : public model{
protected:
   // ABSTRACT(Task)
private:
    const QString openTask = "openTask";
private:
    QJsonObject m_project_abstract;
    QString m_project_id;
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
    QJsonObject getLabels(){
        return value("labels").toObject();
    }
    void setLabels(const QJsonObject& aLabels){
        insert("labels", aLabels);
    }
    QString getTaskName(const QJsonObject& aTask){
        return aTask.value("name").toString();
    }
    QString getImageName(const QJsonObject& aImage){
        auto nms = aImage.value("name").toArray();
        QString ret = "";
        for (auto i : nms)
            ret += i.toString() + "\n";
        return ret;
    }
    QJsonObject prepareImageListGUI(const QJsonArray& aImages){
        QJsonArray data;
        for (auto i : aImages)
            data.push_back(rea::Json("entry", rea::JArray(getImageName(m_images.value(i.toString()).toObject()))));
        return rea::Json("title", rea::JArray("name"),
                         "selects", aImages.size() > 0 ? rea::JArray("0") : QJsonArray(),
                         "data", data);
    }
    QJsonObject prepareTaskListGUI(const QJsonArray& aTasks){
        QJsonArray data;
        for (auto i : aTasks)
            data.push_back(rea::Json("entry", rea::JArray(getTaskName(m_tasks.value(i.toString()).toObject()))));
        return rea::Json("title", rea::JArray("name"),
                         "selects", aTasks.size() > 0 ? rea::JArray("0") : QJsonArray(),
                         "data", data);
    }
    QJsonObject prepareLabelListGUI(const QJsonObject& aLabels){
        QJsonArray data;
        for (auto i : aLabels.keys())
            data.push_back(rea::Json("entry", rea::JArray(i)));
        return rea::Json("title", rea::JArray("group"),
                         "selects", aLabels.size() > 0 ? rea::JArray("0") : QJsonArray(),
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
public:
    project(){
        //open project
        rea::pipeline::add<QJsonObject>([this](rea::stream<QJsonObject>* aInput){
            auto dt = aInput->data();
            if (dt.value("id") != m_project_id){
                m_project_id = dt.value("id").toString();
                m_project_abstract = dt.value("abstract").toObject();
                aInput->out<stgJson>(stgJson(QJsonObject(), "project/" + m_project_id + "/task.json"));
                aInput->out<stgJson>(stgJson(QJsonObject(), "project/" + m_project_id + "/image.json"));
                aInput->out<stgJson>(stgJson(QJsonObject(), "project/" + m_project_id + ".json"));
            }
        }, rea::Json("name", "openProject"))
        ->next(rea::local("deepsightreadJson"))
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

            aInput->out<QJsonObject>(prepareTaskListGUI(tsks), "project_task_updateListView");
            aInput->out<QJsonObject>(prepareImageListGUI(imgs), "project_image_updateListView");
            aInput->out<QJsonObject>(prepareLabelListGUI(lbls), "project_label_updateListView");
            aInput->out<QJsonArray>(QJsonArray(), "project_task_listViewSelected");
            aInput->out<QJsonArray>(QJsonArray(), "project_image_listViewSelected");
            aInput->out<QJsonArray>(QJsonArray(), "project_label_listViewSelected");
        }))
        ->nextB(0, "updateTaskGUI", QJsonObject())
        ->nextB(0, "project_task_updateListView", QJsonObject())
        ->nextB(0, "project_image_updateListView", QJsonObject())
        ->nextB(0, "project_label_updateListView", QJsonObject())
        ->nextB(0, "project_task_listViewSelected", rea::Json("tag", "manual"))
        ->nextB(0, "project_image_listViewSelected", rea::Json("tag", "manual"))
        ->next("project_label_listViewSelected", rea::Json("tag", "manual"));

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
        ->nextB(0, "popMessage", QJsonObject())
        ->nextB(0, "project_task_updateListView", QJsonObject())
        ->nextB(0, "project_task_listViewSelected", rea::Json("tag", "manual"))
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
        ->nextB(0, "project_task_updateListView", QJsonObject())
        ->nextB(0, "project_task_listViewSelected", rea::Json("tag", "manual"))
        ->nextB(0, "deepsightdeletePath", QJsonObject())
        ->next("deepsightwriteJson");

        //open task
        rea::pipeline::find("project_task_listViewSelected")
        ->next(rea::pipeline::add<QJsonArray>([this](rea::stream<QJsonArray>* aInput){
            auto dt = aInput->data();
            if (dt.size() > 0){
                auto tsks = getTasks();
                auto id = tsks[dt[0].toInt()].toString();
                aInput->out<QJsonArray>(QJsonArray({rea::GetMachineFingerPrint(), getProjectName(m_project_abstract), getTaskName(m_tasks.value(id).toObject())}), "title_updateStatus");
                aInput->out<QString>(id, openTask);
            }
        }), rea::Json("tag", openTask))
        ->nextB(0, "title_updateStatus", QJsonObject())
        ->next(openTask);

        //select label group
        rea::pipeline::find("project_label_listViewSelected")
        ->next(rea::pipeline::add<QJsonArray>([this](rea::stream<QJsonArray>* aInput){
           auto dt = aInput->data();
           if (dt.size() == 0)
               aInput->out<QJsonObject>(QJsonObject(), "updateLabelGUI");
           else{
               auto idx = dt[0].toInt();
               auto lbls = getLabels();
               if (idx < lbls.size()){
                   aInput->out<QJsonObject>(rea::Json("key", (lbls.begin() + idx).key(), "val", (lbls.begin() + idx)->toObject()), "updateLabelGUI");
               }
               else
                   aInput->out<QJsonObject>(QJsonObject(), "updateLabelGUI");
           }
        }), rea::Json("tag", "manual"))
        ->next("updateLabelGUI");

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
        ->nextB(0, "popMessage", QJsonObject())
        ->nextB(0, "project_label_updateListView", QJsonObject())
        ->nextB(0, "project_label_listViewSelected", rea::Json("tag", "manual"))
        ->next("deepsightwriteJson");

        //new label
        auto newLabel = rea::buffer<QJsonArray>(2);
        auto newLabel_tag = rea::Json("tag", "newLabel");
        rea::pipeline::find("_newObject")
        ->next(rea::pipeline::add<QJsonObject>([](rea::stream<QJsonObject>* aInput){
           aInput->out<QJsonArray>(rea::JArray(aInput->data().value("label")));
           aInput->out<QJsonArray>(QJsonArray(), "project_label_listViewSelected");
        }), newLabel_tag)
        ->nextB(0, "project_label_listViewSelected", newLabel_tag, newLabel, newLabel_tag)
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
            aInput->out<QJsonObject>(rea::Json("key", key, "val", lbls), "updateLabelGUI");
        }))
        ->nextB(0, "popMessage", QJsonObject())
        ->nextB(0, "updateLabelGUI", QJsonObject())
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
        ->nextB(0, "project_label_updateListView", QJsonObject())
        ->nextB(0, "project_label_listViewSelected", rea::Json("tag", "manual"))
        ->next("deepsightwriteJson");

        //delete label
        auto deleteLabel = rea::buffer<QJsonArray>(2);
        auto deleteLabel_tag = "deleteLabel";
        rea::pipeline::add<QString>([](rea::stream<QString>* aInput){
            aInput->out<QJsonArray>(rea::JArray(aInput->data()));
            aInput->out<QJsonArray>(QJsonArray(), "project_label_listViewSelected");
        }, rea::Json("name", deleteLabel_tag))
        ->nextB(0, "project_label_listViewSelected", rea::Json("tag", deleteLabel_tag), deleteLabel, rea::Json("tag", deleteLabel_tag))
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
            aInput->out<QJsonObject>(rea::Json("key", key, "val", lbls), "updateLabelGUI");
        }))
        ->nextB(0, "updateLabelGUI", QJsonObject())
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
            aInput->out<QJsonObject>(rea::Json("key", key, "val", lbls), "updateLabelGUI");
        }))
        ->nextB(0, "updateLabelGUI", QJsonObject())
        ->next("deepsightwriteJson");

        //select image
        rea::pipeline::find("project_image_listViewSelected")
        ->next(rea::pipeline::add<QJsonArray>([this](rea::stream<QJsonArray>* aInput){
           auto dt = aInput->data();
           if (dt.size() == 0)
               aInput->out<QJsonObject>(QJsonObject(), "updateImageGUI");
           else{
               auto idx = dt[0].toInt();
               auto imgs = getImages();
               if (idx < imgs.size()){
                   auto nm = imgs[idx].toString();
                   aInput->out<QJsonObject>(m_tasks.value(nm).toObject(), "updateImageGUI");
               }
               else
                   aInput->out<QJsonObject>(QJsonObject(), "updateImageGUI");
           }
        }), rea::Json("tag", "manual"))
        ->next("updateImageGUI");

        const QString importImage = "importImage";
        rea::pipeline::find("_selectFile")
        ->next(rea::pipeline::add<QJsonArray>([this](rea::stream<QJsonArray>* aInput){
            auto pths = aInput->data();
            auto ch = getChannelCount();
            for (int i = 0; i < pths.size(); i += ch){

            }
        }), rea::Json("tag", "importImage"));
    }
};

static rea::regPip<QQmlApplicationEngine*> init_proj([](rea::stream<QQmlApplicationEngine*>* aInput){
    static project cur_proj;
    aInput->out();
}, QJsonObject(), "regQML");
