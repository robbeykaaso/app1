#include "qsgModel.h"

#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/convex_hull_2.h>
#include <CGAL/Polygon_2_algorithms.h>

typedef CGAL::Exact_predicates_inexact_constructions_kernel K;
typedef K::Point_2 Point_2;

bool pointIsIn(const std::vector<rea::pointList>& aPoints, const Point_2& aPoint){
    for (auto i = aPoints.begin(); i != aPoints.end(); ++i){
        std::vector<Point_2> pts;
        for (auto j : *i)
            pts.push_back(Point_2(j.x(), j.y()));
        if (i == aPoints.begin()){
            if (CGAL::bounded_side_2(pts.begin(), pts.end(), aPoint) == CGAL::ON_UNBOUNDED_SIDE)
                return false;
        }else if (CGAL::bounded_side_2(pts.begin(), pts.end(), aPoint) == CGAL::ON_BOUNDED_SIDE)
            return false;
    }
}

class polyObjectEx : public rea::polyObject{
public:
    polyObjectEx(const QJsonObject& aConfig) : polyObject(aConfig){

    }
    bool bePointSelected(double aX, double aY) override{
        return shapeObject::bePointSelected(aX, aY) && pointIsIn(m_points, Point_2(aX, aY));
    }
};

static rea::regPip<QJsonObject, rea::pipePartial> create_poly([](rea::stream<QJsonObject>* aInput){
    aInput->out<std::shared_ptr<rea::qsgObject>>(std::make_shared<polyObjectEx>(aInput->data()));
}, rea::Json("name", "create_qsgobject_poly"));

class ellipseObjectEx : public rea::ellipseObject{
public:
    ellipseObjectEx(const QJsonObject& aConfig) : rea::ellipseObject(aConfig){

    }
    bool bePointSelected(double aX, double aY) override{
        return shapeObject::bePointSelected(aX, aY) && pointIsIn(m_points, Point_2(aX, aY));
    }
};

static rea::regPip<QJsonObject, rea::pipePartial> create_ellipse([](rea::stream<QJsonObject>* aInput){
    aInput->out<std::shared_ptr<rea::qsgObject>>(std::make_shared<ellipseObjectEx>(aInput->data()));
}, rea::Json("name", "create_qsgobject_ellipse"));
