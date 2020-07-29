#ifndef REAL_FRAMEWORK_IMAGEMODEL_H_
#define REAL_FRAMEWORK_IMAGEMODEL_H_

#include <memory>
#include <QJsonObject>
#include <QSGNode>
#include <QQuickWindow>
#include <QSGSimpleTextureNode>

using pointList = std::vector<QPointF>;

QRectF calcBoundBox(const pointList& aPoints);

class qsgModel;

class qsgObject : public QJsonObject{
public:
    virtual ~qsgObject(){}
    qsgObject(const QJsonObject& aConfig) : QJsonObject(aConfig){

    }
    virtual QSGNode* getQSGNode(QQuickWindow* aWindow = nullptr, qsgModel* aParent = nullptr){
        if (aWindow)
            m_window = aWindow;
        if (aParent)
            m_parent = aParent;
        return nullptr;
    }
    virtual void addQSGNode(QSGNode* aParent = nullptr) {
        auto nd = getQSGNode();
        if (nd){
            if (aParent)
                aParent->appendChildNode(nd);
            else
                nd->parent()->removeChildNode(nd);
        }
    }
    virtual void transformChanged() {}
protected:
    QQuickWindow* m_window;
    qsgModel* m_parent;
};

class imageObject : public qsgObject{
public:
    imageObject(const QJsonObject& aConfig) : qsgObject(aConfig){

    }
    QSGNode* getQSGNode(QQuickWindow* aWindow = nullptr, qsgModel* aParent = nullptr) override;
    void addQSGNode(QSGNode* aParent = nullptr) override {
        qsgObject::addQSGNode(aParent);
        if (!aParent)
            m_node = nullptr;
    }
private:
    QString getPath();
    QSGSimpleTextureNode* m_node = nullptr;
};

class shapeObject : public qsgObject{
public:
    shapeObject(const QJsonObject& aConfig) : qsgObject(aConfig){

    }
    QRectF getBoundBox() {return m_bound;}
    //virtual void transform(const QJsonObject& aTransform);
    virtual bool canBePickedUp(int aX, int aY);
    QSGNode* getQSGNode(QQuickWindow* aWindow = nullptr, qsgModel* aParent = nullptr) override;
    void addQSGNode(QSGNode* aParent = nullptr) override;
    void transformChanged() override;
protected:
    void setQSGGemoetry(const pointList& aPointList, QSGGeometryNode& aNode, unsigned int aMode, std::vector<uint32_t>* aIndecies = nullptr);
    void setColor(QSGGeometryNode& aNode);
    void tryAppendArrowNodes(int aCount);
    void calcArrow(const QPointF& aStart, const QPointF& aEnd, QSGGeometryNode& aNode);
    QRectF m_bound = QRectF(0, 0, 0, 0); //leftbottomrighttop
    pointList m_points;
    QSGGeometryNode* m_node = nullptr;
    std::vector<QSGGeometryNode*> m_arrows;
    QSGSimpleTextureNode* m_text = nullptr;
private:
    void updateTextValue(const QJsonObject& aTextConfig);
    void updateTextLocation(const QJsonObject& aTextConfig, const QSGTransformNode* aTransform = nullptr);
protected:
    QJsonArray getPoints();
    int getWidth();
    QColor getColor();
    QString getText();
    int getFaceOpacity();
    QJsonObject getTextConfig();
    QJsonObject getArrowConfig();
    bool getShowArrow(const QJsonObject& aConfig);
};

class polyObject : public shapeObject{
public:
    polyObject(const QJsonObject& aConfig);
    QSGNode* getQSGNode(QQuickWindow* aWindow = nullptr, qsgModel* aParent = nullptr) override;
    void addQSGNode(QSGNode* aParent = nullptr) override;
    void transformChanged() override;
private:
};

class ellipseObject : public shapeObject{
public:
    ellipseObject(const QJsonObject& aConfig);
    QSGNode* getQSGNode(QQuickWindow* aWindow = nullptr, qsgModel* aParent = nullptr) override;
    void addQSGNode(QSGNode* aParent = nullptr) override;
    void transformChanged() override;
private:
    class l_qsgPoint3D : public QPointF{
    public:
        l_qsgPoint3D(float aX, float aY, float aZ) : QPointF(aX, aY), z(aZ){}
        float z;
        std::shared_ptr<l_qsgPoint3D> nxt = nullptr;
    };

    std::shared_ptr<l_qsgPoint3D> evalPoint(const QPointF& aCenter, const QPointF& aRadius, double aParam);
    bool getCCW();
    QPointF getRadius();
    QPointF getCenter();
    double getAngle();
};

class qsgModel : public QJsonObject{
public:
    qsgModel(){}
    qsgModel(const QJsonObject& aConfig, QMap<QString, QImage> aImages);
    void clearQSGObjects();
    void show(QSGNode* aTransform, QQuickWindow* aWindow);
    void zoom(int aStep, const QPointF& aCenter, double aRatio = 0);
    void move(const QPointF& aDistance);
    void transformChanged();

    int getWidth();
    int getHeight();
    QTransform getTransform(bool aDeserialize = false);
private:
    QJsonObject getArrowConfig();
    bool getShowArrow(const QJsonObject& aConfig);
    int getFaceOpacity();
    QJsonObject getTextConfig();
    bool getTextVisible(const QJsonObject& aConfig);
    QPoint getTextSize(const QJsonObject& aConfig);
    QString getTextLocation(const QJsonObject& aConfig);
    void setTransform();
    QMap<QString, std::shared_ptr<qsgObject>> m_objects;
    QTransform m_trans;
private:
    QMap<QString, QImage> m_images;
    friend shapeObject;
    friend imageObject;
};

#endif
