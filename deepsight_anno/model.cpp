#include "model.h"

QString model::getProjectName(const QJsonObject& aProject){
    return aProject.value("name").toString();
}
