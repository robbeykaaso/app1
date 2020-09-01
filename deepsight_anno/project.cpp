#include "reactive2.h"
#include "../storage/storage.h"
#include "model.h"

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
        rea::pipeline::add<QJsonObject>([this](rea::stream<QJsonObject>* aInput){  //open project
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
            replaceModel(dt[2].getData());

            auto tsks = getTasks();
            auto lbls = getLabels();
            aInput->out<QJsonObject>(prepareTaskListGUI(tsks), "project_task_updateListView");
            aInput->out<QJsonObject>(prepareLabelListGUI(lbls), "project_label_updateListView");
            aInput->out<QJsonArray>(QJsonArray(), "project_task_listViewSelected");
            aInput->out<QJsonArray>(QJsonArray(), "project_label_listViewSelected");
        }))
        ->nextB(0, "updateTaskGUI", QJsonObject())
        ->nextB(0, "project_label_updateListView", QJsonObject())
        ->nextB(0, "project_task_updateListView", QJsonObject())
        ->nextB(0, "project_task_listViewSelected", rea::Json("tag", "manual"))
        ->next("project_label_listViewSelected", rea::Json("tag", "manual"));

        rea::pipeline::find("_newObject")  //new task
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
               if (tsks.size() == 1)
                   aInput->out<QJsonArray>(QJsonArray(), "project_task_listViewSelected");
           }
        }), rea::Json("tag", "newTask"))
        ->nextB(0, "popMessage", QJsonObject())
        ->nextB(0, "project_task_updateListView", QJsonObject())
        ->nextB(0, "project_task_listViewSelected", rea::Json("tag", "manual"))
        ->next("deepsightwriteJson");

        rea::pipeline::find("project_task_listViewSelected")  //select task
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

        rea::pipeline::find("_makeSure")  //delete task
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

        rea::pipeline::find("project_task_listViewSelected")  //open task
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

        rea::pipeline::find("project_label_listViewSelected")  //select label group
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
        //new label
        //delete label
    }
};

static rea::regPip<QQmlApplicationEngine*> init_proj([](rea::stream<QQmlApplicationEngine*>* aInput){
    static project cur_proj;
    aInput->out();
}, QJsonObject(), "regQML");
