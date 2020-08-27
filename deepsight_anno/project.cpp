#include "reactive2.h"

class project : public QJsonObject{
public:
    project(){
        rea::pipeline::find("title_updateStatus")
            ->next(rea::pipeline::add<QJsonArray>([](rea::stream<QJsonArray>* aInput){
                if (aInput->data().size() == 2)
                    aInput->out<QJsonObject>(rea::Json("title", rea::JArray("name"),
                                                       "selects", rea::JArray("0"),
                                                       "data", rea::JArray(
                                                                   rea::Json("entry", rea::JArray("task1")),
                                                                   rea::Json("entry", rea::JArray("task2")),
                                                                   rea::Json("entry", rea::JArray("task3")))),
                                             "project_task_updateListView");
            }))
            ->next("project_task_updateListView");

        rea::pipeline::find("project_task_listViewSelected")
        ->next(rea::pipeline::add<QJsonArray>([](rea::stream<QJsonArray>* aInput){
                   auto dt = aInput->data();
                   aInput->out<double>(2, "switchPage");
                   aInput->out<QJsonArray>(QJsonArray({"User1", "Project1", "Task1"}), "title_updateStatus");
               }), rea::Json("tag", "openTask"))
        ->nextB(0, "switchPage", QJsonObject())
        ->nextB(0, "title_updateStatus", QJsonObject());
    }
};

static rea::regPip<QQmlApplicationEngine*> init_proj([](rea::stream<QQmlApplicationEngine*>* aInput){
    static project cur_proj;
    aInput->out();
}, QJsonObject(), "regQML");
