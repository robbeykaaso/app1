#ifndef REAL_APPLICATION2_MODEL_H_
#define REAL_APPLICATION2_MODEL_H_

#include "reactive2.h"
#include "util/cv.h"
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
    virtual ~model(){}
protected:
    const QString s3_bucket_name = "deepsight";
    QString getProjectName(const QJsonObject& aProject);
};

class shapeModel{
public:
    QJsonObject getShapes(const QJsonObject& aImage);
    void setShapes(QJsonObject& aImage, const QJsonObject& aShapes);
};

class imageModel : public model, public shapeModel{
protected:
    QJsonObject getLabels();
    QJsonObject getShapeLabels(const QJsonObject& aLabels);
    QJsonObject getImageLabels(const QJsonObject& aImageAbstract);
    void setImageLabels(QJsonObject& aImageAbstract, const QJsonObject& aLabels);
    QJsonArray getImageName(const QJsonObject& aImage);
    QString getImageStringName(const QJsonObject& aImage);
    QJsonObject getFilter();
    void setFilter(const QJsonObject& aFilter);
    bool modifyImage(const QJsonArray& aModification, QJsonObject& aImage, QString& aPath);
    void serviceLabelStatistics(const QString& aName);
    void serviceShowImageStatus(const QString aName, const QString& aChannel, QImage aImage);
    void serviceSelectFirstImageIndex(const QString aName);
    virtual QJsonObject getImageAbstracts() = 0;
    virtual int getChannelCount() = 0;
protected:
    QJsonArray m_transform;
    QString m_project_id = "";
    QJsonObject m_image;
    QString m_current_image = "";
    int m_first_image_index = 0; //the first grid image channel
private:
    struct labelStatisticRec{
        int count = 0;
        QSet<QString> images;
    };
};

class IProjectInfo : public QJsonObject{
public:
    IProjectInfo(QJsonObject* aProjectImages, const QJsonObject& aProjectInfo) : QJsonObject(aProjectInfo){
        project_images = aProjectImages;
    }
    IProjectInfo() : QJsonObject(){}
    QJsonObject* project_images;
};

#endif
