//#include "../storage/storage.h"
#include "protocal.h"
#include "../storage/awsStorage.h"
#include "../util/py.h"
#include "util.h"
#include "model.h"
#include "reactive2.h"
#include <QDateTime>

class customAWSStorage : public awsStorage{
public:
    customAWSStorage(const QString& aType = "") : awsStorage(aType) {
        //tell server aws info
        rea::pipeline::find("clientBoardcast")
        ->next(rea::pipeline::add<QJsonObject>([this](rea::stream<QJsonObject>* aInput){
            if (aInput->data().value("value") == "socket is connected"){
                aInput->out<QJsonObject>(rea::Json(protocal.value(protocal_connect).toObject().value("req").toObject(),
                                                   "s3_ip_port", QString::fromStdString(m_aws.getIPPort()),
                                                   "s3_access_key", QString::fromStdString(m_aws.getAccessKey()),
                                                   "s3_secret_key", QString::fromStdString(m_aws.getSecretKey())),
                                         "callServer");
            }
        }))
        ->next("callServer")
        ->next(rea::pipeline::add<QJsonObject>([](rea::stream<QJsonObject>* aInput){
            assert(aInput->data().value("type") == "connect");
        }), rea::Json("tag", protocal_connect));
    }
};

class user : public model{
protected:
   // ABSTRACT(Project)
private:
    const QString openProject = "openProject";
    const QString loaduser = "loadUser";
    const QString openUser = "openUser";
private:
    QJsonArray getProjects(){
        return value("projects").toArray();
    }
    void setProjects(const QJsonArray& aProjects){
        insert("projects", aProjects);
    }
    QString getProjectOwner(const QJsonObject& aProject){
        return aProject.value("owner").toString();
    }
    QJsonObject prepareProjectListGUI(const QJsonArray& aProjects){
        QJsonArray data;
        for (auto i : aProjects){
            auto proj = m_projects.value(i.toString()).toObject();
            data.push_back(rea::Json("entry", rea::JArray(getProjectName(proj), getProjectOwner(proj) == rea::GetMachineFingerPrint())));
        }
        return rea::Json("title", rea::JArray("name", "owner"),
                         "selects", aProjects.size() > 0 ? rea::JArray(0) : QJsonArray(),
                         "data", data);
    }
    QJsonArray addProject(const QString& aID){
        auto mdls = getProjects();
        mdls.push_back(aID);
        setProjects(mdls);
        return mdls;
    }
    QString insertProject(QJsonObject& aProject){
        auto tm = QDateTime::currentDateTime().toString(Qt::DateFormat::ISODate);
        auto tms = tm.split("T");
        auto id = rea::generateUUID();
        m_projects.insert(id, rea::Json(aProject, "time", tms[0] + " " + tms[1], "owner", rea::GetMachineFingerPrint()));
        return id;
    }
    QJsonObject m_projects;
public:
    user(){
        //open user
        rea::pipeline::add<double>([this](rea::stream<double>* aInput){
            aInput->out<QJsonArray>(QJsonArray({rea::GetMachineFingerPrint()}), "title_updateNavigation");
            auto projs = getProjects();
            aInput->out<QJsonObject>(prepareProjectListGUI(projs), "user_updateListView");
            aInput->out<QJsonArray>(QJsonArray(), "user_listViewSelected");
        }, rea::Json("name", openUser))
        ->nextB("user_listViewSelected", rea::Json("tag", "manual"))
        ->nextB("title_updateNavigation", rea::Json("tag", "manual"))
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

        //delete project
        rea::pipeline::find("_makeSure")
        ->next(rea::local("user_listViewSelected"), rea::Json("tag", "deleteProject"))
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
               aInput->out<rea::stgJson>(rea::stgJson(*this, "user/" + rea::GetMachineFingerPrint() + ".json"), s3_bucket_name + "writeJson");

               if (dels.size() > 0){
                   for (auto i : dels){
                       m_projects.remove(i);
                       aInput->out<QString>("project/" + i + ".json", s3_bucket_name + "deletePath");
                       aInput->out<QString>("project/" + i, s3_bucket_name + "deletePath");
                   }
                   aInput->out<rea::stgJson>(rea::stgJson(m_projects, "project.json"), s3_bucket_name + "writeJson");
               }

               aInput->out<QJsonObject>(prepareProjectListGUI(getProjects()), "user_updateListView");
               aInput->out<QJsonArray>(QJsonArray(), "user_listViewSelected");
           }
        }))
        ->nextB("user_updateListView")
        ->nextB("user_listViewSelected", rea::Json("tag", "manual"))
        ->nextB(s3_bucket_name + "deletePath")
        ->next(s3_bucket_name + "writeJson");

        //open project
        rea::pipeline::find("user_listViewSelected")
        ->next(rea::pipeline::add<QJsonArray>([this](rea::stream<QJsonArray>* aInput){
            auto dt = aInput->data();
            if (dt.size() > 0){
                auto projs = getProjects();
                auto id = projs[dt[0].toInt()].toString();
                aInput->out<QJsonArray>(QJsonArray({rea::GetMachineFingerPrint(), getProjectName(m_projects.value(id).toObject())}), "title_updateNavigation");
                aInput->out<QJsonObject>(rea::Json("id", id, "abstract", m_projects.value(id)), openProject);
            }
        }), rea::Json("tag", openProject))
        ->nextB("title_updateNavigation", rea::Json("tag", "manual"))
        ->next(openProject);

        //new project, import project
        rea::pipeline::find("_newObject")
        ->next(rea::pipeline::add<QJsonObject>([this](rea::stream<QJsonObject>* aInput){
            auto proj = aInput->data();

            auto projs = getProjects();
            if (proj.contains("id")){
                auto id = proj.value("id").toString();
                if (!m_projects.contains(id)){
                    aInput->out<QJsonObject>(rea::Json("title", "warning", "text", "Invalid id!"), "popMessage");
                    return;
                }else{
                    for (auto i : projs)
                        if (i == id){
                            aInput->out<QJsonObject>(rea::Json("title", "warning", "text", "Existed project!"), "popMessage");
                            return;
                        }
                    projs = addProject(id);
                }
            }else{
                auto nm = proj.value("name").toString();
                if (nm == ""){
                    aInput->out<QJsonObject>(rea::Json("title", "warning", "text", "Invalid name!"), "popMessage");
                    return;
                }else if (proj.value("channel").toInt() <= 0){
                    aInput->out<QJsonObject>(rea::Json("title", "warning", "text", "Invalid channel count!"), "popMessage");
                    return;
                }else
                    projs = addProject(insertProject(proj));
            }
            aInput->out<QJsonObject>(prepareProjectListGUI(projs), "user_updateListView");
            aInput->out<rea::stgJson>(rea::stgJson(*this, "user/" + rea::GetMachineFingerPrint() + ".json"), s3_bucket_name + "writeJson");
            aInput->out<rea::stgJson>(rea::stgJson(m_projects, "project.json"), s3_bucket_name + "writeJson");
            aInput->out<QJsonArray>(QJsonArray(), "user_listViewSelected");
        }), rea::Json("tag", "newProject"))
        ->nextB("popMessage")
        ->nextB("user_updateListView")
        ->nextB("user_listViewSelected", rea::Json("tag", "manual"))
        ->next(s3_bucket_name + "writeJson");

        //load user
        rea::pipeline::add<double>([](rea::stream<double>* aInput){
            aInput->out<rea::stgJson>(rea::stgJson(QJsonObject(), "project.json"));
            aInput->out<rea::stgJson>(rea::stgJson(QJsonObject(), "user/" + rea::GetMachineFingerPrint() + ".json"));
        }, rea::Json("name", loaduser))
        ->next(rea::local(s3_bucket_name + "readJson"), rea::Json("tag", loaduser))
        ->next(rea::buffer<rea::stgJson>(2))
        ->next(rea::pipeline::add<std::vector<rea::stgJson>>([this](rea::stream<std::vector<rea::stgJson>>* aInput){
            auto dt = aInput->data();
            m_projects = dt[0].getData();
            dt[1].getData().swap(*this);
            aInput->out<double>(0, openUser);
        }))
        ->next(openUser);

        //initialize storage
        rea::pipeline::add<rea::stgJson>([this](rea::stream<rea::stgJson>* aInput){
            auto dt = aInput->data();
            if (dt.getData().value("local_fs").toBool())
                static fsStorage file_storage(s3_bucket_name);
                //static fsStorage file_storage("Z:/deepsight");
            else
                static customAWSStorage minio_storage(s3_bucket_name);
        })->previous(rea::local("readJson"))
            ->execute(std::make_shared<rea::stream<rea::stgJson>>((rea::stgJson(QJsonObject(), "config_.json"))));
    }
};

static rea::regPip<QQmlApplicationEngine*> init_user([](rea::stream<QQmlApplicationEngine*>* aInput){
    static fsStorage local_storage;
    static user cur_user;
    aInput->out();
}, rea::Json("name", "install1_user"), "regQML");
