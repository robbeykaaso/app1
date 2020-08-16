#include "qsgBoard.h"

class qsgPluginEditNode : public rea::qsgPluginTransform{
public:
    qsgPluginEditNode(const QJsonObject& aConfig) : qsgPluginTransform(aConfig){

    }
private:
    void setSelect(const QString& aName, const QJsonObject& aDetail = QJsonObject()){
        if (!aDetail.empty())
            m_selects.insert(aName);
        else
            m_selects.remove(aName);
        rea::pipeline::run("updateQSGAttr_" + getParentName(), rea::Json("key", rea::JArray("width"),
                                                                         "obj", aName,
                                                                         "val", aDetail.empty() ? 5 : 10,
                                                                         "detail", aDetail));
    }
    void clearSelects(){
        auto sels = m_selects;
        for (auto i : sels)
            setSelect(i);
    }
protected:
    void mousePressEvent(QMouseEvent *event) override {

    }
    void mouseReleaseEvent(QMouseEvent *event) override {
            if (event->button() == Qt::LeftButton){
                auto mdl = getQSGModel();
                if (mdl){
                    QString sel = "";
                    auto objs = mdl->getQSGObjects();
                    auto inv = m_transnode->matrix().inverted();
                    auto pos = inv.map(event->pos());
                    for (auto i : objs.keys()){
                        auto obj = objs.value(i);
                        auto tp = obj->value("type");
                        if ((tp == "poly" || tp == "ellipse") && obj->bePointSelected(pos.x(), pos.y())){
                            sel = i;
                            break;
                        }
                    }
                    if (sel == "")
                        clearSelects();
                    else{
                        if (event->modifiers().testFlag(Qt::ControlModifier)){
                            setSelect(sel, m_selects.contains(sel) ? QJsonObject() : *objs.value(sel));
                        }else{
                            clearSelects();
                            setSelect(sel, *objs.value(sel));
                        }
                    }
                }
            }
    }
    void mouseMoveEvent(QMouseEvent *event) override {
        if (event->buttons().testFlag(Qt::LeftButton)){
            auto inv = m_transnode->matrix().inverted();
            auto pos = inv.map(event->pos());
        }
        qsgPluginTransform::mouseMoveEvent(event);
    }
    void hoverMoveEvent(QHoverEvent *event) override {
        qsgPluginTransform::hoverMoveEvent(event);
    }
    QString getName(rea::qsgBoard* aParent = nullptr) override{
        qsgPluginTransform::getName(aParent);
        updateParent([this](QSGNode* aBackground){
            m_transnode = reinterpret_cast<QSGTransformNode*>(aBackground);
        });
        return m_name;
    }
private:
    QSGTransformNode* m_transnode;
    QSet<QString> m_selects;
};

static rea::regPip<QJsonObject, rea::pipePartial> plugin_select([](rea::stream<QJsonObject>* aInput){
    aInput->out<std::shared_ptr<rea::qsgBoardPlugin>>(std::make_shared<qsgPluginEditNode>(aInput->data()));
}, rea::Json("name", "create_qsgboardplugin_select"));
