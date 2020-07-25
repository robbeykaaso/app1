#include "qsgBoard.h"
#include "reactive2.h"
#include <QSGTransformNode>
#include <QTransform>

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

    rea::pipeline::add<qsgModel>([this](rea::stream<qsgModel>* aInput){
        m_models.push_back(aInput->data());
        update();
    }, rea::Json("name", "updateQSGModel_" + m_name));
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
        lst.clearQSGObjects();
        while (m_models.size() > 1)
            m_models.pop_front();
    }
    if (m_models.size() == 1)
        m_models.front().show(m_trans_node);
    return ret;
}

void qsgBoard::keyPressEvent(QKeyEvent *event){

}

void qsgBoard::mousePressEvent(QMouseEvent *event){

}

void qsgBoard::mouseMoveEvent(QMouseEvent *event){

}

void qsgBoard::mouseReleaseEvent(QMouseEvent *event){

}

void qsgBoard::hoverMoveEvent(QHoverEvent *event){

}

void qsgBoard::wheelEvent(QWheelEvent *event){

}

static rea::regPip<QQmlApplicationEngine*> reg_imageboard([](rea::stream<QQmlApplicationEngine*>* aInput){
    qmlRegisterType<qsgBoard>("QSGBoard", 1, 0, "QSGBoard");
    aInput->out();
}, QJsonObject(), "regQML");
