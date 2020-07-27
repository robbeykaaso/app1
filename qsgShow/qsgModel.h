#ifndef REAL_FRAMEWORK_IMAGEMODEL_H_
#define REAL_FRAMEWORK_IMAGEMODEL_H_

#include <memory>
#include <QJsonObject>
#include <QSGNode>
#include <QQuickWindow>

class qsgPoint2D{
public:
    qsgPoint2D(float aX, float aY) : x(aX), y(aY){}
    float x;
    float y;
};

class qsgObject : public QJsonObject{
public:
    virtual ~qsgObject(){}
    qsgObject(const QJsonObject& aConfig) : QJsonObject(aConfig){

    }
    virtual QSGNode* createQSGNode(QQuickWindow* aWindow){
        m_window = aWindow;
        return nullptr;
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
    shapeObject(const QJsonObject& aConfig) : qsgObject(aConfig){

    }
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
    using pointList = std::vector<qsgPoint2D>;
    virtual void calcBoundBox(float* aBoundBox, const pointList& aPoints);
    void setQSGGemoetry();
    void setColor();
    double m_bound[4]; //leftbottomrighttop
    pointList m_points;
    QSGGeometryNode* m_node = nullptr;
protected:
    QJsonArray getPoints();
    int getWidth();
    QColor getColor();
};

class polyObject : public shapeObject{
public:
    polyObject(const QJsonObject& aConfig);
    QSGNode* getQSGNode() override;
private:
};

class ellipseObject : public shapeObject{
public:
    ellipseObject(const QJsonObject& aConfig);
    QSGNode* getQSGNode() override;
private:
    class l_qsgPoint3D : public qsgPoint2D{
    public:
        l_qsgPoint3D(float aX, float aY, float aZ) : qsgPoint2D(aX, aY), z(aZ){}
        float z;
        std::shared_ptr<l_qsgPoint3D> nxt = nullptr;
    };

    std::shared_ptr<l_qsgPoint3D> evalPoint(const qsgPoint2D& aCenter, const qsgPoint2D& aRadius, double aParam);
    qsgPoint2D getRadius();
    qsgPoint2D getCenter();
    double getAngle();
};

class qsgModel : public QJsonObject{
public:
    qsgModel(){}
    qsgModel(const QJsonObject& aConfig);
    void clearQSGObjects();
    void show(QSGTransformNode* aTransform);
    void zoom(int aStep, const QPointF& aCenter, double aRatio = 0);
    void move(const QPointF& aDistance);

    int getWidth();
    int getHeight();
    QTransform getTransform(bool aDeserialize = false);
private:
    void setTransform();
    std::vector<std::shared_ptr<qsgObject>> m_objects;
    QTransform m_trans;
};

#endif
