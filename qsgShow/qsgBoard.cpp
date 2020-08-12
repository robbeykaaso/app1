#include "qsgBoard.h"
#include "reactive2.h"
#include <QSGTransformNode>
#include <QTransform>

void qsgPluginTransform::wheelEvent(QWheelEvent *event){
    auto cur = event->pos();
    rea::pipeline::run<QJsonObject>("updateQSGAttr_" + getParentName(), rea::Json("key", rea::JArray("transform"),
                                                                         "type", "zoom",
                                                                         "dir", event->delta() < 0 ? - 1 : 1,
                                                                         "center", rea::JArray(cur.x(), cur.y())));
}

void qsgPluginTransform::mouseMoveEvent(QMouseEvent *event){
    if (event->buttons().testFlag(Qt::MiddleButton)){
        auto cur = event->pos();
        rea::pipeline::run<QJsonObject>("updateQSGAttr_" + getParentName(), rea::Json("key", rea::JArray("transform"),
                                                                             "type", "move",
                                                                             "del", rea::JArray(cur.x() - m_lastpos.x(), cur.y() - m_lastpos.y())));
    }
    m_lastpos = event->pos();
}

void qsgPluginTransform::hoverMoveEvent(QHoverEvent *event){
    m_lastpos = event->pos();
}

static rea::regPip<QJsonObject, rea::pipePartial> create_qsgboardplugin_transform([](rea::stream<QJsonObject>* aInput){
   aInput->out<std::shared_ptr<qsgBoardPlugin>>(std::make_shared<qsgPluginTransform>(aInput->data()));
}, rea::Json("name", "create_qsgboardplugin_transform"));

void qsgBoard::beforeDestroy(){
    if (m_models.size() > 0){
        auto lst = m_models.front();
        lst->clearQSGObjects();
    }
    for (auto i : m_plugins)
        i->beforeDestroy();
}

qsgBoard::qsgBoard(QQuickItem *parent) : QQuickItem(parent)
{
    setAcceptedMouseButtons(Qt::LeftButton | Qt::MidButton | Qt::RightButton);
    setFlag(ItemHasContents, true);
    setAcceptHoverEvents(true);
    setSmooth(true);
    setAntialiasing(true);
    //connect(, QQuickWindow::beforeSynchronizing(), this, SLOT(beforeRender()))
}

void qsgBoard::setName(const QString& aName){
    m_name = aName;
    rea::pipeline::add<std::shared_ptr<qsgModel>>([this](rea::stream<std::shared_ptr<qsgModel>>* aInput){
        m_models.push_back(aInput->data());
        update();
    }, rea::Json("name", "updateQSGModel_" + m_name));

    rea::pipeline::add<QJsonObject>([this](rea::stream<QJsonObject>* aInput){
        m_models.push_back(std::make_shared<qsgModel>(aInput->data()));
        update();
    }, rea::Json("name", "replaceQSGModel_" + m_name));

    rea::pipeline::add<QJsonObject>([this](rea::stream<QJsonObject>* aInput){
        if (m_models.size() > 0){
            m_updates.push_back(m_models.front()->updateQSGAttr(aInput->data()));
            update();
        }
    }, rea::Json("name", "updateQSGAttr_" + m_name));

    rea::pipeline::add<QJsonObject>([this](rea::stream<QJsonObject>* aInput){
        for (auto i : m_plugins)
            i->beforeDestroy();
        m_plugins.clear();
        installPlugins(aInput->data().value("ctrls").toArray());
    }, rea::Json("name", "updateQSGCtrl_" + m_name));
}

void qsgBoard::installPlugins(const QJsonArray& aPlugins){
    auto add_qsg_plugin = rea::pipeline::add<std::shared_ptr<qsgBoardPlugin>>([this](rea::stream<std::shared_ptr<qsgBoardPlugin>>* aInput){
        auto plg = aInput->data();
        m_plugins.insert(plg->getName(this), plg);
    });

    for (auto i : aPlugins){
        auto plg = i.toObject();
        auto cmd = "create_qsgboardplugin_" + plg.value("type").toString();
        auto tag = rea::Json("tag", add_qsg_plugin->actName());
        rea::pipeline::find(cmd)->next(add_qsg_plugin, tag);
        rea::pipeline::run<QJsonObject>(cmd, plg, tag);
    }
}

QSGNode* qsgBoard::updatePaintNode(QSGNode* aOldNode, UpdatePaintNodeData* nodedata){
    auto ret = aOldNode;
    if (ret == nullptr){
        m_clip_node = new QSGClipNode();
        m_clip_node->setFlag(QSGNode::OwnedByParent);
        m_clip_node->setClipRect(boundingRect());
        m_clip_node->setIsRectangular(true);
        ret = m_clip_node;

        m_trans_node = new QSGTransformNode();
        m_trans_node->setMatrix(QMatrix4x4(QTransform()));
        m_trans_node->setFlag(QSGNode::OwnedByParent);
        ret->appendChildNode(m_trans_node);
    }
    if (m_models.size() > 1){
        auto lst = m_models.front();
        lst->clearQSGObjects();
        while (m_models.size() > 1)
            m_models.pop_front();
    }
    if (m_models.size() == 1){
        m_models.front()->show(m_trans_node, window(), QPointF(width(), height()));
    }

    for (auto i : m_updates)
        if (i)
            i(m_trans_node);
    m_updates.clear();

    return ret;
}

void qsgBoard::keyPressEvent(QKeyEvent *event){
    for (auto i : m_plugins)
        i->keyPressEvent(event);
}

void qsgBoard::mousePressEvent(QMouseEvent *event){
    for (auto i : m_plugins)
        i->mousePressEvent(event);
}

void qsgBoard::mouseMoveEvent(QMouseEvent *event){
    for (auto i : m_plugins)
        i->mouseMoveEvent(event);
}

void qsgBoard::mouseReleaseEvent(QMouseEvent *event){
    for (auto i : m_plugins)
        i->mouseReleaseEvent(event);
}

void qsgBoard::hoverMoveEvent(QHoverEvent *event){
    for (auto i : m_plugins)
        i->hoverMoveEvent(event);
}

void qsgBoard::wheelEvent(QWheelEvent *event){
    for (auto i : m_plugins)
        i->wheelEvent(event);
}

static rea::regPip<QQmlApplicationEngine*> reg_imageboard([](rea::stream<QQmlApplicationEngine*>* aInput){
    qmlRegisterType<qsgBoard>("QSGBoard", 1, 0, "QSGBoard");
    aInput->out();
}, QJsonObject(), "regQML");
