#ifndef REAL_APPLICATION2_MODEL_H_
#define REAL_APPLICATION2_MODEL_H_

#include "reactive2.h"
#include <QDateTime>

class model : public QJsonObject{
public:
/*#define ABSTRACT(MODEL) \
    QJsonObject get##MODEL##s(){  \
        return value(QString(STR(MODEL##_abstract)).toLower()).toObject(); \
    } \
    void set##MODEL##s(const QJsonObject& a##MODEL##s){ \
        insert(QString(STR(MODEL##_abstract)).toLower(), a##MODEL##s); \
    } \
    void insert##MODEL(QJsonObject& a##MODEL){ \
        auto mdls = get##MODEL##s(); \
        auto tm0 = QDateTime::currentDateTime(); \
        auto tm = tm0.toString(Qt::DateFormat::ISODate); \
        auto tms = tm.split("T"); \
        mdls.insert(QString::number(tm0.toTime_t()) + "_" + rea::generateUUID(), rea::Json(a##MODEL, "time", tms[0] + " " + tms[1])); \
        set##MODEL##s(mdls); \
    } \
    QJsonObject prepare##MODEL##ListGUI(const QJsonObject& a##MODEL##s){ \
        QJsonArray data; \
        for (auto i : a##MODEL##s) \
            data.push_back(rea::Json("entry", rea::JArray(i.toObject().value("name")))); \
        return rea::Json("title", rea::JArray("name"),  \
                         "selects", a##MODEL##s.size() > 0 ? rea::JArray("0") : QJsonArray(),  \
                         "data", data);  \
    }*/
protected:
    QString getProjectName(const QJsonObject& aProject);
    QJsonObject getLabels();
    QJsonObject prepareLabelListGUI(const QJsonObject& aLabels);
};

#endif
