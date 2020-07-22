#ifndef REAL_FRAMEWORK_IMAGEMODEL_H_
#define REAL_FRAMEWORK_IMAGEMODEL_H_

#include <memory>
#include <QJsonObject>
#include <QSGNode>
#include <QQuickWindow>

class qsgObject : public QJsonObject{
public:
    virtual ~qsgObject(){}
    qsgObject(const QJsonObject& aConfig) : QJsonObject(aConfig){

    }
    virtual QSGNode* createQSGNode(QQuickWindow* aWindow){
        m_window = aWindow;
    }
    virtual QSGNode* getQSGNode() = 0;
    virtual void QSGNodeRemoved() {
        auto nd = getQSGNode();
        if (nd)
            nd->parent()->removeChildNode(nd);
    }
private:
    QQuickWindow* m_window;
};

class imageObject : public qsgObject{
private:

};

class shapeObject : public qsgObject{
public:
    virtual QRect getBoundBox() {return QRect(m_bound[0], m_bound[1], m_bound[2] - m_bound[0], m_bound[3] - m_bound[1]);}
    //virtual void transform(const QJsonObject& aTransform);
    virtual bool canBePickedUp(int aX, int aY);
    QSGNode* getQSGNode() override {
        return m_node;
    }
    void QSGNodeRemoved() override {
        qsgObject::QSGNodeRemoved();
        m_node = nullptr;
    }
protected:
    using pointList = std::vector<QPoint>;
    virtual void calcBoundBox(double* aBoundBox, const pointList& aPoints);
    double m_bound[4]; //leftbottomrighttop
    pointList m_points;
    QSGGeometryNode* m_node = nullptr;

};

class qsgModel : public QJsonObject{
public:
    qsgModel() : QJsonObject(){

    }
    void clearQSGObjects();
    void show(QSGNode& aTransform);
private:
    std::vector<std::shared_ptr<qsgObject>> m_objects;
};

#endif
