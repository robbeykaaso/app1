#include "qsgModel.h"
#include "reactive2.h"
#include <queue>
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

void shapeObject::setQSGGemoetry(){
    QSGGeometry *geometry = new QSGGeometry(QSGGeometry::defaultAttributes_Point2D(), int(m_points.size()));
    auto vertices = geometry->vertexDataAsPoint2D();
    for (size_t i = 0; i < m_points.size(); ++i)
        vertices[i].set(m_points[i].x, m_points[i].y);
    geometry->setLineWidth(getWidth());
    geometry->setDrawingMode(QSGGeometry::DrawLineStrip);
    m_node->setGeometry(geometry);
    m_node->setFlag(QSGNode::OwnsGeometry);
    m_node->markDirty(QSGNode::DirtyGeometry);
}

void shapeObject::setColor(){
    QSGFlatColorMaterial *material = new QSGFlatColorMaterial();
    material->setColor(getColor());
    m_node->setMaterial(material);
    m_node->setFlag(QSGNode::OwnsMaterial);
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

static rea::regPip<QJsonObject, rea::pipePartial> create_poly([](rea::stream<QJsonObject>* aInput){
    aInput->out<std::shared_ptr<qsgObject>>(std::make_shared<polyObject>(aInput->data()));
}, rea::Json("name", "create_qsgobject_poly"));

polyObject::polyObject(const QJsonObject& aConfig) : shapeObject(aConfig){

}

QSGNode* polyObject::getQSGNode(QQuickWindow* aWindow) {
    if (!m_node){
        auto pts = getPoints();
        for (int i = 0; i < pts.size(); i += 2)
            m_points.push_back(qsgPoint2D(pts[i].toDouble(), pts[i + 1].toDouble()));
        m_node = new QSGGeometryNode();
        setQSGGemoetry();
        setColor();
    }
    return shapeObject::getQSGNode(aWindow);
}

ellipseObject::ellipseObject(const QJsonObject& aConfig) : shapeObject(aConfig){

}

float squarePointProjectToLine2D(const qsgPoint2D& aPoint, const qsgPoint2D& aStart, const qsgPoint2D& aEnd){
    qsgPoint2D s(aPoint.x - aStart.x, aPoint.y - aStart.y), e = qsgPoint2D(aEnd.x - aStart.x, aEnd.y - aStart.y);
    auto dot = s.x * e.x + s.y * e.y;
    return dot * dot / (e.x * e.x + e.y * e.y);
}

std::shared_ptr<ellipseObject::l_qsgPoint3D> ellipseObject::evalPoint(const qsgPoint2D& aCenter, const qsgPoint2D& aRadius, double aParam){
    return std::make_shared<l_qsgPoint3D>(aCenter.x + (aRadius.x * std::cos(aParam)),
                      aCenter.y + (aRadius.y * std::sin(aParam)),
                      aParam);
}

QSGNode* ellipseObject::getQSGNode(QQuickWindow* aWindow){
    if (!m_node){
        static const double PI = 3.14159265;

        //auto mtx = aTransform->matrix().inverted();
        //auto pt = mtx.map(QPoint(0, 0)) - mtx.map(QPoint(1, 0));
        //auto del = QPoint::dotProduct(pt, pt) * 8;
        auto del = 10;

        auto ct = getCenter();
        auto r = getRadius();

        QTransform trans;
        trans.rotate(getAngle());
        auto i0 = evalPoint(ct, r, 0), i1 = evalPoint(ct, r, PI);
        i0->nxt = i1;
        i1->nxt = evalPoint(ct, r, 2 * PI);
        std::queue<std::shared_ptr<l_qsgPoint3D>> candidates;
        candidates.push(i0);
        candidates.push(i1);
        while (candidates.size() > 0){
            auto cur = candidates.front();
            auto mid = evalPoint(ct, r, 0.5 * (cur->z + cur->nxt->z));
            if (squarePointProjectToLine2D(*mid, *cur, *cur->nxt) > del){
                mid->nxt = cur->nxt;
                cur->nxt = mid;
                candidates.push(mid);
            }else
                candidates.pop();
        }



        auto st = i0;
        while (st){
            auto pt1 = qsgPoint2D(st->x - ct.x, st->y - ct.y);
            auto pt2 = qsgPoint2D(pt1.x * trans.m11() + pt1.y * trans.m12() + trans.m13(),
                                  pt1.x * trans.m21() + pt1.y * trans.m22() + trans.m23());
            m_points.push_back(qsgPoint2D(pt2.x + ct.x, pt2.y + ct.y));
            st = st->nxt;
        }

       /* for( int i = 0; i <= 99999; i++) {
            auto pt0 = *evalPoint(ct, r, i * PI / 50);
            auto pt1 = qsgPoint2D(pt0.x - ct.x, pt0.y - ct.y);
            auto pt2 = qsgPoint2D(pt1.x * trans.m11() + pt1.y * trans.m12() + trans.m13(),
                                  pt1.x * trans.m21() + pt1.y * trans.m22() + trans.m23());
            m_points.push_back(qsgPoint2D(pt2.x + ct.x, pt2.y + ct.y));
        }
        m_points.push_back(m_points.at(0));*/

        m_node = new QSGGeometryNode();
        setQSGGemoetry();
        setColor();
    }
    return shapeObject::getQSGNode(aWindow);
}

qsgPoint2D ellipseObject::getRadius(){
    auto r = value("radius").toArray();
    return qsgPoint2D(r[0].toDouble(), r[1].toDouble());
}

qsgPoint2D ellipseObject::getCenter(){
    auto r = value("center").toArray();
    return qsgPoint2D(r[0].toDouble(), r[1].toDouble());
}

double ellipseObject::getAngle(){
    return value("angle").toDouble();
}

static rea::regPip<QJsonObject, rea::pipePartial> create_ellipse([](rea::stream<QJsonObject>* aInput){
    aInput->out<std::shared_ptr<qsgObject>>(std::make_shared<ellipseObject>(aInput->data()));
}, rea::Json("name", "create_qsgobject_ellipse"));

void qsgModel::clearQSGObjects(){
    for (auto i : m_objects)
        i->QSGNodeRemoved();
}

void qsgModel::show(QSGTransformNode* aTransform, QQuickWindow* aWindow){
    for (auto i : m_objects){
        auto nd = i->getQSGNode(aWindow);
        if (nd){
            aTransform->appendChildNode(nd);
        }
    }
}

void qsgModel::zoom(int aStep, const QPointF& aCenter, double aRatio){
    if (aStep == 0){
        m_trans = QTransform();
        if (aRatio == 0.0){
            setTransform();
            return;
        }
    }
    QPointF ct = m_trans.inverted().map(QPointF(aCenter.x(), aCenter.y()));
    double ratio = aRatio;
    if (ratio == 0.0){
        if (aStep > 0)
            ratio = 1.25;
        else
            ratio = 0.8;
    }

    m_trans.translate(ct.x(), ct.y());
    m_trans.scale(ratio, ratio);
    m_trans.translate(- ct.x(), - ct.y());
    setTransform();
}

void qsgModel::setTransform(){
    QString trans = QString::number(m_trans.m11());
    trans += + ";" + QString::number(m_trans.m12());
    trans += + ";" + QString::number(m_trans.m13());
    trans += + ";" + QString::number(m_trans.m21());
    trans += + ";" + QString::number(m_trans.m22());
    trans += + ";" + QString::number(m_trans.m23());
    trans += + ";" + QString::number(m_trans.m31());
    trans += + ";" + QString::number(m_trans.m32());
    trans += + ";" + QString::number(m_trans.m33());
    insert("transform", trans);
}

QTransform qsgModel::getTransform(bool aDeserialize){
    if (aDeserialize){
        auto trans = value("transform").toString().split(";");
        if (trans.length() == 9)
            m_trans = QTransform(trans[0].toDouble(), trans[1].toDouble(), trans[2].toDouble(),
                                 trans[3].toDouble(), trans[4].toDouble(), trans[5].toDouble(),
                                 trans[6].toDouble(), trans[7].toDouble(), trans[8].toDouble());
        else
            m_trans = QTransform();
    }
    return m_trans;
}

void qsgModel::move(const QPointF& aDistance){
    QPointF ds_ = QPointF(aDistance.x(), aDistance.y()),
            ds0 = m_trans.inverted().map(QPointF(ds_.x(), ds_.y())),
            ds1 = m_trans.inverted().map(QPointF(0, 0)),
            ds = ds0 - ds1;
    m_trans.translate(ds.x(), ds.y());
    setTransform();
}


int qsgModel::getWidth() {
    return value("width").toInt();
}

int qsgModel::getHeight() {
    return value("height").toInt();
}

qsgModel::qsgModel(const QJsonObject& aConfig, QMap<QString, QImage> aImages) : QJsonObject(aConfig){
    auto shps = value("objects").toObject();

    auto add_qsg_obj = rea::pipeline::add<std::shared_ptr<qsgObject>>([this](rea::stream<std::shared_ptr<qsgObject>>* aInput){
        auto dt = aInput->data();
        m_objects.insert(dt->value("id").toString(), dt);
        dt->remove("id");
    });
    for (auto i : shps.keys()){
        auto obj = shps.value(i).toObject();
        auto cmd = "create_qsgobject_" + obj.value("type").toString();
        auto tag = rea::Json("tag", add_qsg_obj->actName());
        rea::pipeline::find(cmd)->next(add_qsg_obj, tag);
        rea::pipeline::run<QJsonObject>(cmd, rea::Json(obj, "id", i), tag);
    }

    getTransform(true);

    m_images = aImages;
}

static rea::regPip<int> unit_test([](rea::stream<int>* aInput){
    rea::pipeline::add<QJsonObject>([](rea::stream<QJsonObject>* aInput){
        QMap<QString, QImage> imgs;
        aInput->out<std::shared_ptr<qsgModel>>(std::make_shared<qsgModel>(aInput->data(), imgs), "updateQSGModel_testbrd");
    }, rea::Json("name", "testQSGShow"))->next("updateQSGModel_testbrd");
}, QJsonObject(), "unitTest");
