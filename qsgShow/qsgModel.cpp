#include "qsgModel.h"
#include "reactive2.h"
#include "tess.h"
#include <array>
#include <queue>
#include <QJsonArray>
#include <QSGFlatColorMaterial>
#include <QPainter>

bool shapeObject::canBePickedUp(int aX, int aY){
    auto bnd = getBoundBox();
    return aX >= bnd.left() && aX <= bnd.right() && aY >= bnd.top() && aY <= bnd.bottom();
}

void shapeObject::doSetQSGGeometry(const pointList& aPointList, QSGGeometryNode* aNode, unsigned int aMode, std::vector<uint32_t>* aIndecies){
    auto sz = aIndecies ? aIndecies->size() : aPointList.size();
    QSGGeometry *geometry = new QSGGeometry(QSGGeometry::defaultAttributes_Point2D(), int(sz));
    auto vertices = geometry->vertexDataAsPoint2D();
    if (aIndecies)
        for (auto i = 0; i < aIndecies->size(); ++i){
            auto idx = aIndecies->at(i);
            vertices[i].set(aPointList[idx].x(), aPointList[idx].y());
        }
    else
        for (auto i = 0; i < aPointList.size(); ++i)
            vertices[i].set(aPointList[i].x(), aPointList[i].y());
    geometry->setLineWidth(getWidth());
    /*if (getConfig()->value("fill").toBool() && aPointList.size() > 2)
        geometry->setDrawingMode(GL_POLYGON);
    else*/
    geometry->setDrawingMode(aMode);
    aNode->setGeometry(geometry);
    aNode->setFlag(QSGNode::OwnsGeometry);
    aNode->markDirty(QSGNode::DirtyGeometry);
};

QSGNode* shapeObject::getQSGNode(QQuickWindow* aWindow, qsgModel* aParent) {
    qsgObject::getQSGNode(aWindow, aParent);

    auto fc = m_parent->getFaceOpacity();
    if (fc < 256){
        if (m_node->childCount() == 0){
            auto face = new QSGGeometryNode();

            std::vector<std::vector<std::array<qreal, 2>>> polygon;
            std::vector<std::array<qreal, 2>> poly;
            for (auto i : m_points){
                poly.push_back({i.x(), i.y()});
            }
            polygon.push_back(poly);
            std::vector<uint32_t> indices = mapbox::earcut(polygon);
            doSetQSGGeometry(m_points, face, QSGGeometry::DrawTriangles, &indices);

            QSGFlatColorMaterial *material = new QSGFlatColorMaterial();

            auto clr0 = getColor();
            auto clr = QColor(clr0.red(), clr0.green(), clr0.blue(), fc);
            material->setColor(clr);

            face->setMaterial(material);
            face->setFlag(QSGNode::OwnedByParent);
            face->setFlag(QSGNode::OwnsMaterial);
            m_node->appendChildNode(face);
        }
    }else
        if (m_node->childCount() > 0)
            m_node->removeAllChildNodes();
    return m_node;
}

void shapeObject::updateTextValue(const QJsonObject& aTextConfig){
    auto sz = m_parent->getTextSize(aTextConfig);
    auto img = QImage(sz.x(), sz.y(), QImage::Format_ARGB32);
    auto clr = getColor();
    img.fill(QColor(clr.red(), clr.green(), clr.blue(), 64));

    auto txt = getText();
    QPainter p(&img);
    auto bnd = p.fontMetrics().boundingRect(txt);
    auto factor = std::min(sz.x() * 0.8 / bnd.width(), sz.y() * 0.8 / bnd.height()); // https://stackoverflow.com/questions/2202717/for-qt-4-6-x-how-to-auto-size-text-to-fit-in-a-specified-width
    if (factor < 1 || factor > 1.25){
        auto font = p.font();
        font.setPointSizeF(font.pointSizeF() * factor);
        p.setFont(font);
    }
    p.setPen(QPen(clr));
    //p.setFont(QFont("TimesNewRoman", std::min(22, int(std::round(img2.width() * 1.0 / wdh * 8))), QFont::Normal));
    p.drawText(img.rect(), Qt::AlignCenter, txt);
    m_text->setTexture(m_window->createTextureFromImage(img));
    m_text->markDirty(QSGNode::DirtyMaterial);
}

void shapeObject::updateTextLocation(const QJsonObject& aTextConfig, const QSGTransformNode* aTransform){
    auto bnd = getBoundBox();
    auto top_lft = bnd.topLeft(), btm_rt = bnd.bottomRight();
    if (aTransform){
        auto trans = aTransform->matrix();
        top_lft = trans.map(top_lft);
        btm_rt = trans.map(btm_rt);
    }
    auto sz = m_parent->getTextSize(aTextConfig);
    auto del = 0.5 * (btm_rt.x() - top_lft.x() - sz.x());
    auto loc = m_parent->getTextLocation(aTextConfig);
    if (loc == "bottom")
        m_text->setRect(top_lft.x() + del, btm_rt.y() + 5, sz.x(), sz.y());
    else
        m_text->setRect(top_lft.x() + del, top_lft.y() - sz.y() - 5, sz.x(), sz.y());
    m_text->markDirty(QSGNode::DirtyGeometry);
}

void shapeObject::addQSGNode(QSGNode* aParent) {
    qsgObject::addQSGNode(aParent);
    if (aParent){
        auto txt_cfg = m_parent->getTextConfig();
        if (m_parent->getTextVisible(txt_cfg) && !m_text){
            m_text = new QSGSimpleTextureNode();// window()->createImageNode();
            m_text->setFlag(QSGNode::OwnsMaterial);
            updateTextValue(txt_cfg);
            updateTextLocation(txt_cfg);
            aParent->parent()->appendChildNode(m_text);
        }
    }else{
        m_node = nullptr;
        if (m_text){
            m_text->parent()->removeChildNode(m_text);
            m_text = nullptr;
        }
    }
}

void shapeObject::transformChanged(){
    if (m_text && m_node){
        auto trans = reinterpret_cast<QSGTransformNode*>(m_node->parent());
        updateTextLocation(m_parent->getTextConfig(), trans);
    }
}

QRectF calcBoundBox(const pointList &aPoints){
    QRectF ret(aPoints[0], aPoints[0]);
    for (auto i : aPoints){
        auto x = i.x(), y = i.y();
        if (x < ret.left())
            ret.setLeft(x);
        if (x > ret.right())
            ret.setRight(x);
        if (y < ret.top())
            ret.setTop(y);
        if (y > ret.bottom())
            ret.setBottom(y);
    }
    return ret;
}

void shapeObject::setQSGGemoetry(){
    doSetQSGGeometry(m_points, m_node, QSGGeometry::DrawLineStrip);
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

QString shapeObject::getText(){
    return value("text").toString();
}

QSGNode* imageObject::getQSGNode(QQuickWindow* aWindow, qsgModel* aParent){
    qsgObject::getQSGNode(aWindow, aParent);
    if (!m_node){
        QImage img;
        auto pth = getPath();
        if (m_parent->m_images.contains(pth))
            img = m_parent->m_images.value(pth);
        else
            img = QImage(pth);
        if (img.width() > 0 && img.height() > 0){
            m_node = new QSGSimpleTextureNode();
            m_node->setTexture(m_window->createTextureFromImage(img));
            m_node->setRect(img.rect());
            m_node->markDirty(QSGNode::DirtyMaterial);
        }
    }
    return m_node;
}

QString imageObject::getPath(){
    return value("path").toString();
}

static rea::regPip<QJsonObject, rea::pipePartial> create_image([](rea::stream<QJsonObject>* aInput){
    aInput->out<std::shared_ptr<qsgObject>>(std::make_shared<imageObject>(aInput->data()));
}, rea::Json("name", "create_qsgobject_image"));

polyObject::polyObject(const QJsonObject& aConfig) : shapeObject(aConfig){

}

QSGNode* polyObject::getQSGNode(QQuickWindow* aWindow, qsgModel* aParent) {
    if (!m_node){
        auto pts = getPoints();
        for (int i = 0; i < pts.size(); i += 2)
            m_points.push_back(QPointF(pts[i].toDouble(), pts[i + 1].toDouble()));
        m_bound = calcBoundBox(m_points);
        m_node = new QSGGeometryNode();
        setQSGGemoetry();
        setColor();
    }
    return shapeObject::getQSGNode(aWindow, aParent);
}

static rea::regPip<QJsonObject, rea::pipePartial> create_poly([](rea::stream<QJsonObject>* aInput){
    aInput->out<std::shared_ptr<qsgObject>>(std::make_shared<polyObject>(aInput->data()));
}, rea::Json("name", "create_qsgobject_poly"));

ellipseObject::ellipseObject(const QJsonObject& aConfig) : shapeObject(aConfig){

}

float squarePointProjectToLine2D(const QPointF& aPoint, const QPointF& aStart, const QPointF& aEnd){
    QPointF s = aPoint - aStart, e = aEnd - aStart;
    auto dot = QPointF::dotProduct(s, e);
    return dot * dot / QPointF::dotProduct(e, e);
}

std::shared_ptr<ellipseObject::l_qsgPoint3D> ellipseObject::evalPoint(const QPointF& aCenter, const QPointF& aRadius, double aParam){
    return std::make_shared<l_qsgPoint3D>(aCenter.x() + (aRadius.x() * std::cos(aParam)),
                                          aCenter.y() + (aRadius.y() * std::sin(aParam)),
                      aParam);
}

QSGNode* ellipseObject::getQSGNode(QQuickWindow* aWindow, qsgModel* aParent){
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
            auto pt = trans.map(QPointF(st->x() - ct.x(), st->y() - ct.y()));
            m_points.push_back(pt + ct);
            st = st->nxt;
        }
        m_bound = calcBoundBox(m_points);

       /* for( int i = 0; i <= 99999; i++) {
            auto pt0 = *evalPoint(ct, r, i * PI / 50);
            auto pt1 = QPointF(pt0.x - ct.x, pt0.y - ct.y);
            auto pt2 = QPointF(pt1.x * trans.m11() + pt1.y * trans.m12() + trans.m13(),
                                  pt1.x * trans.m21() + pt1.y * trans.m22() + trans.m23());
            m_points.push_back(QPointF(pt2.x + ct.x, pt2.y + ct.y));
        }
        m_points.push_back(m_points.at(0));*/

        m_node = new QSGGeometryNode();
        setQSGGemoetry();
        setColor();
    }
    return shapeObject::getQSGNode(aWindow, aParent);
}

QPointF ellipseObject::getRadius(){
    auto r = value("radius").toArray();
    return QPointF(r[0].toDouble(), r[1].toDouble());
}

QPointF ellipseObject::getCenter(){
    auto r = value("center").toArray();
    return QPointF(r[0].toDouble(), r[1].toDouble());
}

double ellipseObject::getAngle(){
    return value("angle").toDouble();
}

static rea::regPip<QJsonObject, rea::pipePartial> create_ellipse([](rea::stream<QJsonObject>* aInput){
    aInput->out<std::shared_ptr<qsgObject>>(std::make_shared<ellipseObject>(aInput->data()));
}, rea::Json("name", "create_qsgobject_ellipse"));

void qsgModel::clearQSGObjects(){
    for (auto i : m_objects)
        i->addQSGNode();
}

void qsgModel::show(QSGNode* aTransform, QQuickWindow* aWindow){
    for (auto i : m_objects){
        i->getQSGNode(aWindow, this);
        i->addQSGNode(aTransform);
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

bool qsgModel::getShowArrow(){
    return value("arrow").toBool();
}

int qsgModel::getFaceOpacity(){
    return value("face").toInt();
}

QJsonObject qsgModel::getTextConfig(){
    return value("text").toObject();
}

bool qsgModel::getTextVisible(const QJsonObject& aConfig){
    return aConfig.value("visible").toBool();
}

QPoint qsgModel::getTextSize(const QJsonObject& aConfig){
    auto sz = aConfig.value("size").toArray();
    if (sz.size() == 2)
        return QPoint(sz[0].toInt(), sz[1].toInt());
    else
        return QPoint(100, 50);
}

QString qsgModel::getTextLocation(const QJsonObject& aConfig){
    return aConfig.value("location").toString();
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

void qsgModel::transformChanged(){
    for (auto i : m_objects)
        i->transformChanged();
}

static rea::regPip<int> unit_test([](rea::stream<int>* aInput){
    rea::pipeline::add<QJsonObject>([](rea::stream<QJsonObject>* aInput){
        QMap<QString, QImage> imgs;
        auto pth = "F:/3M/B4DT/DF Mark/V1-1.bmp";
        imgs.insert(pth, QImage(pth));

        auto cfg = rea::Json("width", 600,
                             "height", 600,
                             "arrow", true,
                             "face", 200,
                             "text", rea::Json("visible", false,
                                               "size", rea::JArray(100, 50)),
                             "objects", rea::Json(
                                            "img_2", rea::Json(
                                                         "type", "image",
                                                         "path", pth
                                                         ),
                                            "shp_0", rea::Json(
                                                         "type", "poly",
                                                         "points", rea::JArray(50, 50, 200, 200, 200, 50, 50, 50),
                                                         "color", "red",
                                                         "width", 3,
                                                         "text", "poly"
                                                         ),
                                            "shp_1", rea::Json(
                                                         "type", "ellipse",
                                                         "center", rea::JArray(400, 400),
                                                         "radius", rea::JArray(300, 200),
                                                         "color", "blue",
                                                         "width", 5,
                                                         "text", "ellipse"
                                                         )
                                                ));

        auto view = aInput->data();
        for (auto i : view.keys())
            cfg.insert(i, view.value(i));

        aInput->out<std::shared_ptr<qsgModel>>(std::make_shared<qsgModel>(cfg, imgs), "updateQSGModel_testbrd");
    }, rea::Json("name", "testQSGShow"))->next("updateQSGModel_testbrd");
}, QJsonObject(), "unitTest");
