#include "model.h"
#include "qsgModel.h"
#include "../util/cv.h"
#include <QQuickItem>

QString model::getProjectName(const QJsonObject& aProject){
    return aProject.value("name").toString();
}

QJsonObject model::getLabels(){
    return value("labels").toObject();
}

QJsonObject model::getImageLabels(const QJsonObject& aImageAbstract){
    return aImageAbstract.value("image_label").toObject();
}

QJsonObject model::getShapes(const QJsonObject& aImage){
    return aImage.value("shapes").toObject();
}

QJsonArray model::getImageName(const QJsonObject& aImage){
    return aImage.value("name").toArray();
}
QString model::getImageStringName(const QJsonObject& aImage){
    auto nms = getImageName(aImage);
    QString ret = "";
    if (nms.size() > 0){
        ret += nms[0].toString();
        if (nms.size() > 1)
            ret += "\n...";
    }
    return ret;
}

QJsonObject model::getFilter(){
    return value("filter").toObject();
}

void model::setFilter(const QJsonObject& aFilter){
    insert("filter", aFilter);
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
