#ifndef REAL_FRAMEWORK_IMAGEMODEL_H_
#define REAL_FRAMEWORK_IMAGEMODEL_H_

#include "reactive2.h"
#include <memory>
#include <QJsonObject>
#include <QSGNode>
#include <QQuickWindow>
#include <QSGSimpleTextureNode>

using pointList = std::vector<QPointF>;
using IUpdateQSGAttr = std::function<void(void)>;

QRectF calcBoundBox(const pointList& aPoints);

class qsgModel;

class qsgObject : public QJsonObject{
public:
    virtual ~qsgObject(){}
    qsgObject(const QJsonObject& aConfig) : QJsonObject(aConfig){

    }
    virtual std::vector<QSGNode*> getQSGNodes(QQuickWindow* aWindow = nullptr, qsgModel* aParent = nullptr, QSGNode* aTransformNode = nullptr){
        if (aWindow)
            m_window = aWindow;
        if (aParent)
            m_parent = aParent;
        return std::vector<QSGNode*>();
    }
    virtual void removeQSGNodes() {
        auto nds = getQSGNodes();
        for (auto i : nds){
            i->parent()->removeChildNode(i);
            delete i;
        }
    }
    virtual void transformChanged() {}
    virtual IUpdateQSGAttr updateQSGAttr(const QString& aModification){return nullptr;}
protected:
    QQuickWindow* m_window;
    qsgModel* m_parent;
};

class imageObject : public qsgObject{
public:
    imageObject(const QJsonObject& aConfig) : qsgObject(aConfig){

    }
    std::vector<QSGNode*> getQSGNodes(QQuickWindow* aWindow = nullptr, qsgModel* aParent = nullptr, QSGNode* aTransformNode = nullptr) override;
    void removeQSGNodes() override {
        qsgObject::removeQSGNodes();
        m_node = nullptr;
    }
    IUpdateQSGAttr updateQSGAttr(const QString& aModification) override;
private:
    void updateImagePath();
    QString getPath();
    QRectF getRange(const QImage& aImage);
    QSGSimpleTextureNode* m_node = nullptr;
};

class shapeObject : public qsgObject{
public:
    shapeObject(const QJsonObject& aConfig) : qsgObject(aConfig){

    }
    QRectF getBoundBox() {return m_bound;}
    virtual bool canBePickedUp(int aX, int aY);
    std::vector<QSGNode*> getQSGNodes(QQuickWindow* aWindow = nullptr, qsgModel* aParent = nullptr, QSGNode* aTransformNode = nullptr) override;
    void removeQSGNodes() override;
    void transformChanged() override;
    IUpdateQSGAttr updateQSGAttr(const QString& aModification) override;
protected:
    void setQSGGemoetry(const pointList& aPointList, QSGGeometryNode& aNode, unsigned int aMode, std::vector<uint32_t>* aIndecies = nullptr);
    void setQSGColor(QSGGeometryNode& aNode, const QColor& aColor);
    void checkArrowVisible(int aCount);
    void checkFaceOpacity();
    void checkTextVisible();
    void checkColor();
    void checkCaption();
    void checkWidth();
    void checkAngle();
    virtual void updateGeometry(){}
    virtual void updateArrowLocation(){}
    void calcArrow(const QPointF& aStart, const QPointF& aEnd, QSGGeometryNode& aNode);
    void updateQSGFace(QSGGeometryNode& aNode, int aOpacity);
    void updateTextLocation(const QJsonObject& aTextConfig);
    QRectF m_bound = QRectF(0, 0, 0, 0); //leftbottomrighttop
    std::vector<pointList> m_points;
    QSGGeometryNode* m_outline = nullptr;
    QSGGeometryNode* m_face = nullptr;
    std::vector<QSGGeometryNode*> m_holes;
    std::vector<QSGGeometryNode*> m_arrows;
    QSGSimpleTextureNode* m_text = nullptr;
private:
    void updateQSGFaceColor(QSGGeometryNode& aNode, int aOpacity);
    void updateTextValue(const QJsonObject& aTextConfig);
    void updateArrowCount(int aCount);
protected:
    int getWidth();
    QColor getColor();
    QString getText();
    int getFaceOpacity();
    QJsonObject getTextConfig();
    QJsonObject getArrowConfig();
    bool getArrowVisible(const QJsonObject& aConfig);
    bool getPoleArrow(const QJsonObject& aConfig);
    double getAngle();
};

class polyObject : public shapeObject{
public:
    polyObject(const QJsonObject& aConfig);
    std::vector<QSGNode*> getQSGNodes(QQuickWindow* aWindow = nullptr, qsgModel* aParent = nullptr, QSGNode* aTransformNode = nullptr) override;
    void transformChanged() override;
    IUpdateQSGAttr updateQSGAttr(const QString& aModification) override;
protected:
    QJsonArray getPoints();
    void updateGeometry() override;
    void updateArrowLocation() override;
    void checkArrowPole();
    void checkGeometry();
private:
};

class ellipseObject : public shapeObject{
public:
    ellipseObject(const QJsonObject& aConfig);
    std::vector<QSGNode*> getQSGNodes(QQuickWindow* aWindow = nullptr, qsgModel* aParent = nullptr, QSGNode* aTransformNode = nullptr) override;
    void transformChanged() override;
    IUpdateQSGAttr updateQSGAttr(const QString& aModification) override;
protected:
    void updateGeometry() override;
    void updateArrowLocation() override;
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
};

class qsgModel : public QJsonObject{
public:
    qsgModel(){}
    qsgModel(const QJsonObject& aConfig);
    ~qsgModel();

    void clearQSGObjects();
    void show(QSGTransformNode* aTransform, QQuickWindow* aWindow, const QPointF& aSize);
    IUpdateQSGAttr updateQSGAttr(const QJsonObject& aModification);
private:
    QString overwriteAttr(QJsonObject& aObject, const QJsonArray& aKeys, const QJsonValue&& aValue);
    void zoom(int aStep, const QPointF& aCenter, double aRatio = 0);
    void move(const QPointF& aDistance);
    void WCS2SCS();
    void addObject(const QJsonObject& aConfig);

    QTransform getTransform(bool aDeserialize = false);
    int getWidth();
    int getHeight();
    QJsonObject getArrowConfig();
    bool getArrowVisible(const QJsonObject& aConfig);
    bool getPoleArrow(const QJsonObject& aConfig);
    int getFaceOpacity();
    QJsonObject getTextConfig();
    bool getTextVisible(const QJsonObject& aConfig);
    QPoint getTextSize(const QJsonObject& aConfig);
    QString getTextLocation(const QJsonObject& aConfig);
    void setTransform();
    QMap<QString, std::shared_ptr<qsgObject>> m_objects;
    QTransform m_trans;
    QPointF m_size;
private:
    rea::pipe0* objectCreator(const QString& aName);
    rea::pipe0* m_add_object;
    QHash<QString, rea::pipe0*> m_creators;
    QQuickWindow* m_window = nullptr;
    QSGTransformNode* m_trans_node = nullptr;
    friend qsgObject;
    friend shapeObject;
    friend polyObject;
    friend imageObject;
};

#endif
