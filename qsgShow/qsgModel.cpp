#include "qsgModel.h"

bool shapeObject::canBePickedUp(int aX, int aY){
    auto bnd = getBoundBox();
    return aX >= bnd.left() && aX <= bnd.right() && aY >= bnd.top() && aY <= bnd.bottom();
}

void shapeObject::calcBoundBox(double *aBoundBox, const pointList &aPoints){
    aBoundBox[0] = aBoundBox[2] = aPoints[0].x();
    aBoundBox[1] = aBoundBox[3] = aPoints[0].y();
    for (auto i : aPoints){
        auto x = i.x(), y = i.y();
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

void qsgModel::clearQSGObjects(){
    for (auto i : m_objects)
        i->QSGNodeRemoved();
}

void qsgModel::show(QSGNode& aTransform){
    for (auto i : m_objects)
        aTransform.appendChildNode(i->getQSGNode());
}

void qsgModel::deserialize(){

}
