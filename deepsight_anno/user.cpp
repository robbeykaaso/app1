#include "../util/py.h"
#include "model.h"
#include "protocal.h"
#include "reactive2.h"
#include "storage.h"
#include "util.h"
#include <QDateTime>

class user : public model {
protected:
  // ABSTRACT(Project)
private:
  const QString openProject = "openProject";
  const QString loaduser = "loadUser";

private:
  QJsonArray getProjects() { return value("projects").toArray(); }
  void setProjects(const QJsonArray &aProjects) {
    insert("projects", aProjects);
  }
  QString getProjectOwner(const QJsonObject &aProject) {
    return aProject.value("owner").toString();
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
    auto mdls = getProjects();
    mdls.push_back(aID);
    setProjects(mdls);
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

public:
  user() {
      // initialize storage
      rea::pipeline::add<rea::stgJson>([this](rea::stream<rea::stgJson> *aInput) {
          auto dt = aInput->data();
          if (dt.getData().contains("fs_root"))
              s3_bucket_name = dt.getData().value("fs_root").toString();
          auto tmp = s3_bucket_name;
          if (dt.getData().value("local_fs").toBool())
              static fsStorage2 file_storage(s3_bucket_name,
                                             [this]() { return m_project_owner; });
          // static fsStorage file_storage("Z:/deepsight");
          else
              static awsStorage2 minio_storage(s3_bucket_name,
                                               dt.getData().value("aws_fs").toObject(),
                                               [this]() { return m_project_owner; });
      })
          ->previous(rea::local("readJson"))
          ->execute(std::make_shared<rea::stream<rea::stgJson>>(
              (rea::stgJson(QJsonObject(), "config_.json"))));

    // select project
    rea::pipeline::find("user_listViewSelected")
        ->next(
            rea::pipeline::add<QJsonArray>(
                [this](rea::stream<QJsonArray> *aInput) {
                  auto dt = aInput->data();
                  if (dt.size() == 0)
                    aInput->out<QJsonObject>(QJsonObject(), "updateProjectGUI");
                  else {
                    auto idx = dt[0].toInt();
                    auto projs = getProjects();
                    if (idx < projs.size()) {
                      auto nm = projs[idx].toString();
                      aInput->out<QJsonObject>(m_projects.value(nm).toObject(),
                                               "updateProjectGUI");
                    } else
                      aInput->out<QJsonObject>(QJsonObject(),
                                               "updateProjectGUI");
                  }
                }),
            rea::Json("tag", "manual"));

    // delete project
    rea::pipeline::find("_makeSure")
        ->next(rea::local("user_listViewSelected"),
               rea::Json("tag", "deleteProject"))
        ->next(rea::pipeline::add<QJsonArray>(
            [this](rea::stream<QJsonArray> *aInput) {
              auto dt = aInput->data();

              if (dt.size() > 0) {
                std::vector<int> idxes;
                for (auto i : dt)
                  idxes.push_back(i.toInt());
                std::sort(idxes.begin(), idxes.end(), std::greater<int>());

                auto owner = rea::GetMachineFingerPrint();
                QStringList dels;
                auto projs = getProjects();
                for (auto i : idxes) {
                  auto nm = projs[i].toString();
                  if (getProjectOwner(m_projects.value(nm).toObject()) ==
                      owner) {
                    dels.push_back(nm);
                  }
                  projs.erase(projs.begin() + i);
                }
                setProjects(projs);
                aInput->out<rea::stgJson>(
                    rea::stgJson(*this, "user/" + rea::GetMachineFingerPrint() +
                                            ".json"),
                    s3_bucket_name + "writeJson");

                if (dels.size() > 0) {
                  for (auto i : dels) {
                    m_projects.remove(i);
                    aInput->out<QString>("project/" + i + ".json",
                                         s3_bucket_name + "deletePath");
                    aInput->out<QString>("project/" + i,
                                         s3_bucket_name + "deletePath");
                  }
                  aInput->out<rea::stgJson>(
                      rea::stgJson(m_projects, "project.json"),
                      s3_bucket_name + "writeJson");
                }

                aInput->out<QJsonObject>(prepareProjectListGUI(getProjects()),
                                         "user_updateListView");
                aInput->out<QJsonArray>(QJsonArray(), "user_listViewSelected",
                                        rea::Json("tag", "manual"));
              }
            }));

    // open project
    rea::pipeline::find("user_listViewSelected")
        ->next(
            rea::pipeline::add<QJsonArray>(
                [this](rea::stream<QJsonArray> *aInput) {
                  auto dt = aInput->data();
                  if (dt.size() > 0) {
                    auto projs = getProjects();
                    auto id = projs[dt[0].toInt()].toString();
                    aInput->out<QJsonArray>(
                        QJsonArray(
                            {rea::GetMachineFingerPrint(),
                             getProjectName(m_projects.value(id).toObject())}),
                        "title_updateNavigation", rea::Json("tag", "manual"));

                    m_project_owner =
                        getProjectOwner(m_projects.value(id).toObject()) ==
                        rea::GetMachineFingerPrint();
                    aInput->out<QJsonObject>(
                        rea::Json("id", id, "abstract", m_projects.value(id)),
                        openProject);
                  }
                }),
            rea::Json("tag", openProject))
        ->next(openProject);

    // new project, import project
    rea::pipeline::find("_newObject")
        ->next(
            rea::pipeline::add<QJsonObject>([this](rea::stream<QJsonObject>
                                                       *aInput) {
              auto proj = aInput->data();

              auto projs = getProjects();
              if (proj.contains("id")) {
                auto id = proj.value("id").toString();
                if (!m_projects.contains(id)) {
                  aInput->out<QJsonObject>(
                      rea::Json("title", "warning", "text", "Invalid id!"),
                      "popMessage");
                  return;
                } else {
                  for (auto i : projs)
                    if (i == id) {
                      aInput->out<QJsonObject>(rea::Json("title", "warning",
                                                         "text",
                                                         "Existed project!"),
                                               "popMessage");
                      return;
                    }
                  projs = addProject(id);
                }
              } else {
                auto nm = proj.value("name").toString();
                if (nm == "") {
                  aInput->out<QJsonObject>(
                      rea::Json("title", "warning", "text", "Invalid name!"),
                      "popMessage");
                  return;
                } else if (proj.value("channel").toInt() <= 0) {
                  aInput->out<QJsonObject>(rea::Json("title", "warning", "text",
                                                     "Invalid channel count!"),
                                           "popMessage");
                  return;
                } else
                  projs = addProject(insertProject(proj));
              }
              aInput->out<QJsonObject>(prepareProjectListGUI(projs),
                                       "user_updateListView");
              aInput->out<rea::stgJson>(
                  rea::stgJson(*this, "user/" + rea::GetMachineFingerPrint() +
                                          ".json"),
                  s3_bucket_name + "writeJson");
              aInput->out<rea::stgJson>(
                  rea::stgJson(m_projects, "project.json"),
                  s3_bucket_name + "writeJson");
              aInput->out<QJsonArray>(QJsonArray(), "user_listViewSelected",
                                      rea::Json("tag", "manual"));
            }),
            rea::Json("tag", "newProject"));

    // load user
    rea::pipeline::add<double>(
        [](rea::stream<double> *aInput) {
          aInput->out<rea::stgJson>(
              rea::stgJson(QJsonObject(), "project.json"));
          aInput->out<rea::stgJson>(rea::stgJson(
              QJsonObject(), "user/" + rea::GetMachineFingerPrint() + ".json"));
        },
        rea::Json("name", loaduser))
        ->next(rea::local(s3_bucket_name + "readJson", rea::Json("thread", 10)))
        ->next(rea::buffer<rea::stgJson>(2))
        ->next(rea::pipeline::add<std::vector<rea::stgJson>>(
            [this](rea::stream<std::vector<rea::stgJson>> *aInput) {
              auto dt = aInput->data();
              m_projects = dt[0].getData();
              dt[1].getData().swap(*this);

              aInput->out<QJsonArray>(
                  QJsonArray({rea::GetMachineFingerPrint()}),
                  "title_updateNavigation", rea::Json("tag", "manual"));
              aInput->out<QJsonObject>(prepareProjectListGUI(getProjects()),
                                       "user_updateListView");
              aInput->out<QJsonArray>(QJsonArray(), "user_listViewSelected",
                                      rea::Json("tag", "manual"));
            }));

    // import old project
    /* rea::pipeline::find("_selectFile")
         ->next(rea::pipeline::add<QJsonArray>([](rea::stream<QJsonArray>*
       aInput){ auto pths = aInput->data(); if (pths.size() > 0){ auto usr =
       pths[0].toString(); aInput->cache<QString>(usr.mid(0,
       usr.lastIndexOf("/userInfo")))->out<rea::stgJson>(rea::stgJson(QJsonObject(),
       usr), "readJson");
                    }
                }), rea::Json("tag", "importOldProject"))
         ->next(rea::local("readJson", rea::Json("thread", 10)))
         ->next(rea::pipeline::add<rea::stgJson>([this](rea::stream<rea::stgJson>*
       aInput){ //cache: root, count, <project_id, project_config>, <project_id,
       project_image_config>, <project_id, image_count> auto rt =
       aInput->cacheData<QString>(0); auto dt = aInput->data().getData(); auto
       projs = getProjects(); auto abs =
       dt.value("project_abstract").toObject();
             aInput->cache<int>(abs.keys().size());
             aInput->cache<std::shared_ptr<QHash<QString,
       QJsonObject>>>(std::make_shared<QHash<QString, QJsonObject>>());
             aInput->cache<std::shared_ptr<QHash<QString,
       QJsonObject>>>(std::make_shared<QHash<QString, QJsonObject>>());
             aInput->cache<std::shared_ptr<QHash<QString,
       int>>>(std::make_shared<QHash<QString, int>>()); for (auto i :
       abs.keys()){ projs.push_back(i); auto proj = abs.value(i).toObject();
                 m_projects.insert(i, rea::Json("name", proj.value("caption"),
       "time", proj.value("time"), "owner", rea::GetMachineFingerPrint()));
                 aInput->out<rea::stgJson>(rea::stgJson(QJsonObject(), rt +
       "/projectInfo/" + i + ".json"));
             }
             setProjects(projs);
             aInput->out<rea::stgJson>(rea::stgJson(*this, "user/" +
       rea::GetMachineFingerPrint() + ".json"), s3_bucket_name + "writeJson");
         }))
         ->nextB(s3_bucket_name + "writeJson")
         ->next(rea::local("readJson", rea::Json("thread", 10)))
         ->next(rea::pipeline::add<rea::stgJson>([this](rea::stream<rea::stgJson>*
       aInput){ auto rt = aInput->cacheData<QString>(0); auto dt =
       aInput->data(); auto id = dt.mid(dt.lastIndexOf("/") + 1, dt.length());
             id = id.mid(0, id.lastIndexOf(".json"));
             auto proj = dt.getData();
             auto cnt = aInput->cacheData<int>(1);
             m_projects.insert(id, rea::Json(m_projects.value(id).toObject(),
       "channel", proj.value("channelcount"))); if (cnt - 1)
                 aInput->cache<int>(cnt - 1, 1);
             else
                 aInput->out<rea::stgJson>(rea::stgJson(m_projects,
       "project.json"), s3_bucket_name + "writeJson");

             auto proj_cfgs = aInput->cacheData<std::shared_ptr<QHash<QString,
       QJsonObject>>>(2);

             auto tsks = proj.value("task_abstract").toObject();
             QJsonArray tsk_lst;
             for (auto i : tsks.keys())
                 tsk_lst.push_back(i);

             auto img_lbls0 = proj.value("image_labels").toObject();
             QJsonObject tar_lbls;
             for (auto i : img_lbls0.keys()){
                 QJsonObject lbls;
                 auto lbls0 = img_lbls0.value(i).toArray();
                 for (auto j : lbls0){
                     auto lbl = j.toObject();
                     lbls.insert(lbl.value("label").toString(),
       lbl.value("color"));
                 }
                 tar_lbls.insert(i, lbls);
             }
             auto lbls0 = proj.value("shape_labels").toArray();
             QJsonObject shp_lbls;
             for (auto i : lbls0){
                 auto lbl = i.toObject();
                 shp_lbls.insert(lbl.value("label").toString(),
       lbl.value("color"));
             }
             tar_lbls.insert("shape", shp_lbls);
             proj_cfgs->insert(id, rea::Json("labels", tar_lbls, "tasks",
       tsk_lst)); aInput->out<rea::stgJson>(rea::stgJson(QJsonObject(), rt +
       "/projectImageInfo/" + id + ".json"));
         }))
         ->nextB(s3_bucket_name + "writeJson")
         ->next(rea::local("readJson", rea::Json("thread", 10)))
         ->next(rea::pipeline::add<rea::stgJson>([this](rea::stream<rea::stgJson>*
       aInput){ auto rt = aInput->cacheData<QString>(0); auto dt =
       aInput->data(); auto id = dt.mid(dt.lastIndexOf("/") + 1, dt.length());
             id = id.mid(0, id.lastIndexOf(".json"));

             auto dt0 = aInput->data().getData();
             auto proj_cfgs = aInput->cacheData<std::shared_ptr<QHash<QString,
       QJsonObject>>>(2); auto proj_imgs =
       aInput->cacheData<std::shared_ptr<QHash<QString, QJsonObject>>>(3); auto
       proj_imgs_cnt = aInput->cacheData<std::shared_ptr<QHash<QString,
       int>>>(4); QJsonObject img_cfgs; auto imgs =
       dt0.value("image_abstract").toObject(); auto proj = proj_cfgs->value(id);
             QJsonArray img_lst;
             proj_imgs_cnt->insert(id, imgs.size());
             for (auto i : imgs.keys()){
                 img_lst.push_back(i);
                 auto img = imgs.value(i).toObject();
                 img_cfgs.insert(i, rea::Json("time", img.value("time"),
       "image_label", img.value("labels"))); auto ot =
       aInput->out<rea::stgJson>(rea::stgJson(QJsonObject(), rt + "/imageInfo/"
       + i + ".json"), "readJson", QJsonObject(), false);
                 ot->cache<QString>(rt);
                 ot->cache<QString>(id);
                 ot->cache<std::shared_ptr<QHash<QString,
       QJsonObject>>>(proj_imgs); ot->cache<std::shared_ptr<QHash<QString,
       int>>>(proj_imgs_cnt);
             }
             proj.insert("images", img_lst);
             proj_cfgs->insert(id, proj);
             proj_imgs->insert(id, img_cfgs);
             //aInput->out<rea::stgJson>(rea::stgJson(img_cfgs, "project/" + id
       + "/image.json"), s3_bucket_name + "writeJson");
             aInput->out<rea::stgJson>(rea::stgJson(proj, "project/" + id +
       ".json"));
         }))
         ->nextB(rea::pipeline::find("readJson")->nextB(
                     rea::pipeline::add<rea::stgJson>([](rea::stream<rea::stgJson>*
       aInput){ auto rt = aInput->cacheData<QString>(0); auto proj =
       aInput->cacheData<QString>(1); auto proj_imgs =
       aInput->cacheData<std::shared_ptr<QHash<QString, QJsonObject>>>(2);

                         auto proj_imgs_cnt =
       aInput->cacheData<std::shared_ptr<QHash<QString, int>>>(3); auto dt =
       aInput->data(); auto id = dt.mid(dt.lastIndexOf("/") + 1, dt.length());
                         id = id.mid(0, id.lastIndexOf(".json"));

                         auto img = aInput->data().getData();
                         auto srcs = img.value("source").toArray();

                         auto proj_imgs_cfg = proj_imgs->value(proj);
                         proj_imgs_cfg.insert(id,
       rea::Json(proj_imgs_cfg.value(id).toObject(), "name", srcs));
                         proj_imgs->insert(proj, proj_imgs_cfg);
                         auto img_cfg = rea::Json("local", img.value("local"),
       "source", img.value("source")); for (auto i : srcs){ auto ot =
       aInput->out<stgCVMat>(stgCVMat(cv::Mat(), rt + "/image/" + id + "/" +
       i.toString()), "", QJsonObject(), false); ot->cache<QString>(proj);
                             ot->cache<QString>(id);
                             ot->cache<std::shared_ptr<QHash<QString,
       int>>>(proj_imgs_cnt); ot->cache<int>(srcs.size());
                             ot->cache<QJsonObject>(img_cfg);
                         }
                     })->nextB(rea::local("readCVMat", rea::Json("thread", 10))
                             ->nextB(rea::pipeline::add<stgCVMat>([this](rea::stream<stgCVMat>*
       aInput){ auto proj = aInput->cacheData<QString>(0); auto id =
       aInput->cacheData<QString>(1); auto proj_imgs_cnt =
       aInput->cacheData<std::shared_ptr<QHash<QString, int>>>(2); auto srcs_cnt
       = aInput->cacheData<int>(3); auto img_cfg =
       aInput->cacheData<QJsonObject>(4); srcs_cnt--; auto dt =
       aInput->data().getData();
                                 //aInput->out<stgCVMat>(stgCVMat(dt, "project/"
       + proj + "/image/" + id + "/"), s3_bucket_name + "writeCVMat");
                                     }))
                                     ->nextB(s3_bucket_name + "writeCVMat")
                                 ),
                     rea::Json("tag", "readV3ImageInfo")
                     ),
                 rea::Json("tag", "readV3ImageInfo"))
         ->next(rea::local(s3_bucket_name + "writeJson", rea::Json("thread",
       11)))
         ->next(rea::pipeline::add<rea::stgJson>([](rea::stream<rea::stgJson>*
       aInput){ auto proj_cfgs =
       aInput->cacheData<std::shared_ptr<QHash<QString, QJsonObject>>>(2); auto
       dt = aInput->data(); auto id = dt.mid(dt.lastIndexOf("/") + 1,
       dt.length()); id = id.mid(0, id.lastIndexOf(".json"));
             proj_cfgs->remove(id);
             if (proj_cfgs->size() == 0)
                 aInput->out<double>(0, "loadUser");
         }))
         ->next("loadUser");*/
  }
};

static rea::regPip<QQmlApplicationEngine *> init_user(
    [](rea::stream<QQmlApplicationEngine *> *aInput) {
      static fsStorage local_storage;
      static user cur_user;
      aInput->out();
    },
    rea::Json("name", "install1_user"), "regQML");
