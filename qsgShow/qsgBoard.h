#ifndef REAL_FRAMEWORK_IMAGEBOARD_H_
#define REAL_FRAMEWORK_IMAGEBOARD_H_

#include "qsgModel.h"
#include "util.h"
#include <QQuickItem>
#include <QQueue>


class qsgBoardPlugin;

class qsgBoard : public QQuickItem{
    Q_OBJECT
    Q_PROPERTY(QString name WRITE setName READ getName)
    Q_PROPERTY(QJsonArray plugins WRITE installPlugins)
public:
    Q_INVOKABLE void beforeDestroy();
public:
    explicit qsgBoard(QQuickItem *parent = nullptr);
protected:
    QSGNode *updatePaintNode(QSGNode* aOldNode, UpdatePaintNodeData* nodedata) override;
    void keyPressEvent(QKeyEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void hoverMoveEvent(QHoverEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    QSGClipNode* m_clip_node = nullptr;
    QSGTransformNode* m_trans_node = nullptr;
    QQueue<std::shared_ptr<qsgModel>> m_models;
private:
    void setName(const QString& aName);
    QString getName() {return m_name;}
    void installPlugins(const QJsonArray& aPlugins);
    QString m_name;
    QMap<QString, std::shared_ptr<qsgBoardPlugin>> m_plugins;
    QQueue<qsgBoardPlugin*> m_updates;
    QQueue<IUpdateQSGAttr> m_updates2;
    bool m_refreshed = false;
    friend qsgBoardPlugin;
};

class qsgBoardPlugin{
public:
    qsgBoardPlugin(const QJsonObject& aConfig) {
        m_name = aConfig.value("name").toString();
        if (m_name == "")
            m_name = rea::generateUUID();
    }
    virtual ~qsgBoardPlugin() = default;

private:
    friend qsgBoard;
protected:
    virtual QString getName(qsgBoard* aParent = nullptr) {
        if (aParent)
            m_parent = aParent;
        return m_name;
    }
    virtual void updateQSGModel(std::shared_ptr<qsgModel> aModel){}
    virtual void wheelEvent(QWheelEvent *event) = 0;
    virtual void mouseMoveEvent(QMouseEvent *event) = 0;
    virtual void hoverMoveEvent(QHoverEvent *event) = 0;
    virtual void updatePaintNode(QSGNode* aBackground) = 0;
    void update(){
        if (m_parent)
            m_parent->m_updates.push_back(this);
        m_parent->update();
    }

    qsgBoard* m_parent;
    QString m_name;
};

class qsgPluginTransform : public qsgBoardPlugin{
public:
    qsgPluginTransform(const QJsonObject& aConfig) : qsgBoardPlugin(aConfig){}
protected:
    void updateQSGModel(std::shared_ptr<qsgModel> aModel) override;
    void wheelEvent(QWheelEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void hoverMoveEvent(QHoverEvent *event) override;
    void updatePaintNode(QSGNode* aBackground) override;
private:
    void calcTransform();
    std::shared_ptr<qsgModel> m_qsgmodel = nullptr;
    QTransform m_trans;
    QPoint m_lastpos;
};

#endif
