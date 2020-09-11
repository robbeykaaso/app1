#include "model.h"
#include "qsgModel.h"

QString model::getProjectName(const QJsonObject& aProject){
    return aProject.value("name").toString();
}

class imageObjectEx : public rea::imageObject{
public:
    imageObjectEx(const QJsonObject& aConfig) : imageObject(aConfig){

    }
    void transformChanged() override {
//auto trans = reinterpret_cast<QSGTransformNode*>(m_outline->parent())->matrix();
    }
};

static rea::regPip<QJsonObject, rea::pipePartial> create_image([](rea::stream<QJsonObject>* aInput){
    aInput->out<std::shared_ptr<rea::qsgObject>>(std::make_shared<imageObjectEx>(aInput->data()));
}, rea::Json("name", "create_qsgobject_image"));
