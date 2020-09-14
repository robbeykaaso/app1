#include "model.h"
#include "storage/storage.h"

class task : public model{
private:
    void setLabels(const QJsonObject& aLabels){
        insert("labels", aLabels);
    }
private:
    QString m_task_id;
    QJsonArray m_project_label_groups;
    void taskManagement(){
        //open task
        rea::pipeline::add<QJsonObject>([this](rea::stream<QJsonObject>* aInput){
            auto dt = aInput->data();
            if (dt.value("id") != m_task_id){
                m_task_id = dt.value("id").toString();
                aInput->out<stgJson>(stgJson(QJsonObject(), "project/" + m_task_id + ".json"));
            }
        }, rea::Json("name", "openTask"))
            ->next(rea::local("deepsightreadJson", rea::Json("thread", 10)))
            ->next(rea::buffer<stgJson>(1))
            ->next(rea::pipeline::add<std::vector<stgJson>>([this](rea::stream<std::vector<stgJson>>* aInput){
                auto dt = aInput->data();
                dt[0].getData().swap(*this);

                aInput->out<QJsonObject>(prepareLabelListGUI(getLabels()), "task_label_updateListView");
                aInput->out<QJsonArray>(QJsonArray(), "task_label_listViewSelected");
                aInput->out<QJsonArray>(QJsonArray(), "candidate_label_listViewSelected");
            }))
            ->nextB(0, "task_label_updateListView", QJsonObject())
            ->nextB(0, "task_label_listViewSelected", rea::Json("tag", "task_manual"))
            ->next("candidate_label_listViewSelected", rea::Json("tag", "candidate_manual"));
    }
    void labelManagement(){
        //update project label groups
        rea::pipeline::find("project_label_updateListView")
            ->next(rea::pipeline::add<QJsonObject>([this](rea::stream<QJsonObject>* aInput){
                m_project_label_groups = aInput->data().value("data").toArray();
            }));

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

        //select candidate label group
        rea::pipeline::find("candidate_label_listViewSelected")
            ->next("getProjectLabel", rea::Json("tag", "candidate_manual"))
            ->next(rea::local("updateCandidateLabelGUI2"), rea::Json("tag", "candidate_manual"));

        //add label group
        rea::pipeline::find("candidate_label_listViewSelected")
        ->next(rea::pipeline::add<QJsonArray>([this](rea::stream<QJsonArray>* aInput){
            auto sels = aInput->data();
            bool mdy = false;
            auto lbls = getLabels();
            for (auto i : sels){
                auto grp = m_project_label_groups[i.toInt()].toObject().value("entry").toArray()[0].toString();
                if (!lbls.contains(grp)){
                    lbls.insert(grp, QJsonObject());
                    mdy = true;
                }
            }
            if (mdy){
                setLabels(lbls);
                aInput->out<stgJson>(stgJson(*this, "project/" + m_task_id + ".json"), "deepsightwriteJson");
                aInput->out<QJsonObject>(prepareLabelListGUI(getLabels()), "task_label_updateListView");
                aInput->out<QJsonArray>(QJsonArray(), "task_label_listViewSelected");
            }
        }), rea::Json("tag", "addTaskLabelGroup"))
            ->nextB(0, "deepsightwriteJson", QJsonObject())
            ->nextB(0, "task_label_updateListView", QJsonObject())
            ->nextB(0, "task_label_listViewSelected", rea::Json("tag", "task_manual"));

        //remove label group
        rea::pipeline::find("task_label_listViewSelected")
            ->next(rea::pipeline::add<QJsonArray>([this](rea::stream<QJsonArray>* aInput){
                   auto sels = aInput->data();
                   if (sels.size() > 0){
                       auto lbls = getLabels();

                       std::vector<int> idxes;
                       for (auto i : sels)
                           idxes.push_back(i.toInt());
                       std::sort(idxes.begin(), idxes.end(), std::greater<int>());
                       for (auto i : idxes)
                           lbls.erase(lbls.begin() + i);

                       setLabels(lbls);
                       aInput->out<stgJson>(stgJson(*this, "project/" + m_task_id + ".json"), "deepsightwriteJson");
                       aInput->out<QJsonObject>(prepareLabelListGUI(getLabels()), "task_label_updateListView");
                       aInput->out<QJsonArray>(QJsonArray(), "task_label_listViewSelected");
                   }
               }), rea::Json("tag", "removeTaskLabelGroup"))
            ->nextB(0, "deepsightwriteJson", QJsonObject())
            ->nextB(0, "task_label_updateListView", QJsonObject())
            ->nextB(0, "task_label_listViewSelected", rea::Json("tag", "task_manual"));

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
                auto lbls = (grps.begin() + sel)->toObject();

                if (add){
                    if (!lbls.contains(lbl))
                        lbls.insert(lbl, QJsonObject());
                    else
                        return;
                }else
                    lbls.remove(lbl);
                grps.insert((grps.begin() + sel).key(), lbls);
                setLabels(grps);
                aInput->out<stgJson>(stgJson(*this, "project/" + m_task_id + ".json"), "deepsightwriteJson");
                aInput->out<QJsonArray>(QJsonArray(), "task_label_listViewSelected");
            }))
            ->nextB(0, "deepsightwriteJson", QJsonObject())
            ->nextB(0, "task_label_listViewSelected", rea::Json("tag", "task_manual"));
    }
public:
    task(){
        taskManagement();
        labelManagement();
    }
};

static rea::regPip<QQmlApplicationEngine*> init_task([](rea::stream<QQmlApplicationEngine*>* aInput){
    static task cur_task;
    aInput->out();
}, QJsonObject(), "regQML");
