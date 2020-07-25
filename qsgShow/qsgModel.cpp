#include "qsgModel.h"
#include "reactive2.h"
#include <QJsonArray>
#include <QSGFlatColorMaterial>

bool shapeObject::canBePickedUp(int aX, int aY){
    auto bnd = getBoundBox();
    return aX >= bnd.left() && aX <= bnd.right() && aY >= bnd.top() && aY <= bnd.bottom();
}

void shapeObject::calcBoundBox(float *aBoundBox, const pointList &aPoints){
    aBoundBox[0] = aBoundBox[2] = aPoints[0].x;
    aBoundBox[1] = aBoundBox[3] = aPoints[0].y;
    for (auto i : aPoints){
        auto x = i.x, y = i.y;
        if (x < aBoundBox[0])
            aBoundBox[0] = x;
        if (x > aBoundBox[2])
            aBoundBox[2] = x;
        if (y < aBoundBox[1])
            aBoundBox[1] = y;
        if (y > aBoundBox[3])
            aBoundBox[3] = y;
    }
}

QJsonArray shapeObject::getPoints(){
    return value("points").toArray();
}

int shapeObject::getWidth(){
    return value("width").toInt(5);
}

QColor shapeObject::getColor(){
    return QColor(value("color").toString("red"));
}

static rea::regPip<std::shared_ptr<ICreateQSGObject>> create_poly([](rea::stream<std::shared_ptr<ICreateQSGObject>>* aInput){
    auto dt = aInput->data();
    dt->setObject(std::make_shared<polyObject>(*dt.get()));
    aInput->setData(dt);
}, rea::Json("name", "create_poly"));

polyObject::polyObject(const QJsonObject& aConfig) : shapeObject(aConfig){

}

QSGNode* polyObject::getQSGNode(QSGTransformNode* aTransform) {
    if (!m_node){
        m_node = new QSGGeometryNode();
        auto pts = getPoints();
        for (int i = 0; i < pts.size(); i += 2)
            m_points.push_back(qsgPoint2D(pts[i].toDouble(), pts[i + 1].toDouble()));

        QSGGeometry *geometry = new QSGGeometry(QSGGeometry::defaultAttributes_Point2D(), int(m_points.size() / 2));
        auto vertices = geometry->vertexDataAsPoint2D();
        for (size_t i = 0; i < m_points.size(); ++i)
            vertices[i].set(m_points[i].x, m_points[i].y);
        geometry->setLineWidth(getWidth());

        geometry->setDrawingMode(QSGGeometry::DrawLineStrip);
        m_node->setGeometry(geometry);
        m_node->setFlag(QSGNode::OwnsGeometry);
        m_node->markDirty(QSGNode::DirtyGeometry);

        QSGFlatColorMaterial *material = new QSGFlatColorMaterial();
        material->setColor(getColor());
        m_node->setMaterial(material);
        m_node->setFlag(QSGNode::OwnsMaterial);
    }
    return m_node;
}

ellipseObject::ellipseObject(const QJsonObject& aConfig) : shapeObject(aConfig){

}

static rea::regPip<std::shared_ptr<ICreateQSGObject>> create_ellipse([](rea::stream<std::shared_ptr<ICreateQSGObject>>* aInput){
    auto dt = aInput->data();
    dt->setObject(std::make_shared<ellipseObject>(*dt.get()));
    aInput->setData(dt);
}, rea::Json("name", "create_ellipse"));

void qsgModel::clearQSGObjects(){
    for (auto i : m_objects)
        i->QSGNodeRemoved();
}

void qsgModel::show(QSGTransformNode* aTransform){
    for (auto i : m_objects)
        aTransform->appendChildNode(i->getQSGNode(aTransform));
}

qsgModel::qsgModel(const QJsonObject& aConfig) : QJsonObject(aConfig){
    auto shps = value("objects").toArray();
    for (auto i : shps){
        auto crt = std::make_shared<ICreateQSGObject>(i.toObject());
        rea::pipeline::run<std::shared_ptr<ICreateQSGObject>>("create_" + crt->value("type").toString(), crt);
        auto obj = crt->getObject();
        if (obj)
            m_objects.push_back(obj);
    }
}

static rea::regPip<int> unit_test([](rea::stream<int>* aInput){
    rea::pipeline::add<QJsonObject>([](rea::stream<QJsonObject>* aInput){
        qsgModel mdl(aInput->data());
        aInput->out<qsgModel>(mdl, "updateQSGModel_testbrd");
    }, rea::Json("name", "testQSGShow"))->next("updateQSGModel_testbrd");
}, QJsonObject(), "unitTest");
