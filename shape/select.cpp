#include "qsgShow/qsgBoard.h"
#include "util/cv.h"
#include "imagePool.h"
#include "command.h"
#include <QPainter>

class shapeOperation{
public:
    shapeOperation(const QPointF& aStart, std::vector<std::shared_ptr<qsgObject>> aObjects){
        m_last = aStart;
        m_objects = aObjects;
    }
    virtual ~shapeOperation(){

    }
    virtual void mouseMoveEvent(const QPointF& aPos){
        m_last = aPos;
    }
protected:
    QPointF m_last;
    std::vector<std::shared_ptr<qsgObject>> m_objects;
};

class shapeMoveOperation : public shapeOperation{
public:
    shapeMoveOperation(const QPointF& aStart, std::vector<std::shared_ptr<qsgObject>> aObjects) : shapeOperation(aStart, aObjects){

    }
    ~shapeMoveOperation() override{

    }
    void mouseMoveEvent(const QPointF& aPos) override{

    }
};

class qsgPluginSelect : public qsgPluginTransform{
public:
    qsgPluginSelect(const QJsonObject& aConfig) : qsgPluginTransform(aConfig){

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
    void beforeDestroy() override{

    }
    void mousePressEvent(QMouseEvent *event) override {

    }
    void mouseReleaseEvent(QMouseEvent *event) override {
        if (!m_shape_operation){
            if (event->button() == Qt::LeftButton){
                auto mdl = getQSGModel();
                if (mdl){
                    QString sel = "";
                    auto objs = mdl->getQSGObjects();
                    auto inv = m_transnode->matrix().inverted();
                    auto pos = inv.map(event->pos());
                    for (auto i : objs.keys()){
                        auto obj = objs.value(i);
                        if (obj->value("type") != "image" && obj->bePointSelected(pos.x(), pos.y())){
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
        }else{
            m_shape_operation = nullptr;
        }
    }
    void mouseMoveEvent(QMouseEvent *event) override {
        if (event->buttons().testFlag(Qt::LeftButton)){
            auto inv = m_transnode->matrix().inverted();
            auto pos = inv.map(event->pos());
            if (m_shape_operation)
                m_shape_operation->mouseMoveEvent(pos);
            else if (m_selects.size() > 0 && m_lastpos != event->pos()){
                std::vector<std::shared_ptr<qsgObject>> lst;
                auto mdl = getQSGModel();
                auto objs = mdl->getQSGObjects();
                for (auto i : m_selects)
                    lst.push_back(objs.value(i));
                m_shape_operation = std::make_shared<shapeMoveOperation>(pos, lst);
            }
        }
        qsgPluginTransform::mouseMoveEvent(event);
    }
    void hoverMoveEvent(QHoverEvent *event) override {
        qsgPluginTransform::hoverMoveEvent(event);
    }
    QString getName(qsgBoard* aParent = nullptr) override{
        qsgPluginTransform::getName(aParent);
        updateParent([this](QSGNode* aBackground){
            m_transnode = reinterpret_cast<QSGTransformNode*>(aBackground);
        });
        return m_name;
    }
private:
    QSGTransformNode* m_transnode;
    QSet<QString> m_selects;
    std::shared_ptr<shapeOperation> m_shape_operation = nullptr;
};

static rea::regPip<QJsonObject, rea::pipePartial> plugin_select([](rea::stream<QJsonObject>* aInput){
    aInput->out<std::shared_ptr<qsgBoardPlugin>>(std::make_shared<qsgPluginSelect>(aInput->data()));
}, rea::Json("name", "create_qsgboardplugin_select"));
