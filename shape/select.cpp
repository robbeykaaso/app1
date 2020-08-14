#include "qsgBoard.h"
#include "util/cv.h"
#include "imagePool.h"
#include "command.h"
#include <QPainter>

class shapeOperation{
public:
    shapeOperation(const QString& aBoard, const QPointF& aStart, const QHash<QString, std::shared_ptr<rea::qsgObject>>& aObjects){
        m_objects = aObjects;
        m_last = aStart;
        m_board = aBoard;
    }
    virtual ~shapeOperation(){

    }
    virtual void mouseMoveEvent(const QPointF& aPos){
        m_last = aPos;
    }
protected:
    QHash<QString, std::shared_ptr<rea::qsgObject>> m_objects;
    QPointF m_last;
    QString m_board;
};

class shapeMoveOperation : public shapeOperation{
public:
    shapeMoveOperation(const QString& aBoard, const QPointF& aStart, const QHash<QString, std::shared_ptr<rea::qsgObject>>& aObjects) : shapeOperation(aBoard, aStart, aObjects){
        m_first = aStart;

        auto arr2pts = [](const QJsonArray& aArray){
            rea::pointList pts;
            for (int j = 0; j < aArray.size(); j += 2)
                pts.push_back(QPointF(aArray[j].toDouble(), aArray[j + 1].toDouble()));
            return pts;
        };

        for (auto i : m_objects.keys()){
            auto obj = m_objects.value(i);
            std::vector<rea::pointList> ptslst;
            if (obj->value("type") == "poly"){
                auto lst = obj->value("points").toArray();
                for (auto j : lst)
                    ptslst.push_back(arr2pts(j.toArray()));
            }else{
                ptslst.push_back(arr2pts(obj->value("center").toArray()));
            }
            m_points.push_back(ptslst);
        }
    }
    ~shapeMoveOperation() override{
        auto brd = m_board;
        auto redo = getModification(m_last - m_first);
        auto undo = getModification(QPointF(0, 0));
        rea::pipeline::run<rea::ICommand>("addCommand",
                                          rea::ICommand([brd, redo](){
                                              for (auto i : redo)
                                                  rea::pipeline::run("updateQSGAttr_" + brd, i.toObject());
                                          }, [brd, undo](){
                                              for (auto i : undo)
                                                  rea::pipeline::run("updateQSGAttr_" + brd, i.toObject());
                                          }));
    }
    void mouseMoveEvent(const QPointF& aPos) override{
        shapeOperation::mouseMoveEvent(aPos);
        auto mdy = getModification(aPos - m_first);
        for (auto i : mdy)
            rea::pipeline::run("updateQSGAttr_" + m_board, i.toObject());
    }
private:
    QJsonArray getModification(const QPointF& aDel){
        QJsonArray ret;
        auto lst = m_points.begin();
        for (auto i : m_objects.keys()){
            QJsonArray val;

            if (m_objects.value(i)->value("type") == "poly"){
                for (auto j : *lst){
                    QJsonArray lst;
                    for (auto k : j){
                        lst.push_back(k.x() + aDel.x());
                        lst.push_back(k.y() + aDel.y());
                    }
                    val.push_back(lst);
                }
            }else{
                for (auto j : *lst->begin()){
                    val.push_back(j.x() + aDel.x());
                    val.push_back(j.y() + aDel.y());
                }
            }
            ret.push_back(rea::Json("obj", i,
                                    "key", m_objects.value(i)->value("type") == "poly" ? rea::JArray("points") : rea::JArray("center"),
                                    "val", val));
            lst++;
        }
        return ret;
    }
    QPointF m_first;
    std::vector<std::vector<rea::pointList>> m_points;
};

class qsgPluginSelect : public rea::qsgPluginTransform{
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
                QHash<QString, std::shared_ptr<rea::qsgObject>> lst;
                auto mdl = getQSGModel();
                auto objs = mdl->getQSGObjects();
                for (auto i : m_selects)
                    lst.insert(i, objs.value(i));
                m_shape_operation = std::make_shared<shapeMoveOperation>(getParentName(), pos, lst);
            }
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
    std::shared_ptr<shapeOperation> m_shape_operation = nullptr;
};

static rea::regPip<QJsonObject, rea::pipePartial> plugin_select([](rea::stream<QJsonObject>* aInput){
    aInput->out<std::shared_ptr<rea::qsgBoardPlugin>>(std::make_shared<qsgPluginSelect>(aInput->data()));
}, rea::Json("name", "create_qsgboardplugin_select"));
