#include "model.h"
#include "imagePool.h"
#include "storage/storage.h"
#include "../util/cv.h"

class task : public model{
private:
    void setLabels(const QJsonObject& aLabels){
        rea::pipeline::run<QJsonObject>("taskLabelChanged", aLabels);
        insert("labels", aLabels);
    }
    QString getImageStage(const QJsonObject& aImage){
        return aImage.value("stage").toString();
    }
    void setImageStage(QJsonObject& aImage, const QString& aStage){
        aImage.insert("stage", aStage);
    }
    QJsonObject getImages(){
        return value("images").toObject();
    }
    void setImages(const QJsonObject& aImages){
        insert("images", aImages);
    }
    QJsonArray getImageList(){
        if (contains("image_list"))
            return value("image_list").toArray();
        else{
            QJsonArray ret;
            for (auto i : m_project_images.keys())
                ret.push_back(i);
            return ret;
        }
    }
    void setImageList(const QJsonArray& aList, bool aRemove = false){
        if (aRemove)
            remove("image_list");
        else
            insert("image_list", aList);
    }
    QJsonObject prepareLabelListGUI(const QJsonObject& aLabels){
        QJsonArray data;
        for (auto i : m_project_labels.keys())
            data.push_back(rea::Json("entry", rea::JArray(i, aLabels.contains(i))));
        return rea::Json("title", rea::JArray("group", "used"),
                         "selects", m_project_labels.size() > 0 ? rea::JArray("0") : QJsonArray(),
                         "data", data);
    }
    QJsonObject prepareImageListGUI(const QJsonObject& aImages){
        QJsonArray data;
        auto lst = getImageList();
        for (auto i : lst){
            auto img = i.toString();
            data.push_back(rea::Json("entry", rea::JArray(getImageStringName(m_project_images.value(img).toObject()),
                                                          aImages.contains(img),
                                                          getImageStage(aImages.value(img).toObject()))));
        }
        return rea::Json("title", rea::JArray("name", "used", "stage"),
                         "entrycount", 30,
                         "selects", lst.size() > 0 ? rea::JArray("0") : QJsonArray(),
                         "data", data);
    }
private:
    QString m_project_id = "";
    QString m_task_id = "";
    QJsonObject m_project_labels;
    QJsonObject m_project_images;
    void taskManagement(){
        //open project
        rea::pipeline::find("openProject")
            ->next(rea::pipeline::add<QJsonObject>([this](rea::stream<QJsonObject>* aInput){
                m_task_id = "";
            }));

        //open task
        rea::pipeline::add<QJsonObject>([this](rea::stream<QJsonObject>* aInput){
            auto dt = aInput->data();
            if (dt.value("id") != m_task_id){
                m_task_id = dt.value("id").toString();
                m_project_labels = dt.value("labels").toObject();
                m_project_images = dt.value("images").toObject();
                m_channel_count = dt.value("channel").toInt();
                m_project_id = dt.value("project").toString();
                m_image_show = dt.value("imageshow").toObject();
                aInput->out<stgJson>(stgJson(QJsonObject(), "project/" + m_task_id + ".json"));
            }
        }, rea::Json("name", "openTask"))
            ->next(rea::local("deepsightreadJson", rea::Json("thread", 10)))
            ->next(rea::buffer<stgJson>(1))
            ->next(rea::pipeline::add<std::vector<stgJson>>([this](rea::stream<std::vector<stgJson>>* aInput){
                auto dt = aInput->data();
                dt[0].getData().swap(*this);
                setLabels(value("labels").toObject());

                aInput->out<QJsonObject>(prepareLabelListGUI(getLabels()), "task_label_updateListView");
                aInput->out<QJsonArray>(QJsonArray(), "task_label_listViewSelected");
                aInput->out<QJsonObject>(prepareImageListGUI(getImages()), "task_image_updateListView");
                aInput->out<QJsonObject>(rea::Json("count", 1), "scatterTaskImageShow");
                aInput->out<QJsonObject>(getFilter(), "updateTaskImageFilterGUI");
            }))
            ->nextB(0, "task_label_updateListView", QJsonObject())
            ->nextB(0, "task_label_listViewSelected", rea::Json("tag", "task_manual"))
            ->nextB(0, "task_image_updateListView", QJsonObject())
            ->nextB(0, "scatterTaskImageShow", QJsonObject())
            ->nextB(0, "updateTaskImageFilterGUI", QJsonObject());
    }
    void labelManagement(){
        //update project labels
        rea::pipeline::find("projectLabelChanged")
            ->next(rea::pipeline::add<QJsonObject>([this](rea::stream<QJsonObject>* aInput){
                if (m_task_id == "")
                    return;
                auto dt = aInput->data();
                if (m_project_labels.value("shape") != dt.value("shape")){
                    m_current_image = "";
                }
                m_project_labels = dt;
                aInput->out<QJsonObject>(prepareLabelListGUI(getLabels()), "task_label_updateListView");
                aInput->out<QJsonArray>(QJsonArray(), "task_label_listViewSelected");
            }))
            ->nextB(0, "task_label_updateListView", QJsonObject())
            ->nextB(0, "task_label_listViewSelected", rea::Json("tag", "task_manual"));

        //select task label group
        rea::pipeline::find("task_label_listViewSelected")
            ->next("getProjectLabel", rea::Json("tag", "task_manual"))
            ->next(rea::pipeline::add<QJsonObject>([this](rea::stream<QJsonObject>* aInput){
                   auto dt = aInput->data();
                   auto lbls = getLabels();
                   auto key = dt.value("key").toString();
                   auto val = lbls.value(key).toObject();
                   auto proj_val = dt.value("val").toObject();
                   for (auto i : val.keys())
                       val.insert(i, proj_val.value(i));
                   aInput->out<QJsonObject>(rea::Json("key", key, "val", val), "updateTaskLabelGUI");
                   aInput->out<QJsonObject>(dt, "updateCandidateLabelGUI");
               }), rea::Json("tag", "task_manual"))
            ->nextB(0, "updateCandidateLabelGUI", QJsonObject())
            ->next("updateTaskLabelGUI");

        //add/remove label
        auto addLabel = rea::buffer<QJsonArray>(2);
        auto addLabel_nm = "addTaskLabel";
        auto addLabel_tag = rea::Json("tag", addLabel_nm);
        rea::pipeline::add<QJsonObject>([](rea::stream<QJsonObject>* aInput){
            auto dt = aInput->data();
            aInput->out<QJsonArray>(rea::JArray(dt.value("label"), dt.value("add")));
            aInput->out<QJsonArray>(QJsonArray(), "task_label_listViewSelected");
        }, rea::Json("name", addLabel_nm))
            ->nextB(0, addLabel, QJsonObject())
            ->next("task_label_listViewSelected", addLabel_tag)
            ->next(addLabel, addLabel_tag)
            ->next(rea::pipeline::add<std::vector<QJsonArray>>([this](rea::stream<std::vector<QJsonArray>>* aInput){
                auto dt = aInput->data();
                auto lbl = dt[0][0].toString();
                auto add = dt[0][1].toBool();
                auto sel = dt[1][0].toInt();
                auto grps = getLabels();
                auto grp = (m_project_labels.begin() + sel).key();
                auto lbls = grps.value(grp).toObject();
                bool updatelist = !grps.contains(grp);
                if (add){
                    if (!lbls.contains(lbl)){
                        lbls.insert(lbl, QJsonObject());
                        grps.insert(grp, lbls);
                    }else
                        return;
                }else{
                    lbls.remove(lbl);
                    if (lbls.size() == 0){
                        grps.remove(grp);
                        updatelist = true;
                    }else
                        grps.insert(grp, lbls);
                }
                if (updatelist)
                    aInput->out<QJsonObject>(rea::Json(prepareLabelListGUI(grps), "selects", rea::JArray(sel)), "task_label_updateListView");
                setLabels(grps);
                aInput->out<stgJson>(stgJson(*this, "project/" + m_task_id + ".json"), "deepsightwriteJson");
                aInput->out<QJsonArray>(QJsonArray(), "task_label_listViewSelected");
                m_current_image = "";
                aInput->out<QJsonArray>(QJsonArray(), "task_image_listViewSelected");
            }))
            ->nextB(0, "task_label_updateListView", QJsonObject())
            ->nextB(0, "deepsightwriteJson", QJsonObject())
            ->nextB(0, "task_label_listViewSelected", rea::Json("tag", "task_manual"))
            ->next("task_image_listViewSelected", rea::Json("tag", "manual"));

        rea::pipeline::add<QJsonObject>([](rea::stream<QJsonObject>* aInput){
            aInput->out();
        }, rea::Json("name", "taskLabelChanged"));
    }
private:
    QString m_current_image = "";
    int m_channel_count;
    int m_show_count = 1;
    QJsonObject m_image_show;
    QHash<QString, int> m_show_cache;
    QJsonObject m_image;
    const QJsonObject selectTaskImage = rea::Json("tag", "selectTaskImage");

    void imageManagement(){
        //update project images
        rea::pipeline::find("deepsightwriteJson")
        ->next(rea::pipeline::add<stgJson>([this](rea::stream<stgJson>* aInput){
            if (m_task_id == "")
                return;
            m_project_images = aInput->data().getData();
            aInput->out<QJsonObject>(prepareImageListGUI(getImages()), "task_image_updateListView");
        }), rea::Json("tag", "updateProjectImages"))
        ->nextB(0, "task_image_updateListView", QJsonObject());

        //select image
        rea::pipeline::find("task_image_listViewSelected")
            ->next(rea::pipeline::add<QJsonArray>([this](rea::stream<QJsonArray>* aInput){
                   if (m_task_id == "")
                       return;
                   auto dt = aInput->data();
                   if (dt.size() == 0){
                       rea::pipeline::run<QJsonArray>("updateQSGCtrl_taskimage_gridder0", QJsonArray());  //must be before "updateQSGModel_projectimage_gridder"
                       m_current_image = "";
                       for (int i = 0; i < m_show_count; ++i)
                           rea::pipeline::run<QJsonObject>("updateQSGModel_taskimage_gridder" + QString::number(i), QJsonObject());
                       aInput->out<QJsonObject>(QJsonObject(), "updateTaskImageGUI");
                   }
                   else{
                       auto lst = getImageList();
                       auto idx = dt[0].toInt();
                       if (idx < lst.size()){
                           auto nm = lst[idx].toString();
                           if (nm != m_current_image){
                               aInput->out<QJsonArray>(QJsonArray(), "updateQSGCtrl_taskimage_gridder0");
                               m_current_image = nm;
                               auto nms = getImageName(m_project_images.value(m_current_image).toObject());
                               aInput->out<stgJson>(stgJson(QJsonObject(), "project/" + m_project_id + "/image/" + nm + ".json"), "deepsightreadJson", selectTaskImage);
                               for (auto i = 0; i < m_show_count; ++i){
                                   auto img = "project/" + m_project_id + "/image/" + nm + "/" + nms[i].toString();
                                   m_show_cache.insert(img, i);
                                   aInput->out<stgCVMat>(stgCVMat(cv::Mat(), img));
                               }
                           }
                           //aInput->out<QJsonObject>(m_images.value(nm).toObject(), "updateTaskImageGUI");
                       }else{
                           aInput->out<QJsonObject>(QJsonObject(), "updateTaskImageGUI");
                       }
                   }
               }), rea::Json("tag", "manual"))
            ->nextB(0, "updateQSGCtrl_projectimage_gridder0", QJsonObject())
            ->nextB(0, "deepsightreadJson", selectTaskImage, rea::pipeline::add<stgJson>([this](rea::stream<stgJson>* aInput){
                m_image = aInput->data().getData();
                auto img_lbls = getImageLabels(m_project_images.value(m_current_image).toObject());
                auto tsk_lbls = getLabels();
                std::vector<QString> dels;
                for (auto i : img_lbls.keys()){
                    auto lbl0 = tsk_lbls.value(i).toObject();
                    auto lbl1 = m_project_labels.value(i).toObject();
                    auto lbl = img_lbls.value(i).toString();
                    if (!lbl0.contains(lbl) || !lbl1.contains(lbl))
                        dels.push_back(i);
                }
                for (auto  i: dels)
                    img_lbls.remove(i);
                aInput->out<QJsonObject>(rea::Json(m_image, "image_label", img_lbls), "updateTaskImageGUI");
            }), selectTaskImage, "updateTaskImageGUI", QJsonObject())
            ->nextB(0, "updateTaskImageGUI", QJsonObject())
            ->next(rea::local("deepsightreadCVMat", rea::Json("thread", 10)))
            ->next(rea::pipeline::add<stgCVMat>([this](rea::stream<stgCVMat>* aInput){
                auto dt = aInput->data();
                auto ch = m_show_cache.value(dt);
                auto pth = QString(dt);
                auto img = cvMat2QImage(dt.getData());
                rea::imagePool::cacheImage(pth, img);

                auto objs = rea::Json("img_" + m_current_image, rea::Json(
                                                                    "type", "image",
                                                                    "range", rea::JArray(0, 0, img.width(), img.height()),
                                                                    "path", pth,
                                                                    "transform", m_image_show));

                auto tsk_shp_lbls = getLabels().value("shape").toObject(), prj_shp_lbls = m_project_labels.value("shape").toObject();

                auto shps = getShapes(m_image);
                for (auto i : shps.keys()){
                    auto shp = shps.value(i).toObject();
                    auto lbl = shp.value("label").toString();
                    if (tsk_shp_lbls.contains(lbl) && prj_shp_lbls.contains(lbl)){
                        if (shp.value("type") == "ellipse"){
                            objs.insert(i, rea::Json("type", "ellipse",
                                                     "caption", shp.value("label"),
                                                     "center", shp.value("center"),
                                                     "radius", rea::JArray(shp.value("xradius"), shp.value("yradius")),
                                                     "angle", shp.value("angle")));
                        }else if (shp.value("type") == "polyline"){
                            QJsonArray pts;
                            pts.push_back(shp.value("points"));
                            auto holes = shp.value("holes").toArray();
                            for (auto i : holes)
                                pts.push_back(i);
                            objs.insert(i, rea::Json("type", "poly",
                                                     "caption", shp.value("label"),
                                                     "points", pts));
                        }
                    }
                }

                auto cfg = rea::Json("id", "project/" + m_project_id + "/image/" + m_current_image + ".json",
                                     "width", img.width(),
                                     "height", img.height(),
                                     "face", 100,
                                     "text", rea::Json("visible", true,
                                                       "size", rea::JArray(80, 30)
                                                       //"location", "bottom"
                                                                                                                  ),
                                     "objects", objs);
                rea::pipeline::run<QJsonObject>("updateQSGModel_taskimage_gridder" + QString::number(ch), cfg);
                //aInput->out<QJsonObject>(cfg, "updateQSGModel_projectimage_gridder0");
            }));

        //use task image
        auto useTaskImage = rea::buffer<QJsonArray>(2);
        const QString useTaskImage_nm = "useTaskImage";
        rea::pipeline::add<bool>([](rea::stream<bool>* aInput){
            aInput->out<QJsonArray>(rea::JArray(aInput->data()));
            aInput->out<QJsonArray>(QJsonArray(), "task_image_listViewSelected");
        }, rea::Json("name", useTaskImage_nm))
            ->nextB(0, "task_image_listViewSelected", rea::Json("tag", useTaskImage_nm), useTaskImage, rea::Json("tag", useTaskImage_nm))
            ->next(useTaskImage)
            ->next(rea::pipeline::add<std::vector<QJsonArray>>([this](rea::stream<std::vector<QJsonArray>>* aInput){
                auto dt = aInput->data();
                auto lst = getImageList();
                auto imgs = getImages();
                auto add = dt[0][0].toBool();
                auto sels = dt[1];
                bool mdy = false;
                for (auto i : sels){
                    auto img = lst[i.toInt()].toString();
                    if (!imgs.contains(img) == add){
                        if (add)
                            imgs.insert(img, QJsonObject());
                        else
                            imgs.remove(img);
                        mdy = true;
                    }
                }
                if (mdy){
                    setImages(imgs);
                    aInput->out<stgJson>(stgJson(*this, "project/" + m_task_id + ".json"), "deepsightwriteJson");
                    aInput->out<QJsonObject>(prepareImageListGUI(getImages()), "task_image_updateListView");
                    aInput->out<QJsonArray>(QJsonArray(), "task_image_listViewSelected");
                }
            }))
            ->nextB(0, "deepsightwriteJson", QJsonObject())
            ->nextB(0, "task_image_updateListView", QJsonObject())
            ->next("task_image_listViewSelected", rea::Json("tag", "manual"));

        //filter images
        rea::pipeline::add<QJsonObject>([this](rea::stream<QJsonObject>* aInput){
            auto dt = aInput->data();
            QJsonArray lst;
            auto imgs = getImages();
            if (dt.value("type") == "all"){
                setImageList(lst, true);
            }else if (dt.value("type") == "used"){
                for (auto i : imgs.keys())
                    lst.push_back(i);
                setImageList(lst);
            }else if (dt.value("type") == "stage"){
                auto stg = dt.value("value").toString();
                for (auto i : imgs.keys()){
                    auto img = imgs.value(i).toObject();
                    if (img.value("stage") == stg)
                        lst.push_back(i);
                }
                setImageList(lst);
            }else
                return;
            setFilter(dt);
            aInput->out<QJsonObject>(prepareImageListGUI(imgs), "task_image_updateListView");
            aInput->out<QJsonArray>(QJsonArray(), "task_image_listViewSelected");
            aInput->out<stgJson>(stgJson(*this, "project/" + m_task_id + ".json"), "deepsightwriteJson");
        }, rea::Json("name", "filterTaskImages"))
            ->nextB(0, "task_image_updateListView", QJsonObject())
            ->nextB(0, "task_image_listViewSelected", rea::Json("tag", "manual"))
            ->nextB(0, "deepsightwriteJson", QJsonObject());

        //automatic set image stage
        if (!rea::pipeline::find("automaticSetImageStage", false))
            rea::pipeline::add<QJsonObject>([](rea::stream<QJsonObject>* aInput){
                auto dt = aInput->data();
                auto imgs = dt.value("images").toObject();
                std::cout << dt.value("train").toString().toInt() << std::endl;
                std::cout << dt.value("test").toString().toInt() << std::endl;
                std::cout << dt.value("validation").toString().toInt() << std::endl;
                for (auto i : imgs.keys()){
                    auto img = imgs.value(i).toObject();
                    img.insert("stage", "test");
                    imgs.insert(i, img);
                }
                aInput->setData(rea::Json(dt, "images", imgs))->out();
            }, rea::Json("name", "automaticSetImageStage"));

        const QString setImageStage_nm = "setImageStage";
        auto setImageStage_tag = rea::Json("tag", setImageStage_nm);
        rea::pipeline::find("_newObject")
        ->next(rea::pipeline::add<QJsonObject>([this](rea::stream<QJsonObject>* aInput){
            auto dt = aInput->data();
            aInput->setData(rea::Json(dt, "images", getImages()))->out();
        }), setImageStage_tag)
        ->next("automaticSetImageStage")
        ->next(rea::pipeline::add<QJsonObject>([this](rea::stream<QJsonObject>* aInput){
            auto dt = aInput->data();
            auto imgs = dt.value("images").toObject();
            setImages(imgs);
            aInput->out<QJsonObject>(prepareImageListGUI(imgs), "task_image_updateListView");
            aInput->out<QJsonArray>(QJsonArray(), "task_image_listViewSelected");
            aInput->out<stgJson>(stgJson(*this, "project/" + m_task_id + ".json"), "deepsightwriteJson");
        }))
        ->nextB(0, "task_image_updateListView", QJsonObject())
        ->nextB(0, "task_image_listViewSelected", rea::Json("tag", "manual"))
        ->nextB(0, "deepsightwriteJson", QJsonObject());

        //manual set image stage
        auto setImageStage = rea::buffer<QJsonArray>(2);
        rea::pipeline::add<QString>([](rea::stream<QString>* aInput){
            aInput->out<QJsonArray>(rea::JArray(aInput->data()));
            aInput->out<QJsonArray>(QJsonArray(), "task_image_listViewSelected");
        }, rea::Json("name", setImageStage_nm))
        ->nextB(0, "task_image_listViewSelected", setImageStage_tag, setImageStage, setImageStage_tag)
        ->next(setImageStage)
        ->next(rea::pipeline::add<std::vector<QJsonArray>>([this](rea::stream<std::vector<QJsonArray>>* aInput){
            auto dt = aInput->data();
            auto stg = dt[0][0].toString();
            auto sels = dt[1];
            auto lst = getImageList();
            auto imgs = getImages();
            bool mdy = false;
            for (auto i : sels){
                auto nm = lst[i.toInt()].toString();
                if (imgs.contains(nm)){
                    auto img = imgs.value(nm).toObject();
                    this->setImageStage(img, stg);
                    imgs.insert(nm, img);
                    mdy = true;
                }
            }
            if (mdy){
                setImages(imgs);
                aInput->out<QJsonObject>(prepareImageListGUI(imgs), "task_image_updateListView");
                aInput->out<QJsonArray>(QJsonArray(), "task_image_listViewSelected");
                aInput->out<stgJson>(stgJson(*this, "project/" + m_task_id + ".json"), "deepsightwriteJson");
            }
        }))
        ->nextB(0, "task_image_updateListView", QJsonObject())
        ->nextB(0, "task_image_listViewSelected", rea::Json("tag", "manual"))
        ->nextB(0, "deepsightwriteJson", QJsonObject());
    }
private:
    void guiManagement(){
        //the qsgnode will be destroyed by qt after switch page
        rea::pipeline::find("title_updateStatus")
            ->next(rea::pipeline::add<QJsonArray>([this](rea::stream<QJsonArray>* aInput){
                if (aInput->data().size() == 3)
                    aInput->out<QJsonArray>(QJsonArray(), "task_image_listViewSelected");
                else
                    m_current_image = "";
            }))
            ->next("task_image_listViewSelected", rea::Json("tag", "manual"));

        //switch scatter mode
        rea::pipeline::add<QJsonObject>([this](rea::stream<QJsonObject>* aInput){
            auto dt = aInput->data();
            if (dt.contains("count"))
                m_show_count = dt.value("count").toInt();
            else
                m_show_count = m_show_count == m_channel_count ? 1 : m_channel_count;
            aInput->out<QJsonObject>(rea::Json("size", m_show_count), "taskimage_updateViewCount");
            m_current_image = "";
            aInput->out<QJsonArray>(QJsonArray(), "task_image_listViewSelected");
        }, rea::Json("name", "scatterTaskImageShow"))
            ->nextB(0, "taskimage_updateViewCount", QJsonObject())
            ->next("task_image_listViewSelected", rea::Json("tag", "manual"));

        //modify image show
        rea::pipeline::find("_newObject")
        ->next(rea::pipeline::add<QJsonObject>([this](rea::stream<QJsonObject>* aInput){
               if (m_task_id == "")
                   return;
               auto dt = aInput->data();
               bool mdy = false;
               for (auto i : dt.keys()){
                   if (dt.value(i) != m_image_show.value(i)){
                       m_image_show.insert(i, dt.value(i));
                       mdy = true;
                   }
               }
               if (mdy){
                   if (m_current_image != ""){
                       for (int i = 0; i < m_show_count; ++i)
                           rea::pipeline::run<QJsonObject>("updateQSGAttr_taskimage_gridder" + QString::number(i),
                                                           rea::Json("obj", "img_" + m_current_image, "key", rea::JArray("transform"), "val", m_image_show));
                   }
               }
        }), rea::Json("tag", "setImageShow"));
    }
public:
    task(){
        taskManagement();
        labelManagement();
        imageManagement();
        guiManagement();
    }
};

static rea::regPip<QQmlApplicationEngine*> init_task([](rea::stream<QQmlApplicationEngine*>* aInput){
    static task cur_task;
    aInput->out();
}, QJsonObject(), "regQML");
