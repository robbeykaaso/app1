#include "qsgBoard.h"
#include "reactive2.h"
#include <QSGTransformNode>
#include <QTransform>

void qsgPluginTransform::updateQSGModel(std::shared_ptr<qsgModel> aModel){
    m_qsgmodel = aModel;
    calcTransform();
}

void qsgPluginTransform::wheelEvent(QWheelEvent *event){
    auto cur = event->pos();

    if (m_qsgmodel){
        if (event->delta() < 0)
            m_qsgmodel->zoom(- 1, QPointF(cur.x(), cur.y()));
        else
            m_qsgmodel->zoom(1, QPointF(cur.x(), cur.y()));
        calcTransform();
        update();
    }
}

void qsgPluginTransform::mouseMoveEvent(QMouseEvent *event){
    if (m_qsgmodel)
        if (event->buttons().testFlag(Qt::MiddleButton)){
            auto cur = event->pos();
            m_qsgmodel->move(QPointF(cur.x() - m_lastpos.x(), cur.y() - m_lastpos.y()));
            calcTransform();
            update();
        }
    m_lastpos = event->pos();
}

void qsgPluginTransform::hoverMoveEvent(QHoverEvent *event){
    m_lastpos = event->pos();
}

void qsgPluginTransform::updatePaintNode(QSGNode* aBackground){
    if (!aBackground)
        return;
    auto trans_node = reinterpret_cast<QSGTransformNode*>(aBackground);
    trans_node->setMatrix(QMatrix4x4(m_trans));
    trans_node->markDirty(QSGNode::DirtyMatrix);
}

void qsgPluginTransform::calcTransform(){
    if (!m_qsgmodel){
        m_trans = QTransform();
        return;
    }
    auto width = m_qsgmodel->getWidth(),
         height = m_qsgmodel->getHeight();
    if (width == 0)
        width = int(m_parent->width());
    if (height == 0)
        height = int(m_parent->height());

    QTransform trans0;
    trans0.scale(m_parent->width() * 1.0 / width, m_parent->height() * 1.0 / height);
    // return aTransform;
    auto ratio =  width * 1.0 / height;
    QTransform trans;
    if (ratio > m_parent->width() * 1.0 / m_parent->height()){
        auto ry = m_parent->width() / ratio / m_parent->height();
        trans = trans.scale(1, ry);
        trans = trans.translate(0, (m_parent->height() - m_parent->width() / ratio) * 0.5 / ry);
        //m_image->setRect(QRect(0, (m_ui_height - m_ui_width / ratio) * 0.5, m_ui_width, m_ui_width / ratio));
    }else{
        auto rx = (m_parent->height() * ratio) / m_parent->width();
        trans = trans.scale(rx, 1);
        trans = trans.translate((m_parent->width() - m_parent->height() * ratio) * 0.5 / rx, 0);
        //m_image->setRect(QRect((m_ui_width - m_ui_height * ratio) * 0.5, 0, m_ui_height * ratio, m_ui_height));
    }
    m_trans = trans0 * trans * m_qsgmodel->getTransform();
}

static rea::regPip<QJsonObject, rea::pipePartial> create_ellipse([](rea::stream<QJsonObject>* aInput){
   aInput->out<std::shared_ptr<qsgBoardPlugin>>(std::make_shared<qsgPluginTransform>(aInput->data()));
}, rea::Json("name", "create_qsgboardplugin_transform"));

void qsgBoard::beforeDestroy(){

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
        for (auto i : m_plugins){
            i->updateQSGModel(aInput->data());
            m_updates.push_back(i.get());
        }
        m_refreshed = false;
        update();
    }, rea::Json("name", "updateQSGModel_" + m_name));
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
        auto trans_node = new QSGTransformNode();
        trans_node->setMatrix(QMatrix4x4(QTransform()));
        trans_node->setFlag(QSGNode::OwnedByParent);
        ret = trans_node;
        m_trans_node = trans_node;
    }
    if (m_models.size() > 1){
        auto lst = m_models.front();
        lst->clearQSGObjects();
        while (m_models.size() > 1)
            m_models.pop_front();
    }
    if (!m_refreshed && m_models.size() == 1){
        m_models.front()->show(m_trans_node, window());
        m_refreshed = true;
    }

    for (auto i : m_updates)
        i->updatePaintNode(ret);
    m_updates.clear();
    return ret;
}

void qsgBoard::keyPressEvent(QKeyEvent *event){

}

void qsgBoard::mousePressEvent(QMouseEvent *event){

}

void qsgBoard::mouseMoveEvent(QMouseEvent *event){
    for (auto i : m_plugins)
        i->mouseMoveEvent(event);
}

void qsgBoard::mouseReleaseEvent(QMouseEvent *event){

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
