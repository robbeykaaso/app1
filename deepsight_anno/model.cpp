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

void imageModel::serviceLabelStatistics(const QString& aName){
    using groupStatisticRec = QHash<QString, labelStatisticRec>;
    rea::pipeline::add<QJsonObject>([this](rea::stream<QJsonObject>* aInput){
        auto valid_lbls = std::make_shared<QHash<QString, QSet<QString>>>();
        auto grps = getLabels();
        for (auto i : grps.keys()){
            auto lbls = grps.value(i).toObject();
            for (auto j : lbls.keys())
                rea::tryFind(valid_lbls.get(), i)->insert(j);
        }

        auto imgs = getImageAbstracts();
        auto ret = std::make_shared<QHash<QString, groupStatisticRec>>();
        for (auto i : imgs.keys()){
            auto img = imgs.value(i).toObject();
            auto lbls = getImageLabels(img);
            if (lbls.size() > 0)
                for (auto j : lbls.keys()){
                    auto lbl = lbls.value(j).toString();
                    if (rea::tryFind(valid_lbls.get(), j)->contains(lbl)){
                        auto rec = rea::tryFind(rea::tryFind(ret.get(), j), lbl);
                        rec->count++;
                        rec->images.insert(i);
                    }
                }
            else{
                auto rec = rea::tryFind(rea::tryFind(ret.get(), QString("")), QString("no image"));
                rec->count++;
                rec->images.insert(i);
            }
        }

        aInput->cache<int>(0);
        aInput->cache<std::shared_ptr<QHash<QString, QSet<QString>>>>(valid_lbls);
        aInput->cache<std::shared_ptr<QHash<QString, groupStatisticRec>>>(ret);
        aInput->out<QJsonObject>(rea::Json("title", "statistics", "sum", imgs.size()), "updateProgress");
        for (auto i : imgs.keys())
            aInput->out<stgJson>(stgJson(QJsonObject(), "project/" + m_project_id + "/image/" + i + ".json"));
    }, rea::Json("name", "calc" + aName + "LabelStatistics"))
        ->nextB("updateProgress")
        ->next(rea::local("deepsightreadJson", rea::Json("thread", 10)))
        ->next(rea::pipeline::add<stgJson>([this, aName](rea::stream<stgJson>* aInput){
            auto imgs = getImageAbstracts();

            auto cnt = aInput->cacheData<int>(0);
            ++cnt;
            aInput->out<QJsonObject>(QJsonObject(), "updateProgress");
            if (cnt == imgs.size()){
                auto ret = aInput->cacheData<std::shared_ptr<QHash<QString, groupStatisticRec>>>(2);
                QJsonArray data;
                for (auto i : ret->keys()){
                    auto recs = ret->value(i);
                    for (auto j : recs.keys()){
                        auto rec = recs.value(j);
                        QJsonArray imgs;
                        for (auto k : rec.images)
                            imgs.push_back(k);
                        data.push_back(rea::Json("entry", rea::JArray(i, j, rec.count, imgs)));
                    }
                }

                aInput->out<QJsonObject>(rea::Json("title", rea::JArray("group", "label", "count"),
                                                   "selects", data.size() > 0 ? rea::JArray(0) : QJsonArray(),
                                                   "data", data,
                                                   "tag", "filter" + aName + "Images"), "showLabelStatistics");
            }else{
                aInput->cache<int>(cnt, 0);
                auto valid_lbls = aInput->cacheData<std::shared_ptr<QHash<QString, QSet<QString>>>>(1);
                auto ret = aInput->cacheData<std::shared_ptr<QHash<QString, groupStatisticRec>>>(2);
                auto valid_lbls2 = rea::tryFind(valid_lbls.get(), QString("shape"));
                auto shps = aInput->data().getData().value("shapes").toObject();
                if (shps.size() > 0)
                    for (auto i : shps){
                        auto lbl = i.toObject().value("label").toString();
                        if (valid_lbls2->contains(lbl)){
                            auto rec = rea::tryFind(rea::tryFind(ret.get(), QString("shape")), lbl);
                            rec->count++;
                            rec->images.insert(*(imgs.keys().begin() + cnt - 1));
                        }
                    }
                else{
                    auto rec = rea::tryFind(rea::tryFind(ret.get(), QString("")), QString("no shape"));
                    rec->count++;
                    rec->images.insert(*(imgs.keys().begin() + cnt - 1));
                }
            }
        }))
        ->nextB("updateProgress")
        ->next("showLabelStatistics")
        ->next("filter" + aName + "Images", rea::Json("tag", "filter" + aName + "Images"));
}

void imageModel::serviceShowPosStatus(const QString aName, const QString& aChannel, QImage aImage){
    rea::pipeline::find("updateQSGPos_" + aName + "image_gridder" + aChannel)
        ->next(rea::pipeline::add<QJsonObject>([aName, aImage](rea::stream<QJsonObject>* aInput){
            auto dt = aInput->data();
            QJsonArray ret;
            auto x = dt.value("x").toInt(), y= dt.value("y").toInt();
            ret.push_back("x: " + QString::number(x) + "; " +
                          "y: " + QString::number(y));
            if (x >= 0 && x < aImage.width() && y >= 0 && y < aImage.height()){
                auto clr = aImage.pixelColor(x, y);
                ret.push_back("r: " + QString::number(clr.red()) + "; " +
                              "g: " + QString::number(clr.green()) + "; " +
                              "b: " + QString::number(clr.blue()));
            }
            aInput->out<QJsonArray>(ret, aName + "image_updateStatus");
        }, rea::Json("name", "updateQSGPosMapShow_" + aChannel)))
        ->next(aName + "image_updateStatus");
}
