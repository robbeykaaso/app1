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
            ->next(rea::local("updateCandidateLabelGUI"), rea::Json("tag", "task_manual"));

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

        //add label

        //remove label
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
