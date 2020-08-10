#include "qsgModel.h"
#include "imagePool.h"
#include "tess.h"
#include <array>
#include <queue>
#include <QJsonArray>
#include <QSGFlatColorMaterial>
#include <QSGVertexColorMaterial>
#include <QPainter>

bool shapeObject::canBePickedUp(int aX, int aY){
    auto bnd = getBoundBox();
    return aX >= bnd.left() && aX <= bnd.right() && aY >= bnd.top() && aY <= bnd.bottom();
}

IUpdateQSGAttr shapeObject::updateQSGAttr(const QString& aModification){
    if (aModification == "face_")
        return [this](){checkFaceOpacity();};
    else if (aModification == "text_visible_")
        return [this](){checkTextVisible();};
    else if (aModification == "color_")
        return [this](){checkColor();};
    else if (aModification == "caption_")
        return [this](){checkCaption();};
    else if (aModification == "width_")
        return [this](){checkWidth();};
    else if (aModification == "angle_")
        return [this](){checkAngle();};
    else
        return nullptr;
}

std::vector<QSGNode*> shapeObject::getQSGNodes(QQuickWindow* aWindow, qsgModel* aParent, QSGNode* aTransformNode){
    auto ret = qsgObject::getQSGNodes(aWindow, aParent, aTransformNode);
    if (!m_outline){
        m_outline = new QSGGeometryNode();
        updateGeometry();
        setQSGColor(*m_outline, getColor());
        aTransformNode->appendChildNode(m_outline);
    }
    ret.push_back(m_outline);
    checkFaceOpacity();
    checkTextVisible();
    return ret;
}

int shapeObject::getFaceOpacity(){
    if (contains("face"))
        return value("face").toInt();
    else
        return m_parent->getFaceOpacity();
}

QJsonObject shapeObject::getTextConfig(){
    if (contains("text"))
        return value("text").toObject();
    else
        return m_parent->getTextConfig();
}

QJsonObject shapeObject::getArrowConfig(){
    if (contains("arrow"))
        return value("arrow").toObject();
    else
        return m_parent->getArrowConfig();
}

bool shapeObject::getArrowVisible(const QJsonObject& aConfig){
    return m_parent->getArrowVisible(aConfig);
}

bool shapeObject::getPoleArrow(const QJsonObject& aConfig){
    return m_parent->getPoleArrow(aConfig);
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

void shapeObject::updateTextLocation(const QJsonObject& aTextConfig){
    auto bnd = getBoundBox();
    auto top_lft = bnd.topLeft(), btm_rt = bnd.bottomRight();

    auto trans = reinterpret_cast<QSGTransformNode*>(m_outline->parent())->matrix();
    top_lft = trans.map(top_lft);
    btm_rt = trans.map(btm_rt);

    auto sz = m_parent->getTextSize(aTextConfig);
    auto del = 0.5 * (btm_rt.x() - top_lft.x() - sz.x());
    auto loc = m_parent->getTextLocation(aTextConfig);
    if (loc == "bottom")
        m_text->setRect(top_lft.x() + del, btm_rt.y() + 5, sz.x(), sz.y());
    else if (loc == "middle"){
        auto del2 = 0.5 * (btm_rt.y() - top_lft.y() - sz.y());
        m_text->setRect(top_lft.x() + del, top_lft.y() + del2, sz.x(), sz.y());
    }
    else
        m_text->setRect(top_lft.x() + del, top_lft.y() - sz.y() - 5, sz.x(), sz.y());
    m_text->markDirty(QSGNode::DirtyGeometry);
}

void shapeObject::removeQSGNodes() {
    qsgObject::removeQSGNodes();

    m_outline = nullptr;
    m_holes.clear();
    if (m_text){
        m_text->parent()->removeChildNode(m_text); //cannot delete here
        m_text = nullptr;
    }
    for (auto i : m_arrows){
        i->parent()->removeChildNode(i);
        delete i;
    }
    m_arrows.clear();
}

void shapeObject::transformChanged(){
    if (m_text && m_outline){
        updateTextLocation(getTextConfig());
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

void shapeObject::setQSGGemoetry(const pointList& aPointList, QSGGeometryNode& aNode, unsigned int aMode, std::vector<uint32_t>* aIndecies){
    auto sz = aIndecies ? aIndecies->size() : aPointList.size();
    auto wth = getWidth();
    QSGGeometry *geometry = new QSGGeometry(QSGGeometry::defaultAttributes_Point2D(), int(sz));
    auto vertices = geometry->vertexDataAsPoint2D();
    if (aIndecies)
        for (auto i = 0; i < aIndecies->size(); ++i){
            auto idx = aIndecies->at(i);
            vertices[i].set(aPointList[idx].x(), aPointList[idx].y());
        }
    else if (wth)
        for (auto i = 0; i < aPointList.size(); ++i)
            vertices[i].set(aPointList[i].x(), aPointList[i].y());
    geometry->setLineWidth(wth);
    /*if (getConfig()->value("fill").toBool() && aPointList.size() > 2)
        geometry->setDrawingMode(GL_POLYGON);
    else*/
    geometry->setDrawingMode(aMode);
    aNode.setGeometry(geometry);
    aNode.setFlag(QSGNode::OwnsGeometry);
    aNode.markDirty(QSGNode::DirtyGeometry);
}

void shapeObject::setQSGColor(QSGGeometryNode& aNode, const QColor& aColor){
    QSGFlatColorMaterial *material = new QSGFlatColorMaterial();
    material->setColor(aColor);
    aNode.setMaterial(material);
    aNode.setFlag(QSGNode::OwnsMaterial);
    aNode.markDirty(QSGNode::DirtyMaterial);
}

void shapeObject::updateArrowCount(int aCount){
    for (int i  = m_arrows.size() - 1; i >= aCount; --i){
        auto arr = m_arrows[i];
        arr->parent()->removeChildNode(arr);
        m_arrows.pop_back();
        delete arr;
    }
    auto prt = m_outline->parent()->parent();
    for (int i = m_arrows.size(); i < aCount; ++i){
        auto arrow = new QSGGeometryNode();
        pointList pts;
        setQSGGemoetry(pts, *arrow, QSGGeometry::DrawLineStrip);
        setQSGColor(*arrow, getColor());
        prt->appendChildNode(arrow);
        m_arrows.push_back(arrow);
    }
}

void shapeObject::checkArrowVisible(int aCount){
    if (m_outline){
        if (getArrowVisible(getArrowConfig())) {
            updateArrowCount(aCount);
            if (m_arrows.size() > 0)
                updateArrowLocation();
        }else
            updateArrowCount(0);
    }
}

void shapeObject::updateQSGFace(QSGGeometryNode& aNode, int aOpacity){
    std::vector<std::vector<std::array<qreal, 2>>> polygon;
    pointList pts;
    for (auto i : m_points){
        std::vector<std::array<qreal, 2>> poly;
        for (auto j : i){
            poly.push_back({j.x(), j.y()});
            pts.push_back(j);
        }
        polygon.push_back(poly);
    }
    std::vector<uint32_t> indices = mapbox::earcut(polygon);
    setQSGGemoetry(pts, aNode, QSGGeometry::DrawTriangles, &indices);
    updateQSGFaceColor(aNode, aOpacity);
}

void shapeObject::updateQSGFaceColor(QSGGeometryNode& aNode, int aOpacity){
    auto clr0 = getColor();
    auto clr = QColor(clr0.red(), clr0.green(), clr0.blue(), aOpacity);
    setQSGColor(aNode, clr);
}

void shapeObject::checkFaceOpacity(){
    if (!m_outline)
        return;
    auto fc = getFaceOpacity();
    if (fc > 0){
        if (!m_face){
            m_face = new QSGGeometryNode();
            updateQSGFace(*m_face, fc);
            m_face->setFlag(QSGNode::OwnedByParent);
            m_outline->appendChildNode(m_face);
        }else
            updateQSGFace(*m_face, fc);
    }else
        if (m_face){
            m_face->parent()->removeChildNode(m_face);
            delete m_face;
            m_face = nullptr;
        }
}

void shapeObject::checkTextVisible(){
    auto txt_cfg = getTextConfig();
    if (m_parent->getTextVisible(txt_cfg)){
        if (!m_text){
            m_text = new QSGSimpleTextureNode();// window()->createImageNode();
            m_text->setFlag(QSGNode::OwnsMaterial);
            updateTextValue(txt_cfg);
            updateTextLocation(txt_cfg);
            m_outline->parent()->parent()->appendChildNode(m_text);
        }
    }else
        if (m_text){
            m_text->parent()->removeChildNode(m_text);
            m_text = nullptr;
        }
}

void shapeObject::checkColor(){
    if (m_outline)
        setQSGColor(*m_outline, getColor());
    for (auto i : m_arrows)
        setQSGColor(*i, getColor());
    if (m_face)
        updateQSGFaceColor(*m_face, getFaceOpacity());
    auto txt_cfg = getTextConfig();
    if (m_parent->getTextVisible(txt_cfg) && m_text)
        updateTextValue(txt_cfg);
}

void shapeObject::checkCaption(){
    auto txt_cfg = getTextConfig();
    if (m_parent->getTextVisible(txt_cfg) && m_text)
        updateTextValue(txt_cfg);
}

void shapeObject::checkWidth(){
    if (m_outline){
        for (int i = 0; i < m_points.size(); ++i)
            if (i == 0)
                setQSGGemoetry(m_points[i], *m_outline, QSGGeometry::DrawLineStrip);
            else
                setQSGGemoetry(m_points[i], *m_holes[i - 1], QSGGeometry::DrawLineStrip);
    }
    if (m_arrows.size() > 0)
        updateArrowLocation();
}

void shapeObject::checkAngle(){
    updateGeometry();
    if (m_arrows.size() > 0)
        updateArrowLocation();
    if (m_face)
        updateQSGFace(*m_face, getFaceOpacity());
    auto txt_cfg = getTextConfig();
    if (m_parent->getTextVisible(txt_cfg) && m_text)
        updateTextLocation(txt_cfg);
}

int shapeObject::getWidth(){
    return value("width").toInt(5);
}

QColor shapeObject::getColor(){
    if (contains("color"))
        return QColor(value("color").toString());
    else
        return QColor(m_parent->value("color").toString("red"));
}

QString shapeObject::getText(){
    return value("caption").toString();
}

void shapeObject::calcArrow(const QPointF& aStart, const QPointF& aEnd, QSGGeometryNode& aNode){
    const float si = 0.5, co = 0.866;
    auto dir = aStart - aEnd;
    dir = dir / sqrt(QPointF::dotProduct(dir, dir)) * 15;
    pointList pts;
    pts.push_back(QPointF(dir.x() * co + dir.y() * si, - dir.x() * si + dir.y() * co) + aEnd);
    pts.push_back(aEnd);
    pts.push_back(QPointF(dir.x() * co - dir.y() * si, dir.x() * si + dir.y() * co) + aEnd);
    setQSGGemoetry(pts, aNode, QSGGeometry::DrawLineStrip);
}

double shapeObject::getAngle(){
    return value("angle").toDouble();
}

std::vector<QSGNode*> imageObject::getQSGNodes(QQuickWindow* aWindow, qsgModel* aParent, QSGNode* aTransformNode){
    auto ret = qsgObject::getQSGNodes(aWindow, aParent, aTransformNode);
    if (!m_node){
        m_node = new QSGSimpleTextureNode();
        updateImagePath();
        aTransformNode->appendChildNode(m_node);
    }
    ret.push_back(m_node);
    return ret;
}

void imageObject::updateImagePath(){
    QImage img = rea::imagePool::readCache(getPath());
    if (img.width() == 0 || img.height() == 0){
        img = QImage(10, 10, QImage::Format_ARGB32);
        img.fill(QColor("transparent"));
    }
    m_node->setTexture(m_window->createTextureFromImage(img));
    m_node->setRect(getRange(img));
    m_node->markDirty(QSGNode::DirtyMaterial);
}

IUpdateQSGAttr imageObject::updateQSGAttr(const QString& aModification){
    if (aModification == "path_" || aModification == "range_")
        return [this](){
            updateImagePath();
        };
    else
        return qsgObject::updateQSGAttr(aModification);
}

QRectF imageObject::getRange(const QImage& aImage){
    auto rg = value("range").toArray();
    return rg.size() == 4 ? QRectF(QPointF(rg[0].toDouble(), rg[1].toDouble()),
                                   QPointF(rg[2].toDouble(), rg[3].toDouble())) : aImage.rect();
}

QString imageObject::getPath(){
    return value("path").toString();
}

static rea::regPip<QJsonObject, rea::pipePartial> create_image([](rea::stream<QJsonObject>* aInput){
    aInput->out<std::shared_ptr<qsgObject>>(std::make_shared<imageObject>(aInput->data()));
}, rea::Json("name", "create_qsgobject_image"));

polyObject::polyObject(const QJsonObject& aConfig) : shapeObject(aConfig){

}

QJsonArray polyObject::getPoints(){
    return value("points").toArray();
}

void polyObject::checkGeometry(){
    updateGeometry();
    if (m_arrows.size() > 0)
        checkArrowPole();
    if (m_face)
        updateQSGFace(*m_face, getFaceOpacity());
    auto txt_cfg = getTextConfig();
    if (m_parent->getTextVisible(txt_cfg) && m_text)
        updateTextLocation(txt_cfg);
}

std::vector<QSGNode*> polyObject::getQSGNodes(QQuickWindow* aWindow, qsgModel* aParent, QSGNode* aTransformNode) {
    auto ret = shapeObject::getQSGNodes(aWindow, aParent, aTransformNode);
    checkArrowPole();
    return ret;
}

void polyObject::updateGeometry(){
    auto pts0 = getPoints();
    if (pts0.size() == 0)
        return;
    std::vector<pointList> pts2;
    for (auto j : pts0){
        auto pts = j.toArray();
        pointList pts1;
        for (int i = 0; i < pts.size(); i += 2)
            pts1.push_back(QPointF(pts[i].toDouble(), pts[i + 1].toDouble()));
        pts2.push_back(pts1);
    }
    auto bnd = calcBoundBox(pts2[0]);

    auto ct = bnd.center();
    QTransform trans;
    trans.rotate(getAngle());
    m_points.clear();
    for (auto j : pts2){
        pointList pts1;
        for (auto i : j)
            pts1.push_back(trans.map(i - ct) + ct);
        m_points.push_back(pts1);
    }
    m_bound = calcBoundBox(m_points[0]);
    setQSGGemoetry(m_points[0], *m_outline, QSGGeometry::DrawLineStrip);
}

void polyObject::updateArrowLocation(){
    auto trans = reinterpret_cast<QSGTransformNode*>(m_outline->parent())->matrix();
    int idx = 0;
    for (auto j : m_points){
        auto st = trans.map(j[0]);
        bool pole = getPoleArrow(getArrowConfig());
        for (size_t i = 1; i < j.size(); ++i){
            auto ed = trans.map(j[i]);
            calcArrow(st, ed, *m_arrows[idx]);
            if (pole)
                calcArrow(ed, st, *m_arrows[idx + m_points.size() - 1]);
            st = ed;
            idx++;
        }
    }
}

void polyObject::transformChanged(){
    shapeObject::transformChanged();
    if (m_outline && m_arrows.size() > 0)
        updateArrowLocation();
}

IUpdateQSGAttr polyObject::updateQSGAttr(const QString& aModification){
    if (aModification == "arrow_visible_")
        return [this](){
            checkArrowPole();
        };
    else if (aModification == "arrow_pole_")
        return [this](){
            checkArrowPole();
        };
    else if (aModification == "points_")
        return [this](){checkGeometry();};
    else
        return shapeObject::updateQSGAttr(aModification);
}

static rea::regPip<QJsonObject, rea::pipePartial> create_poly([](rea::stream<QJsonObject>* aInput){
    aInput->out<std::shared_ptr<qsgObject>>(std::make_shared<polyObject>(aInput->data()));
}, rea::Json("name", "create_qsgobject_poly"));

void polyObject::checkArrowPole(){
    int sz = 0;
    for (auto i : m_points)
        sz += i.size() - 1;
    if (getPoleArrow(getArrowConfig()))
        checkArrowVisible(sz * 2);
    else
        checkArrowVisible(sz);
}

void ellipseObject::updateArrowLocation(){
    auto w_trans = reinterpret_cast<QSGTransformNode*>(m_outline->parent())->matrix();
    auto r = getRadius();
    auto ct = getCenter();
    auto del = getCCW() ?  - 1 : 1;
    pointList pts = {QPointF(ct.x() - r.x(), ct.y()), QPointF(ct.x(), ct.y() + r.y()),
                     QPointF(ct.x() + r.x(), ct.y()), QPointF(ct.x(), ct.y() - r.y()),
                     QPointF(ct.x() - r.x(), ct.y() + del), QPointF(ct.x() + del, ct.y() + r.y()),
                     QPointF(ct.x() + r.x(), ct.y() - del), QPointF(ct.x() - del, ct.y() - r.y())};
    QTransform trans;
    auto ang = getAngle();
    trans.rotate(ang);
    for (auto i = 0; i < 4; ++i)
        calcArrow(w_trans.map(trans.map(pts[i + 4] - ct) + ct), w_trans.map(trans.map(pts[i] - ct) + ct), *m_arrows[i]);
}

void ellipseObject::transformChanged(){
    shapeObject::transformChanged();
    if (m_arrows.size() > 0 && m_outline)
        updateArrowLocation();
}

IUpdateQSGAttr ellipseObject::updateQSGAttr(const QString& aModification){
    if (aModification == "arrow_visible_")
        return [this](){
            checkArrowVisible(4);
        };
    else if (aModification == "center_" || aModification == "radius_")
        return [this](){
            checkAngle();
        };
    else if (aModification == "ccw_")
        return [this](){
            if (m_arrows.size() > 0 && m_outline)
                updateArrowLocation();
        };
    else
        return shapeObject::updateQSGAttr(aModification);
}

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

std::vector<QSGNode*> ellipseObject::getQSGNodes(QQuickWindow* aWindow, qsgModel* aParent, QSGNode* aTransformNode){
    auto ret = shapeObject::getQSGNodes(aWindow, aParent, aTransformNode);
    checkArrowVisible(4);
    return ret;
}

void ellipseObject::updateGeometry(){
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

    pointList pts;
    auto st = i0;
    while (st){
        auto pt = trans.map(QPointF(st->x() - ct.x(), st->y() - ct.y()));
        pts.push_back(pt + ct);
        st = st->nxt;
    }
    m_bound = calcBoundBox(pts);

    /* for( int i = 0; i <= 99999; i++) {
            auto pt0 = *evalPoint(ct, r, i * PI / 50);
            auto pt1 = QPointF(pt0.x - ct.x, pt0.y - ct.y);
            auto pt2 = QPointF(pt1.x * trans.m11() + pt1.y * trans.m12() + trans.m13(),
                                  pt1.x * trans.m21() + pt1.y * trans.m22() + trans.m23());
            m_points.push_back(QPointF(pt2.x + ct.x, pt2.y + ct.y));
        }
        m_points.push_back(m_points.at(0));*/

    setQSGGemoetry(pts, *m_outline, QSGGeometry::DrawLineStrip);

    m_points.clear();
    m_points.push_back(pts);
}

QPointF ellipseObject::getRadius(){
    auto r = value("radius").toArray();
    return QPointF(r[0].toDouble(), r[1].toDouble());
}

bool ellipseObject::getCCW(){
    return value("ccw").toBool();
}

QPointF ellipseObject::getCenter(){
    auto r = value("center").toArray();
    return QPointF(r[0].toDouble(), r[1].toDouble());
}

static rea::regPip<QJsonObject, rea::pipePartial> create_ellipse([](rea::stream<QJsonObject>* aInput){
    aInput->out<std::shared_ptr<qsgObject>>(std::make_shared<ellipseObject>(aInput->data()));
}, rea::Json("name", "create_qsgobject_ellipse"));

void qsgModel::clearQSGObjects(){
    for (auto i : m_objects)
        i->removeQSGNodes();
}

void qsgModel::show(QSGTransformNode* aTransform, QQuickWindow* aWindow, const QPointF& aSize){
    if (!m_window){
        m_size = aSize;
        m_window = aWindow;
        m_trans_node = aTransform;
        for (auto i : m_objects)
            i->getQSGNodes(aWindow, this, aTransform);
        WCS2SCS();
    }
}

/*{type: "add", obj: "shp_3", val: {
     type: "poly",
     points: [300, 300, 500, 300, 400, 400, 300, 300],
     color: "green",
     caption: "new_obj",
     face: 200
 }} : {type: "del", obj: "shp_3"}}*/

IUpdateQSGAttr qsgModel::updateQSGAttr(const QJsonObject& aModification){
    if (aModification.contains("obj")){
        auto obj = aModification.value("obj").toString();
        if (m_objects.contains(obj)){
            auto nd = m_objects.value(obj);
            auto mdy = overwriteAttr(*nd, aModification.value("key").toArray(), aModification.value("val"));
            if (mdy != "")
                return nd->updateQSGAttr(mdy);
        }
    }else{
        auto kys = aModification.value("key").toArray();
        if (kys.size() > 0){
            if (kys[0] == "transform"){
                if (aModification.value("type") == "zoom"){
                    auto ct = aModification.value("center").toArray();
                    zoom(aModification.value("dir").toInt(), QPointF(ct[0].toDouble(), ct[1].toDouble()));
                }else if (aModification.value("type") == "move"){
                    auto del = aModification.value("del").toArray();
                    move(QPointF(del[0].toDouble(), del[1].toDouble()));
                }else
                    return nullptr;
                setTransform();
                return [this](){
                    WCS2SCS();
                };
            }else if (kys[0] == "objects"){
                if (aModification.value("type") == "add"){
                    auto obj = aModification.value("tar").toString();
                    if (!contains(obj)){
                        auto attr = aModification.value("val").toObject();
                        insert(obj, attr);
                        addObject(rea::Json(attr, "id", obj));
                        return [this, obj](){
                            m_objects.value(obj)->getQSGNodes(m_window, this, m_trans_node);
                        };
                    }
                }else if (aModification.value("type") == "del"){
                    auto obj = aModification.value("tar").toString();
                    if (contains(obj)){
                        remove(obj);
                        return [this, obj](){
                            m_objects.value(obj)->removeQSGNodes();
                            m_objects.remove(obj);
                        };
                    }
                }
            }else{
                auto mdy = overwriteAttr(*this, aModification.value("key").toArray(), aModification.value("val"));
                if (mdy != "")
                    return [this, mdy](){
                        for (auto i : m_objects){
                            auto up = i->updateQSGAttr(mdy);
                            if (up)
                                up();
                        }
                    };
            }
        }
    }
    return nullptr;
}

void qsgModel::zoom(int aStep, const QPointF& aCenter, double aRatio){
    if (aStep == 0){
        m_trans = QTransform();
        if (aRatio == 0.0){
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
}

void qsgModel::WCS2SCS(){
    auto width = getWidth(),
         height = getHeight();
    if (width == 0)
        width = int(m_size.x());
    if (height == 0)
        height = int(m_size.y());
    QTransform trans0;
    trans0.scale(m_size.x() * 1.0 / width, m_size.y() * 1.0 / height);
    // return aTransform;
    auto ratio =  width * 1.0 / height;
    QTransform trans;
    if (ratio > m_size.x() * 1.0 / m_size.y()){
        auto ry = m_size.x() / ratio / m_size.y();
        trans = trans.scale(1, ry);
        trans = trans.translate(0, (m_size.y() - m_size.x() / ratio) * 0.5 / ry);
        //m_image->setRect(QRect(0, (m_ui_height - m_ui_width / ratio) * 0.5, m_ui_width, m_ui_width / ratio));
    }else{
        auto rx = (m_size.y() * ratio) / m_size.x();
        trans = trans.scale(rx, 1);
        trans = trans.translate((m_size.x() - m_size.y() * ratio) * 0.5 / rx, 0);
        //m_image->setRect(QRect((m_ui_width - m_ui_height * ratio) * 0.5, 0, m_ui_height * ratio, m_ui_height));
    }
    m_trans_node->setMatrix(QMatrix4x4(trans0 * trans * getTransform()));
    m_trans_node->markDirty(QSGNode::DirtyMatrix);
    for (auto i : m_objects)
        i->transformChanged();
}

QString qsgModel::overwriteAttr(QJsonObject& aObject, const QJsonArray& aKeys, const QJsonValue&& aValue){
    if (aKeys.size() == 0)
        return "";
    else if (aKeys.size() == 1){
        auto ret = aKeys[0].toString();
        if (aObject.value(ret) == aValue)
            return "";
        else{
            aObject.insert(ret, aValue);
            return ret + "_";
        }
    }
    QString ret = "";
    std::vector<QJsonObject> objs;
    auto org = aObject;
    for (int i = 0; i < aKeys.size(); ++i){
        auto key = aKeys[i].toString();
        ret += key + "_";
        if (i == aKeys.size() - 1){
            if (org.value(key) == aValue)
                return "";
            objs.back().insert(key, aValue);
        }
        else{
            if (i == 0)
                objs.push_back(aObject.value(key).toObject());
            else
                objs.push_back(objs.back().value(key).toObject());
            org = org.value(key).toObject();
        }
    }
    for (int i = objs.size() - 1; i > 0; --i){
        objs[i - 1].insert(aKeys[i].toString(), objs[i]);
    }
    aObject.insert(aKeys[0].toString(), objs[0]);
    return ret;
}

QJsonObject qsgModel::getArrowConfig(){
    return value("arrow").toObject();
}

bool qsgModel::getArrowVisible(const QJsonObject& aConfig){
    return aConfig.value("visible").toBool();
}

bool qsgModel::getPoleArrow(const QJsonObject& aConfig){
    return aConfig.value("pole").toBool();
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
}


int qsgModel::getWidth() {
    return value("width").toInt();
}

int qsgModel::getHeight() {
    return value("height").toInt();
}

rea::pipe0* qsgModel::objectCreator(const QString& aName){
    if (!m_creators.contains(aName))
        m_creators.insert(aName, rea::local(aName));
    return m_creators.value(aName);
}

void qsgModel::addObject(const QJsonObject& aConfig){
    auto cmd = "create_qsgobject_" + aConfig.value("type").toString();
    objectCreator(cmd)->nextB(0, m_add_object, QJsonObject())  //ensure the create_qsgobject_ is in the same thead of here
        ->execute(std::make_shared<rea::stream<QJsonObject>>(aConfig));
}

qsgModel::qsgModel(const QJsonObject& aConfig) : QJsonObject(aConfig){
    auto shps = value("objects").toObject();

    m_add_object = rea::pipeline::add<std::shared_ptr<qsgObject>>([this](rea::stream<std::shared_ptr<qsgObject>>* aInput){
        auto dt = aInput->data();
        m_objects.insert(dt->value("id").toString(), dt);
        dt->remove("id");
    });
    for (auto i : shps.keys())
        addObject(rea::Json(shps.value(i).toObject(), "id", i));
    getTransform(true);
}

qsgModel::~qsgModel(){
    for (auto i : m_creators)
        rea::pipeline::remove(i->actName());
}

static rea::regPip<int> unit_test([](rea::stream<int>* aInput){
    rea::pipeline::add<QJsonObject>([](rea::stream<QJsonObject>* aInput){
        static int count = 0;
        auto pth = "F:/3M/B4DT/DF Mark/V1-1.bmp";
        QImage img(pth);
        rea::imagePool::cacheImage(pth, img);
        auto cfg = rea::Json("width", img.width() ? img.width() : 600,
                             "height", img.height() ? img.height() : 600,
                             "arrow", rea::Json("visible", false,
                                                "pole", true),
                             "face", 200,
                             "text", rea::Json("visible", false,
                                               "size", rea::JArray(100, 50),
                                               "location", "bottom"),
                             "color", "blue",
                             "objects", rea::Json(
                                            "img_2", rea::Json(
                                                         "type", "image",
                                                         "range", rea::JArray(0, 0, 400, 300),
                                                         "path", pth
                                                         ),
                                            "shp_0", rea::Json(
                                                         "type", "poly",
                                                         "points", rea::JArray(QJsonArray(),
                                                                               rea::JArray(50, 50, 200, 200, 200, 50, 50, 50),
                                                                               rea::JArray(80, 70, 120, 100, 120, 70, 80, 70)),
                                                         "color", "red",
                                                         "width", 3,
                                                         "caption", "poly",
                                                         "angle", 20,
                                                         "face", 50,
                                                         "text", rea::Json("visible", true,
                                                                           "size", rea::JArray(100, 50))
                                                         ),
                                            "shp_1", rea::Json(
                                                         "type", "ellipse",
                                                         "center", rea::JArray(400, 400),
                                                         "radius", rea::JArray(300, 200),
                                                         "width", 5,
                                                         "ccw", false,
                                                         "angle", 30,
                                                         "caption", "ellipse"
                                                         )
                                                ));

        auto view = aInput->data();
        for (auto i : view.keys())
            cfg.insert(i, view.value(i));

        if (count)
            aInput->out<QJsonObject>(cfg, "replaceQSGModel_testbrd");
        else
            aInput->out<std::shared_ptr<qsgModel>>(std::make_shared<qsgModel>(cfg), "updateQSGModel_testbrd");
        ++count;
    }, rea::Json("name", "testQSGShow"))->nextB(0, "replaceQSGModel_testbrd", QJsonObject())->next("updateQSGModel_testbrd");
}, QJsonObject(), "unitTest");
