#include "model.h"
#include "qsgModel.h"
#include "../util/cv.h"
#include "storage/storage.h"
#include <QQuickItem>

QString model::getProjectName(const QJsonObject& aProject){
    return aProject.value("name").toString();
}

QJsonObject imageModel::getLabels(){
    return value("labels").toObject();
}

QJsonObject imageModel::getImageLabels(const QJsonObject& aImageAbstract){
    return aImageAbstract.value("image_label").toObject();
}

void imageModel::setImageLabels(QJsonObject& aImageAbstract, const QJsonObject& aLabels){
    aImageAbstract.insert("image_label", aLabels);
}

QJsonArray imageModel::getImageName(const QJsonObject& aImage){
    return aImage.value("name").toArray();
}
QString imageModel::getImageStringName(const QJsonObject& aImage){
    auto nms = getImageName(aImage);
    QString ret = "";
    if (nms.size() > 0){
        ret += nms[0].toString();
        if (nms.size() > 1)
            ret += "\n...";
    }
    return ret;
}

QJsonObject imageModel::getFilter(){
    return value("filter").toObject();
}

void imageModel::setFilter(const QJsonObject& aFilter){
    insert("filter", aFilter);
}

QJsonObject shapeModel::getShapes(const QJsonObject& aImage){
    return aImage.value("shapes").toObject();
}

void shapeModel::setShapes(QJsonObject& aImage, const QJsonObject& aShapes){
    aImage.insert("shapes", aShapes);
}

bool imageModel::modifyImage(const QJsonArray& aModification, QJsonObject& aImage, QString& aPath){
    bool modified = false;
    for (auto i : aModification){
        auto dt = i.toObject();
        if (dt.value("cmd").toBool()){
            if (dt.contains("id") && dt.value("id") != aPath){
                aPath = dt.value("id").toString();
                return false;
            }
            auto shps = getShapes(aImage);
            if (dt.value("key") == QJsonArray({"objects"})){
                if (dt.value("type") == "add"){
                    auto shp = dt.value("val").toObject();
                    auto key = dt.value("tar").toString();
                    if (shp.value("type") == "ellipse"){
                        auto r = shp.value("radius").toArray();
                        shps.insert(key, rea::Json("type", "ellipse",
                                                   "label", shp.value("caption"),
                                                   "center", shp.value("center"),
                                                   "xradius", r[0],
                                                   "yradius", r[1]));
                        setShapes(aImage, shps);
                        modified = true;
                    }else if (shp.value("type") == "poly"){
                        auto pts = shp.value("points").toArray();
                        QJsonArray holes;
                        for (int i = 1; i < pts.size(); ++i)
                            holes.push_back(pts[i]);
                        shps.insert(key, rea::Json("type", "polyline",
                                                   "label", shp.value("caption"),
                                                   "points", pts[0],
                                                   "holes", holes));
                        setShapes(aImage, shps);
                        modified = true;
                    }
                }else if (dt.value("type") == "del"){
                    shps.remove(dt.value("tar").toString());
                    setShapes(aImage, shps);
                    modified = true;
                }
            }else{
                auto nm = dt.value("obj").toString();
                if (shps.contains(nm)){
                    auto shp = shps.value(nm).toObject();
                    auto key = dt.value("key").toArray()[0].toString();
                    if (shp.value("type") == "ellipse"){
                        if (key == "caption"){
                            shp.insert("label", dt.value("val"));
                        }else if (key == "radius"){
                            auto r = dt.value("val").toArray();
                            shp.insert("xradius", r[0]);
                            shp.insert("yradius", r[1]);
                        }else
                            shp.insert(key, dt.value("val"));
                        shps.insert(nm, shp);
                        setShapes(aImage, shps);
                        modified = true;
                    }else if (shp.value("type") == "polyline"){
                        if (key == "caption"){
                            shp.insert("label", dt.value("val"));
                        }else if (key == "points"){
                            auto pts = dt.value("val").toArray();
                            shp.insert("points", pts[0]);
                            QJsonArray holes;
                            for (int j = 1; j < pts.size(); ++j)
                                holes.push_back(pts[j]);
                            shp.insert("holes", holes);
                        }else
                            shp.insert(key, dt.value("val"));
                        shps.insert(nm, shp);
                        setShapes(aImage, shps);
                        modified = true;
                    }
                }
            }
        }
    }
    return modified;
}
