#include "reactive2.h"
#include "../storage/storage.h"
#include "model.h"

class project : public model{
protected:
   // ABSTRACT(Task)
public:
    project(){
/*        rea::pipeline::add<QString>([this](rea::stream<QString>* aInput){  //open project
            if (aInput->data() != getID()){
                setID(aInput->data());
                aInput->out<stgJson>(stgJson(QJsonObject(), "projectInfo/" + aInput->data() + ".json"));
            }
        }, rea::Json("name", "openProject"))
        ->next(rea::local("deepsightreadJson"), rea::Json("tag", "loadProject"))
        ->next(rea::pipeline::add<stgJson>([this](rea::stream<stgJson>* aInput){
            replaceModel(aInput->data().getData());
            auto tsks = getTasks();
            aInput->out<QJsonObject>(prepareTaskListGUI(tsks), "project_task_updateListView");
            if (tsks.size() > 0)
                aInput->out<QJsonObject>(tsks.begin().value().toObject(), "updateTaskGUI");
            else
                aInput->out<QJsonObject>(QJsonObject(), "updateTaskGUI");
        }))
        ->nextB(0, "updateTaskGUI", QJsonObject())
        ->next("project_task_updateListView");

        rea::pipeline::find("project_task_listViewSelected")  //open task
        ->next(rea::pipeline::add<QJsonArray>([](rea::stream<QJsonArray>* aInput){
            auto dt = aInput->data();
            aInput->out<QJsonArray>(QJsonArray({"User1", "Project1", "Task1"}), "title_updateStatus");
        }), rea::Json("tag", "openTask"))
        ->nextB(0, "title_updateStatus", QJsonObject());

        rea::pipeline::find("project_task_listViewSelected")  //select task
        ->next(rea::pipeline::add<QJsonArray>([this](rea::stream<QJsonArray>* aInput){
            auto dt = aInput->data();
            if (dt.size() == 0)
                aInput->out<QJsonObject>(QJsonObject(), "updateTaskGUI");
            else{
                auto idx = dt[0].toInt();
                auto tsks = getTasks();
                if (idx < tsks.size())
                    aInput->out<QJsonObject>((tsks.begin() + idx)->toObject(), "updateTaskGUI");
               else
                   aInput->out<QJsonObject>(QJsonObject(), "updateTaskGUI");
           }
       }), rea::Json("tag", "manual"))
        ->next("updateTaskGUI");

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
                aInput->out<stgJson>(stgJson(*this, "projectInfo/" + getID() + ".json"), "deepsightwriteJson");
                if (tsks.size() == 1)
                    aInput->out<QJsonArray>(QJsonArray(), "project_task_listViewSelected");
            }
        }), rea::Json("tag", "newTask"))
        ->nextB(0, "popMessage", QJsonObject())
        ->nextB(0, "project_task_updateListView", QJsonObject())
        ->nextB(0, "project_task_listViewSelected", rea::Json("tag", "manual"))
        ->next("deepsightwriteJson");
        */
    }
private:
    void setID(const QString& aID){
        insert("id", aID);
    }
    QString getID(){
        return value("id").toString();
    }
};

static rea::regPip<QQmlApplicationEngine*> init_proj([](rea::stream<QQmlApplicationEngine*>* aInput){
    static project cur_proj;
    aInput->out();
}, QJsonObject(), "regQML");
