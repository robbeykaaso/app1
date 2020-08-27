#include "../storage/storage.h"
#include "reactive2.h"

class user : public QJsonObject{
public:
    user(){
        rea::pipeline::add<double>([](rea::stream<double>* aInput){
            aInput->out<double>(0, "switchPage");
            aInput->out<QJsonArray>(QJsonArray({"User1"}), "title_updateStatus");
            aInput->out<QJsonObject>(rea::Json("title", rea::JArray("name"),
                                               "selects", rea::JArray("0"),
                                               "data", rea::JArray(
                                                           rea::Json("entry", rea::JArray("proj1")),
                                                           rea::Json("entry", rea::JArray("proj2")),
                                                           rea::Json("entry", rea::JArray("proj3")))),
                                     "user_updateListView");
        }, rea::Json("name", "initSoftware"))
        ->nextB(0, "switchPage", QJsonObject())
        ->nextB(0, "title_updateStatus", QJsonObject())
        ->next("user_updateListView");

        rea::pipeline::find("user_listViewSelected")
        ->next(rea::pipeline::add<QJsonArray>([](rea::stream<QJsonArray>* aInput){
            auto dt = aInput->data();
            aInput->out<double>(1, "switchPage");
            aInput->out<QJsonArray>(QJsonArray({"User1", "Project1"}), "title_updateStatus");
        }), rea::Json("tag", "openProject"))
        ->nextB(0, "switchPage", QJsonObject())
        ->nextB(0, "title_updateStatus", QJsonObject());
    }
};

static rea::regPip<QQmlApplicationEngine*> init_user([](rea::stream<QQmlApplicationEngine*>* aInput){
    static fsStorage local_storage;
    static user cur_user;
    aInput->out();
}, QJsonObject(), "regQML");
