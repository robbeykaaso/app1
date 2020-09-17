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

QJsonObject imageModel::getShapes(const QJsonObject& aImage){
    return aImage.value("shapes").toObject();
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

void imageModel::setShapes(QJsonObject& aImage, const QJsonObject& aShapes){
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

void imageModel::modifyImage0(const QString& aName){
    //modify image
    rea::pipeline::find("QSGAttrUpdated_" + aName + "image_gridder0")
        ->next(rea::pipeline::add<QJsonArray>([this](rea::stream<QJsonArray>* aInput){
            if (m_current_image == "")
                return;
            QString cur = "project/" + m_project_id + "/image/" + m_current_image + ".json";
            QString pth = cur;
            if (modifyImage(aInput->data(), m_image, pth))
                aInput->out<stgJson>(stgJson(m_image, pth), "deepsightwriteJson");
            else if (pth != cur){
                aInput->cache<QJsonArray>(aInput->data())->out<stgJson>(stgJson(QJsonObject(), pth));
            }
        }))
        ->nextB(0, "deepsightwriteJson", QJsonObject())
        ->next(rea::local("deepsightreadJson", rea::Json("thread", 10)))
        ->next(rea::pipeline::add<stgJson>([this](rea::stream<stgJson>* aInput){
            auto dt = aInput->data().getData();
            auto pth = QString(aInput->data());
            modifyImage(aInput->cacheData<QJsonArray>(0), dt, pth);
            aInput->out<stgJson>(stgJson(dt, pth), "deepsightwriteJson");
        }))
        ->next("deepsightwriteJson");
}

class imageObjectEx : public rea::imageObject{
private:
    QImage m_image;
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
        m_image = img;
        return img;
    }
    void appendToParent(QSGNode* aTransformNode) override{
        if (contains("transform"))
            aTransformNode->parent()->insertChildNodeBefore(m_node, aTransformNode);
        else
            imageObject::appendToParent(aTransformNode);
    }
    void transformChanged() override {
        if (!contains("transform"))
            return;
        auto cfg = value("transform").toObject();

        auto trans = reinterpret_cast<QSGTransformNode*>(m_node->parent()->childAtIndex(1))->matrix(),
             inv_trans = trans.inverted();
        auto origin0 = inv_trans.map(QPoint(0, 0)), origin1 = inv_trans.map(QPoint(m_window->width(), m_window->height()));

        int left = std::max(0, origin0.x()), bottom = std::max(0, origin0.y()),
            right = std::min(origin1.x(), m_image.width()), top = std::min(origin1.y(), m_image.height());

        if (right <= left || top <= bottom)
            return;

        cv::Mat img0;
        auto fmt = cfg.value("colorFormat").toString();
        try{
            auto src = QImage2cvMat(m_image);
            if (fmt == "BayerRG2RGB")
                cv::cvtColor(src, img0, cv::COLOR_BayerRG2BGR);
            else if (fmt == "BayerRG2Gray")
                cv::cvtColor(src, img0, cv::COLOR_BayerRG2GRAY);
            else if (fmt == "RGB2Gray")
                cv::cvtColor(src, img0, cv::COLOR_RGB2GRAY);
            else
                img0 = src;
        }catch(...){
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
/*
static rea::regPip<QJsonObject, rea::pipePartial> create_image([](rea::stream<QJsonObject>* aInput){
    aInput->out<std::shared_ptr<rea::qsgObject>>(std::make_shared<imageObjectEx>(aInput->data()));
}, rea::Json("name", "create_qsgobject_image"));*/
