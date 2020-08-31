#include "../storage/storage.h"
#include "util.h"
#include "model.h"
#include "reactive2.h"
#include <QDateTime>

class user : public model{
protected:
   // ABSTRACT(Project)
private:
    QJsonArray getProjects(){
        return value("projects").toArray();
    }
    void setProjects(const QJsonArray& aProjects){
        insert("projects", aProjects);
    }
    QString getProjectName(const QJsonObject& aProject){
        return aProject.value("name").toString();
    }
    QString getProjectOwner(const QJsonObject& aProject){
        return aProject.value("owner").toString();
    }
    QJsonObject prepareProjectListGUI(const QJsonArray& aProjects){
        QJsonArray data;
        for (auto i : aProjects)
            data.push_back(rea::Json("entry", rea::JArray(getProjectName(m_projects.value(i.toString()).toObject()))));
        return rea::Json("title", rea::JArray("name"),
                         "selects", aProjects.size() > 0 ? rea::JArray("0") : QJsonArray(),
                         "data", data);
    }
    void insertProject(QJsonObject& aProject){
        auto mdls = getProjects();
        auto tm = QDateTime::currentDateTime().toString(Qt::DateFormat::ISODate);
        auto tms = tm.split("T");
        auto id = rea::generateUUID();
        mdls.push_back(id);
        m_projects.insert(id, rea::Json(aProject, "time", tms[0] + " " + tms[1], "owner", rea::GetMachineFingerPrint()));
        setProjects(mdls);
    }
    QJsonObject m_projects;
public:
    user(){
        rea::pipeline::add<double>([this](rea::stream<double>* aInput){  //open user
            aInput->out<QJsonArray>(QJsonArray({rea::GetMachineFingerPrint()}), "title_updateStatus");
            auto projs = getProjects();
            aInput->out<QJsonObject>(prepareProjectListGUI(projs), "user_updateListView");
            if (projs.size() > 0)
                aInput->out<QJsonObject>(m_projects.value(projs.begin()->toString()).toObject(), "updateProjectGUI");
        }, rea::Json("name", "openUser"))
        ->nextB(0, "updateProjectGUI", QJsonObject())
        ->nextB(0, "title_updateStatus", QJsonObject())
        ->next("user_updateListView");

        rea::pipeline::find("user_listViewSelected")  //select project
        ->next(rea::pipeline::add<QJsonArray>([this](rea::stream<QJsonArray>* aInput){
           auto dt = aInput->data();
           if (dt.size() == 0)
               aInput->out<QJsonObject>(QJsonObject(), "updateProjectGUI");
           else{
               auto idx = dt[0].toInt();
               auto projs = getProjects();
               if (idx < projs.size()){
                   auto nm = projs[idx].toString();
                   aInput->out<QJsonObject>(m_projects.value(nm).toObject(), "updateProjectGUI");
               }
               else
                   aInput->out<QJsonObject>(QJsonObject(), "updateProjectGUI");
           }
        }), rea::Json("tag", "manual"))
        ->next("updateProjectGUI");

        auto deleteProject = rea::Json("tag", "deleteProject");
        rea::pipeline::find("_makeSure")  //delete project
        ->next("user_listViewSelected", deleteProject)
        ->next(rea::pipeline::add<QJsonArray>([this](rea::stream<QJsonArray>* aInput){
           auto dt = aInput->data();

           if (dt.size() > 0){
               std::vector<int> idxes;
               for (auto i : dt)
                   idxes.push_back(i.toInt());
               std::sort(idxes.begin(), idxes.end(), std::greater<int>());

               auto owner = rea::GetMachineFingerPrint();
               QStringList dels;
               auto projs = getProjects();
               for (auto i : idxes){
                   auto nm = projs[i].toString();
                   if (getProjectOwner(m_projects.value(nm).toObject()) == owner){
                       dels.push_back(nm);
                   }
                   projs.erase(projs.begin() + i);
               }
               setProjects(projs);
               aInput->out<stgJson>(stgJson(*this, "user/" + rea::GetMachineFingerPrint() + ".json"), "deepsightwriteJson");

               if (dels.size() > 0){
                   for (auto i : dels){
                       m_projects.remove(i);
                       aInput->out<QString>("project/" + i + ".json", "deepsightdeletePath");
                       aInput->out<QString>("project/" + i, "deepsightdeletePath");
                   }
                   aInput->out<stgJson>(stgJson(m_projects, "project.json"), "deepsightwriteJson");
               }

               aInput->out<QJsonObject>(prepareProjectListGUI(getProjects()), "user_updateListView");
               aInput->out<QJsonArray>(QJsonArray(), "user_listViewSelected");
           }
        }), deleteProject)
        ->nextB(0, "user_updateListView", QJsonObject())
        ->nextB(0, "user_listViewSelected", rea::Json("tag", "manual"))
        ->nextB(0, "deepsightdeletePath", QJsonObject())
        ->next("deepsightwriteJson");

/*        rea::pipeline::find("user_listViewSelected")  //open project
        ->next(rea::pipeline::add<QJsonArray>([this](rea::stream<QJsonArray>* aInput){
            auto dt = aInput->data();
            aInput->out<QJsonArray>(QJsonArray({"User1", "Project1"}), "title_updateStatus");
            if (dt.size() > 0){
                auto projs = getProjects();
                aInput->out<QString>(projs.keys()[dt[0].toInt()], "openProject");
            }
        }), rea::Json("tag", "openProject"))
        ->nextB(0, "title_updateStatus", QJsonObject())
        ->next("openProject");
*/

        rea::pipeline::find("_newObject")  //new project
        ->next(rea::pipeline::add<QJsonObject>([this](rea::stream<QJsonObject>* aInput){
            auto proj = aInput->data();
            auto nm = proj.value("name").toString();
            if (nm == "")
                aInput->out<QJsonObject>(rea::Json("title", "warning", "text", "Invalid name!"), "popMessage");
            else{
                insertProject(proj);
                auto projs = getProjects();
                aInput->out<QJsonObject>(prepareProjectListGUI(projs), "user_updateListView");
                aInput->out<stgJson>(stgJson(*this, "user/" + rea::GetMachineFingerPrint() + ".json"), "deepsightwriteJson");
                aInput->out<stgJson>(stgJson(m_projects, "project.json"), "deepsightwriteJson");
                if (projs.size() == 1)
                    aInput->out<QJsonArray>(QJsonArray(), "user_listViewSelected");
            }
        }), rea::Json("tag", "newProject"))
        ->nextB(0, "popMessage", QJsonObject())
        ->nextB(0, "user_updateListView", QJsonObject())
        ->nextB(0, "user_listViewSelected", rea::Json("tag", "manual"))
        ->next("deepsightwriteJson");

        const QString loaduser = "loadUser";
        rea::pipeline::add<double>([](rea::stream<double>* aInput){ //load user
            aInput->out<stgJson>(stgJson(QJsonObject(), "project.json"));
            aInput->out<stgJson>(stgJson(QJsonObject(), "user/" + rea::GetMachineFingerPrint() + ".json"));
        }, rea::Json("name", loaduser))
        ->next(rea::local("deepsightreadJson"), rea::Json("tag", loaduser))
        ->next(rea::buffer<stgJson>(2))
        ->next(rea::pipeline::add<std::vector<stgJson>>([this](rea::stream<std::vector<stgJson>>* aInput){
            auto dt = aInput->data();
            m_projects = dt[0].getData();
            replaceModel(dt[1].getData());
            aInput->out<double>(0, "openUser");
        }))
        ->next("openUser");
    }
private:
    /*QJsonObject getProjects(){
        return value("project_abstract").toObject();
    }
    void setProjects(const QJsonObject& aProjects){
        insert("project_abstract", aProjects);
    }
    void insertProject(QJsonObject& aProject){
        auto projs = getProjects();
        auto tm0 = QDateTime::currentDateTime();
        auto tm = tm0.toString(Qt::DateFormat::ISODate);
        auto tms = tm.split("T");
        projs.insert(QString::number(tm0.toTime_t()) + "_" + rea::generateUUID(), rea::Json(aProject, "time", tms[0] + " " + tms[1]));
        setProjects(projs);
    }
    QJsonObject prepareProjectListGUI(const QJsonObject& aProjects){
        QJsonArray data;
        for (auto i : aProjects)
            data.push_back(rea::Json("entry", rea::JArray(i.toObject().value("name"))));
        return rea::Json("title", rea::JArray("name"),
                         "selects", aProjects.size() > 0 ? rea::JArray("0") : QJsonArray(),
                         "data", data);
    }*/
};

static rea::regPip<QQmlApplicationEngine*> init_user([](rea::stream<QQmlApplicationEngine*>* aInput){
    static fsStorage local_storage("deepsight");
    static user cur_user;
    aInput->out();
}, QJsonObject(), "regQML");
