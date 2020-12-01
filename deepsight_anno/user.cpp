#include "../util/py.h"
#include "model.h"
#include "protocal.h"
#include "reactive2.h"
#include "storage.h"
#include "util.h"
#include <QDateTime>

class user : public model {
private:
    QJsonArray getProjects(const QJsonObject& aUserConfig) { return aUserConfig.value("projects").toArray(); }
  void setProjects(QJsonObject& aUserConfig, const QJsonArray &aProjects) {
      aUserConfig.insert("projects", aProjects);
  }
  void setProjectPublic(QJsonObject& aProject){
      aProject.insert("public", !aProject.value("public").toBool());
  }
  bool getProjectPublic(const QJsonObject& aProject){
      return aProject.value("public").toBool();
  }
  QString getProjectOwner(const QJsonObject& aProject) {
    return aProject.value("owner").toString();
  }
  bool setCurrentTask(const QString& aTask){
      auto ret = value("current_task") != aTask;
      if (ret)
          insert("current_task", aTask);
      return ret;
  }
  bool setCurrentProject(const QString& aProject){
      auto ret = value("current_project") != aProject;
      if (ret)
          insert("current_project", aProject);
      return ret;
  }
  QString getStoragePath(){
      return "user/" + rea::GetMachineFingerPrint() + ".json";
  }
  QString getCurrentProject(){
      return value("current_project").toString();
  }
  QString getCurrentTask(){
      return value("current_task").toString();
  }
  bool setPageIndex(const QString& aType, int aIndex){
      auto key = aType + "_page_index";
      auto ret = value(key) != aIndex;
      if (ret)
          insert(key, aIndex);
      return ret;
  }
  int getPageIndex(const QString& aType){
      auto key = aType + "_page_index";
      return value(key).toInt();
  }
  QJsonObject getDrawFreeParam(){
      return value("draw_free").toObject();
  }
  int getDrawFreeRadius(const QJsonObject& aParam){
      return aParam.value("radius").toInt(10);
  }
  void setDrawFreeRadius(QJsonObject& aParam, int aRadius){
      aParam.insert("radius", aRadius);
  }
  void setDrawFreeParam(const QJsonObject& aParam){
      insert("draw_free", aParam);
  }
  QJsonObject prepareProjectListGUI(const QJsonArray &aProjects) {
    QJsonArray data;
    for (auto i : aProjects) {
      auto proj = m_projects.value(i.toString()).toObject();
      data.push_back(rea::Json(
          "entry",
          rea::JArray(getProjectName(proj),
                      getProjectOwner(proj) == rea::GetMachineFingerPrint())));
    }
    return rea::Json("title", rea::JArray("name", "owner"), "selects",
                     aProjects.size() > 0 ? rea::JArray(0) : QJsonArray(),
                     "data", data);
  }
  QJsonArray addProject(const QString &aID) {
    auto mdls = getProjects(*this);
    mdls.push_back(aID);
    setProjects(*this, mdls);
    return mdls;
  }  
  QString insertProject(QJsonObject &aProject) {
    auto tm = QDateTime::currentDateTime().toString(Qt::DateFormat::ISODate);
    auto tms = tm.split("T");
    auto id = rea::generateUUID();
    m_projects.insert(id, rea::Json(aProject, "time", tms[0] + " " + tms[1],
                                    "owner", rea::GetMachineFingerPrint()));
    return id;
  }
  QJsonObject m_projects;
  bool m_project_owner;
  int m_title_state = 0;

public:
    user() {        
      // initialize storage
      rea::pipeline::add<rea::stgJson>([this](rea::stream<rea::stgJson> *aInput) {
          auto dt = aInput->data();
          if (dt.getData().contains("root_dir"))
              s3_bucket_name = dt.getData().value("root_dir").toString();
#ifdef USEAWS
          if (dt.getData().value("local_fs").toBool())
              static fsStorage2 file_storage(s3_bucket_name,
                                             [this]() { return m_project_owner; });
          else
              static awsStorage2 minio_storage(s3_bucket_name,
                                               dt.getData().value("aws_fs").toObject(),
                                               [this]() { return m_project_owner; });
#else
          static fsStorage2 file_storage(s3_bucket_name,
                                         [this]() { return m_project_owner; });
#endif
      })
          ->previous(rea::local("readJson"))
          ->execute(std::make_shared<rea::stream<rea::stgJson>>((rea::stgJson("config_.json"))));

    // select project
    rea::pipeline::find("user_listViewSelected")
        ->nextF<QJsonArray>([this](rea::stream<QJsonArray> *aInput) {
            auto dt = aInput->data();
            if (dt.size() == 0)
                aInput->out<QJsonObject>(QJsonObject(), "updateProjectGUI");
            else {
                auto idx = dt[0].toInt();
                auto projs = getProjects(*this);
                if (idx < projs.size()) {
                  auto nm = projs[idx].toString();
                  aInput->out<QJsonObject>(m_projects.value(nm).toObject(), "updateProjectGUI");
                } else
                  aInput->out<QJsonObject>(QJsonObject(), "updateProjectGUI");
            }
          },  rea::Json("tag", "manual"));

    // delete project
    rea::pipeline::find("_makeSure")
        ->next(rea::local("user_listViewSelected"), rea::Json("tag", "deleteProject"))
        ->nextF<QJsonArray>([this](rea::stream<QJsonArray> *aInput) {
              auto dt = aInput->data();

              if (dt.size() > 0) {
                std::vector<int> idxes;
                for (auto i : dt)
                  idxes.push_back(i.toInt());
                std::sort(idxes.begin(), idxes.end(), std::greater<int>());

                auto owner = rea::GetMachineFingerPrint();
                QStringList dels;
                auto projs = getProjects(*this);
                for (auto i : idxes) {
                  auto nm = projs[i].toString();
                  if (getProjectOwner(m_projects.value(nm).toObject()) == owner) {
                    dels.push_back(nm);
                  }
                  projs.erase(projs.begin() + i);
                }
                setProjects(*this, projs);
                aInput->out<rea::stgJson>(rea::stgJson(*this, getStoragePath()), s3_bucket_name + "writeJson");

                if (dels.size() > 0) {
                  for (auto i : dels) {
                    m_projects.remove(i);
                    aInput->out<QString>("project/" + i + ".json", s3_bucket_name + "deletePath");
                    aInput->out<QString>("project/" + i, s3_bucket_name + "deletePath");
                  }
                  aInput->out<rea::stgJson>(rea::stgJson(m_projects, "project.json"), s3_bucket_name + "writeJson");
                }

                aInput->out<QJsonObject>(prepareProjectListGUI(getProjects(*this)), "user_updateListView");
                aInput->out<QJsonArray>(QJsonArray(), "user_listViewSelected", rea::Json("tag", "manual"));
              }
            });

    // share project
    rea::pipeline::find("user_listViewSelected")
        ->nextF<QJsonArray>([this](rea::stream<QJsonArray>* aInput){
            auto dt = aInput->data();
            auto projs = getProjects(*this);
            auto owner = rea::GetMachineFingerPrint();
            bool sv = false;
            for (auto i : dt){
                auto id = projs[i.toInt()].toString();
                if (getProjectOwner(m_projects.value(id).toObject()) == owner){
                    auto proj = m_projects.value(id).toObject();
                    setProjectPublic(proj);
                    m_projects.insert(id, proj);
                    sv = true;
                }
            }
            if (sv){
                aInput->out<rea::stgJson>(rea::stgJson(m_projects, "project.json"), s3_bucket_name + "writeJson");
                aInput->out<QJsonArray>(QJsonArray(), "user_listViewSelected", rea::Json("tag", "manual"));
            }
        }, rea::Json("tag", "shareProject"));

    // new project, import project
    rea::pipeline::add<QJsonObject>([this](rea::stream<QJsonObject>* aInput){
        QJsonArray model, value;
        for (auto i : m_projects.keys()){
            auto proj = m_projects.value(i).toObject();
            if (getProjectPublic(proj)){
                model.push_back(getProjectName(proj));
                value.push_back(i);
            }
        }
        if (model.size() > 0)
            aInput->out<QJsonObject>(rea::Json("title", "import project",
                                               "content", rea::Json("id", rea::Json("model", model,
                                                                                    "value", value,
                                                                                    "index", 0)),
                                               "tag", rea::Json("tag", "newProject")),
                                     "_newObject");
        else
            aInput->out<QJsonObject>(rea::Json("title", "import project",
                                               "content", rea::Json("id", ""),
                                               "tag", rea::Json("tag", "newProject")),
                                     "_newObject");
    }, rea::Json("name", "importProject"))
    ->next("_newObject")
    ->nextF<QJsonObject>([this](rea::stream<QJsonObject> *aInput) {
        auto proj = aInput->data();

        auto projs = getProjects(*this);
        if (proj.contains("id")) {
            auto id = proj.value("id").toString();
            if (!m_projects.contains(id)) {
              aInput->out<QJsonObject>(rea::Json("title", "warning", "text", "Invalid id!"), "popMessage");
              return;
            } else {
              for (auto i : projs)
                if (i == id) {
                  aInput->out<QJsonObject>(rea::Json("title", "warning", "text", "Existed project!"), "popMessage");
                  return;
                }
              projs = addProject(id);
            }
        } else {
            auto nm = proj.value("name").toString();
            if (nm == "") {
                aInput->out<QJsonObject>(rea::Json("title", "warning", "text", "Invalid name!"), "popMessage");
                return;
            } else if (proj.value("channel").toInt() <= 0) {
                aInput->out<QJsonObject>(rea::Json("title", "warning", "text", "Invalid channel count!"), "popMessage");
                return;
            } else
                projs = addProject(insertProject(proj));
        }
        aInput->out<QJsonObject>(prepareProjectListGUI(projs), "user_updateListView");
        aInput->out<rea::stgJson>(rea::stgJson(*this, getStoragePath()), s3_bucket_name + "writeJson");
        aInput->out<rea::stgJson>(rea::stgJson(m_projects, "project.json"), s3_bucket_name + "writeJson");
        aInput->out<QJsonArray>(QJsonArray(), "user_listViewSelected", rea::Json("tag", "manual"));
    },  rea::Json("tag", "newProject"));

    // open project
    const QString openProject = "openProject";
    rea::pipeline::find("user_listViewSelected")
        ->nextF<QJsonArray>([this, openProject](rea::stream<QJsonArray> *aInput) {
            auto dt = aInput->data();
            if (dt.size() > 0) {
                auto projs = getProjects(*this);
                auto id = projs[dt[0].toInt()].toString();
                m_project_owner = getProjectOwner(m_projects.value(id).toObject()) == rea::GetMachineFingerPrint();
                aInput->out<QJsonObject>(rea::Json("id", id, "abstract", m_projects.value(id)), openProject);
                if (setCurrentProject(id))
                    aInput->out<rea::stgJson>(rea::stgJson(*this, getStoragePath()), s3_bucket_name + "writeJson");
            }
        }, rea::Json("tag", openProject));

    // load user
    rea::pipeline::add<double>([this](rea::stream<double> *aInput) {
        aInput->outB<rea::stgJson>(rea::stgJson("project.json"))
            ->out<rea::stgJson>(rea::stgJson(getStoragePath()));
    }, rea::Json("name", "loadUser"))
        ->next(rea::local(s3_bucket_name + "readJson", rea::Json("thread", 10)))
        ->next(rea::buffer<rea::stgJson>(2))
        ->nextF<std::vector<rea::stgJson>>([this, openProject](rea::stream<std::vector<rea::stgJson>> *aInput) {
            auto dt = aInput->data();
            m_projects = dt[0].getData();
            dt[1].getData().swap(*this);

            aInput->outB<QJsonArray>(QJsonArray({rea::GetMachineFingerPrint()}), "title_updateNavigation", rea::Json("tag", "manual"))
                ->outB<QJsonObject>(prepareProjectListGUI(getProjects(*this)), "user_updateListView")
                ->outB<QJsonArray>(QJsonArray(), "user_listViewSelected", rea::Json("tag", "manual"));

            auto cur_proj = getCurrentProject();
            auto cur_task = getCurrentTask();
            if (cur_proj != ""){
                m_project_owner = getProjectOwner(m_projects.value(cur_proj).toObject()) == rea::GetMachineFingerPrint();
                aInput->out<QJsonObject>(rea::Json("id", cur_proj,
                                                   "abstract", m_projects.value(cur_proj),
                                                   "current_task", cur_task), openProject);
            }
            aInput->out<QJsonObject>(rea::Json("project", cur_proj == "" ? 0 : getPageIndex("project"),
                                               "task", cur_task == "" ? 0 : getPageIndex("task")), "recoverPageIndex");
        });

    // record current task
    rea::pipeline::find("openTask")
        ->nextF<IProjectInfo>([this](rea::stream<IProjectInfo>* aInput){
            if (setCurrentTask(aInput->data().value("id").toString()))
                aInput->out<rea::stgJson>(rea::stgJson(*this, getStoragePath()), s3_bucket_name + "writeJson");
    });

    // record current project and task
    rea::pipeline::find("title_updateNavigation")
        ->nextF<QJsonArray>([this](rea::stream<QJsonArray>* aInput){
            auto dt = aInput->data();
            if ((dt.size() == 1)){
                if (m_title_state > 1){
                    bool sv = setCurrentProject("");
                    if (setCurrentTask("") || sv)
                        aInput->out<rea::stgJson>(rea::stgJson(*this, getStoragePath()), s3_bucket_name + "writeJson");
                }
            }else if (dt.size() == 2){
                if (setCurrentTask(""))
                    aInput->out<rea::stgJson>(rea::stgJson(*this, getStoragePath()), s3_bucket_name + "writeJson");
            }
            m_title_state = dt.size();
        }, rea::Json("tag", "manual"));

    //record page index
    rea::pipeline::add<QJsonObject>([this](rea::stream<QJsonObject>* aInput){
        auto dt = aInput->data();
        if (setPageIndex(dt.value("type").toString(), dt.value("index").toInt()))
            aInput->out<rea::stgJson>(rea::stgJson(*this, getStoragePath()), s3_bucket_name + "writeJson");
    }, rea::Json("name", "pageIndexChanged"));

    //recover page index
    rea::pipeline::add<QJsonObject>([](rea::stream<QJsonObject>* aInput){
        aInput->out();
    }, rea::Json("name", "recoverPageIndex"));

    //get mode param
    rea::pipeline::add<QJsonObject>([this](rea::stream<QJsonObject>* aInput){
        auto dt = aInput->data();
        auto cmd = dt.value("cmd").toString();
        auto prms = dt.value("prm").toArray();
        if (prms.size() > 0){
            auto prm = prms[0].toObject();
            if (prm.value("type") == "drawfree" || prm.value("type") == "drawmask")
                prms[0] = rea::Json(getDrawFreeParam(), "type", prm.value("type"));
        }
        aInput->out<QJsonArray>(prms, cmd);
    }, rea::Json("name", "beforeUpdateQSGCtrl"));

    //set drawfree param
    rea::pipeline::add<QJsonObject>([this](rea::stream<QJsonObject>* aInput){
        auto prm = getDrawFreeParam();
        aInput->setData(rea::Json("title", "draw free", "content", rea::Json("radius", getDrawFreeRadius(prm)), "tag", rea::Json("tag", "setDrawFreeParam")))->out();
    }, rea::Json("name", "setDrawFreeParam"))
        ->next("_newObject")
        ->nextF<QJsonObject>([this](rea::stream<QJsonObject>* aInput){
            auto prm = getDrawFreeParam();
            auto dt = aInput->data();
            setDrawFreeRadius(prm, dt.value("radius").toInt());
            setDrawFreeParam(prm);
            aInput->out<rea::stgJson>(rea::stgJson(*this, getStoragePath()), s3_bucket_name + "writeJson");
            aInput->out<QJsonObject>(dt, "afterSetDrawFreeParam");
        }, rea::Json("tag", "setDrawFreeParam"));
  }
};

static rea::regPip<QQmlApplicationEngine *> init_user([](rea::stream<QQmlApplicationEngine *> *aInput) {
    static fsStorage local_storage;
    static user cur_user;
    aInput->out();
}, rea::Json("name", "install1_user"), "regQML");
