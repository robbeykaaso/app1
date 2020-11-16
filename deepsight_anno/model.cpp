#include "model.h"
#include "qsgModel.h"
#include "../util/cv.h"
#include "storage/storage.h"
#include <QQuickItem>

QString s3_bucket_name = "deepsight";

QString model::getProjectName(const QJsonObject& aProject){
    return aProject.value("name").toString();
}

QJsonObject imageModel::getLabels(){
    return value("labels").toObject();
}

QJsonObject imageModel::getShapeLabels(const QJsonObject& aLabels){
    return aLabels.value("shape").toObject();
}

void imageModel::setShapeLabels(QJsonObject& aLabelGroups, const QJsonObject& aLabels){
    aLabelGroups.insert("shape", aLabels);
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
            aInput->out<rea::stgJson>(rea::stgJson(QJsonObject(), "project/" + m_project_id + "/image/" + i + ".json"));
    }, rea::Json("name", "calc" + aName + "LabelStatistics"))
        ->next(rea::local(s3_bucket_name + "readJson", rea::Json("thread", 10)))
        ->next(rea::pipeline::add<rea::stgJson>([this, aName](rea::stream<rea::stgJson>* aInput){
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
        ->next("showLabelStatistics")
        ->next("filter" + aName + "Images", rea::Json("tag", "filter" + aName + "Images"));
}

void imageModel::serviceShowImageStatus(const QString aName, const QString& aChannel, QImage aImage, const QString& aImagePath){
    //show mouse position and its pixel
    rea::pipeline::find("updateQSGPos_" + aName + "image_gridder" + aChannel)
        ->next(rea::pipeline::add<QJsonObject>([this, aName, aImage, aImagePath](rea::stream<QJsonObject>* aInput){
            auto dt = aInput->data();
            m_transform = dt.value("transform").toArray();

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

            aInput->out<QJsonObject>(rea::Json(dt, "coor", ret, "cmd", aName + "image_updateStatus"), "updateActualImagePixel" + aImagePath);
            //aInput->out<QJsonArray>(ret, aName + "image_updateStatus");
        }, rea::Json("name", "updateQSGPosMapShow_" + aChannel)));

    //show selected contours' statistics
    rea::pipeline::add<QJsonObject>([this, aImagePath](rea::stream<QJsonObject>* aInput){
        auto dt = aInput->data();
        m_selects_cache = dt;
        QJsonObject statistics = rea::Json("title", rea::JArray("key", "value"));
        if (dt.contains("shapes")){
            aInput->out<QJsonObject>(dt, "extractContourStatistics" + aImagePath);
        }else
            aInput->out<QJsonObject>(statistics, "region_info_updateListView");
        aInput->out<QJsonObject>(aInput->data());
    }, rea::Json("name", "updateQSGSelects_" + aName + "image_gridder" + aChannel, "replace", true));

    if (m_show_selects_cache.contains("shapes")){
        m_show_selects_cache.insert("invisible", true);
        rea::pipeline::run<QJsonObject>("updateQSGSelects_" + aName + "image_gridder0", m_show_selects_cache);
        m_show_selects_cache = QJsonObject();
    }
}

void imageModel::serviceSelectFirstImageIndex(const QString aName){
    rea::pipeline::add<QJsonObject>([this, aName](rea::stream<QJsonObject>* aInput){
        auto dt = aInput->data();
        auto chs = getChannelCount();
        if (dt.value("next").toBool()){
            m_current_image = "";
            m_first_image_index = std::min(m_first_image_index + 1, chs - 1);
        }else if (dt.value("previous").toBool()){
            m_current_image = "";
            m_first_image_index = std::max(m_first_image_index - 1, 0);
        }else if (dt.contains("index")){
            m_current_image = "";
            m_first_image_index = std::min(std::max(0, dt.value("index").toInt()), chs - 1);
        }
        aInput->out<QJsonArray>(QJsonArray(), aName + "_image_listViewSelected", rea::Json("tag", "manual"));
        aInput->out<double>(m_first_image_index);
        m_show_selects_cache = m_selects_cache;
    }, rea::Json("name", "switch" + aName + "FirstImageIndex"));
}

class imageObjectEx : public rea::imageObject{
private:
    cv::Mat m_image;
public:
    imageObjectEx(const QJsonObject& aConfig) : imageObject(aConfig){

    }
    rea::IUpdateQSGAttr updateQSGAttr(const QString& aModification) override{
        if (aModification == "transform_")
            return [this](QSGNode*){
                transformChanged();
            };
        else
            return imageObject::updateQSGAttr(aModification);
    }
    QImage updateImagePath(bool aForce = false) override{
        auto img = imageObject::updateImagePath(aForce);
        m_image = QImage2cvMat(img);
        return img;
    }
    void appendToParent(QSGNode* aTransformNode) override{
        if (contains("transform")){
            if (aTransformNode->parent())
                aTransformNode->parent()->insertChildNodeBefore(m_node, aTransformNode);
        }
        else
            imageObject::appendToParent(aTransformNode);
    }
    void transformChanged() override {
        if (!contains("transform"))
            return;
        if (!m_node->parent())
            return;
        auto cfg = value("transform").toObject();

        auto trans = reinterpret_cast<QSGTransformNode*>(m_node->parent()->childAtIndex(1))->matrix(),
             inv_trans = trans.inverted();
        auto origin0 = inv_trans.map(QPoint(0, 0)), origin1 = inv_trans.map(QPoint(m_window->width(), m_window->height()));

        int left = std::max(0, origin0.x()), bottom = std::max(0, origin0.y()),
            right = std::min(origin1.x(), m_image.cols), top = std::min(origin1.y(), m_image.rows);

        if (right <= left || top <= bottom)
            return;

        cv::Mat img0;
        auto fmt = cfg.value("colorFormat").toString();
        try{
            //auto src = QImage2cvMat(m_image);
            if (fmt == "BayerRG2RGB")
                cv::cvtColor(m_image, img0, cv::COLOR_BayerRG2BGR);
            else if (fmt == "BayerRG2Gray")
                cv::cvtColor(m_image, img0, cv::COLOR_BayerRG2GRAY);
            else if (fmt == "RGB2Gray")
                cv::cvtColor(m_image, img0, cv::COLOR_RGB2GRAY);
            else
                img0 = m_image.clone();
        }catch(...){
            img0 = m_image.clone();
            std::cout << "format changed fail!" << std::endl;
        }

        auto range = cv::Rect(std::max(origin0.x(), 0),
                              std::max(origin0.y(), 0),
                              std::min(right - left, origin1.x() - origin0.x()),
                              std::min(top - bottom, origin1.y() - origin0.y()));
        auto dst0 = img0(range);
        origin0 = trans.map(QPoint(left, bottom)), origin1 = trans.map(QPoint(right, top));
        cv::Mat img = cv::Mat::zeros(origin1.y() - origin0.y(), origin1.x() - origin0.x(), img0.type());

        auto md = cfg.value("resizeMode").toString();
        if (md == "nearest")
            cv::resize(dst0, img, img.size(), 0, 0, cv::InterpolationFlags::INTER_NEAREST);
        else if (md == "cubic")
            cv::resize(dst0, img, img.size(), 0, 0, cv::InterpolationFlags::INTER_CUBIC);
        else if (md == "area")
            cv::resize(dst0, img, img.size(), 0, 0, cv::InterpolationFlags::INTER_AREA);
        else if (md == "lanczos4")
            cv::resize(dst0, img, img.size(), 0, 0, cv::InterpolationFlags::INTER_LANCZOS4);
        else
            cv::resize(dst0, img, img.size(), 0, 0, cv::InterpolationFlags::INTER_LINEAR);
        m_node->setTexture(m_window->window()->createTextureFromImage(cvMat2QImage(img)));
        m_node->setRect(origin0.x(), origin0.y(), origin1.x() - origin0.x(), origin1.y() - origin0.y());
        m_node->markDirty(QSGNode::DirtyMaterial);
    }
};

static rea::regPip<QQmlApplicationEngine*> init_createimage([](rea::stream<QQmlApplicationEngine*>* aInput){
    rea::pipeline::add<QJsonObject, rea::pipePartial>([](rea::stream<QJsonObject>* aInput){
        aInput->out<std::shared_ptr<rea::qsgObject>>(std::make_shared<imageObjectEx>(aInput->data()));
    }, rea::Json("name", "create_qsgobject_imagebak"));
}, rea::Json("name", "create_qsgobject_image_0"), "regQML");

static rea::regPip<QJsonObject> init_set_imageshow([](rea::stream<QJsonObject>* aInput){
    aInput->setData(rea::Json("resizeMode", rea::JArray("linear", "nearest", "cubic", "area", "lanczos4"),
                              "colorFormat", rea::JArray("None", "BayerRG2RGB", "BayerRG2Gray", "RGB2Gray")))->out();
}, rea::Json("name", "setImageShow"));

static rea::regPip<std::vector<stgCVMat>, rea::pipePartial> init_custom_show_imgs([](rea::stream<std::vector<stgCVMat>>* aInput){
    aInput->cache<QJsonArray>(QJsonArray(), 1);
    aInput->cache<QJsonArray>(QJsonArray());
    aInput->out();
}, rea::Json("name", "imageShowFilter"));
//

/*static rea::regPip<std::vector<cv::Mat>> init_extract_contour_statistics([](rea::stream<std::vector<cv::Mat>>* aInput){
    QJsonArray data;
    data.push_back(rea::Json("entry", rea::JArray("hello", "world")));

    aInput->out<QJsonObject>(rea::Json("title", rea::JArray("key", "value"),
                                       "selects", data.size() > 0 ? rea::JArray(0) : QJsonArray(),
                                       "data", data), "region_info_updateListView");
}, rea::Json("name", "extractContourStatistics"));*/
