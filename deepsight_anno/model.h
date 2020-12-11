#ifndef REAL_APPLICATION2_MODEL_H_
#define REAL_APPLICATION2_MODEL_H_

#include "reactive2.h"
#include "util/cv.h"
#include <QDateTime>

extern QString s3_bucket_name;

namespace rea{

template <typename T, typename F = pipeFunc<T>>
class pipeThrottle2 : public pipe<T, F> {
protected:
    pipeThrottle2(const QString& aName, int aThreadNo = 0, bool aReplace = false) : pipe<T, F>(aName, aThreadNo, aReplace) {

    }
    ~pipeThrottle2() override{
        if (m_timer >= 0)
            killTimer(m_timer);
    }
    bool event( QEvent* e) override{
        if(e->type()== pipe0::streamEvent::type){
            auto eve = reinterpret_cast<pipe0::streamEvent*>(e);
            if (eve->getName() == pipe0::m_name){
                auto stm = std::dynamic_pointer_cast<stream<T>>(eve->getStream());
                if (m_cache)
                    m_cache = stm;
                else{
                    doEvent(stm);
                    if (stm->template cacheData<bool>(0)){
                        m_cache = stm;
                        m_timer = startTimer(5);
                    }else{
                        stm->template cache<bool>(false)->template out<T>(stm->data(), "", QJsonObject(), false);
                        pipe0::doNextEvent(pipe0::m_next, stm);
                    }
                }
            }
        }else if (e->type() == QEvent::Timer){
            auto tm_e = reinterpret_cast<QTimerEvent*>(e);
            auto id = tm_e->timerId();
            if (m_timer == id){
                doEvent(m_cache);
                if (!m_cache->template cacheData<bool>(0)){
                    killTimer(id);
                    auto stm = m_cache;
                    m_cache = nullptr;
                    m_timer = - 1;
                    stm->template cache<bool>(false)->template out<T>(stm->data(), "", QJsonObject(), false);
                    pipe0::doNextEvent(pipe0::m_next, stm);
                }
            }
        }
        return true;
    }
private:
    std::shared_ptr<stream<T>> m_cache = nullptr;
    int m_timer = - 1;
    friend pipeline;
};

};


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
    QString getProjectName(const QJsonObject& aProject);
#define popWarning(aContent) \
    aInput->out<QJsonObject>(rea::Json("title", "warning", "text", aContent), "popMessage")
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
    void setShapeLabels(QJsonObject& aLabelGroups, const QJsonObject& aLabels);
    QJsonObject getImageLabels(const QJsonObject& aImageAbstract);
    void setImageLabels(QJsonObject& aImageAbstract, const QJsonObject& aLabels);
    QJsonArray getImageName(const QJsonObject& aImage);
    QString getImageStringName(const QJsonObject& aImage);
    QJsonObject getFilter();
    void setFilter(const QJsonObject& aFilter);
    bool modifyImage(const QJsonArray& aModification, QJsonObject& aImage, QString& aPath);
    void serviceLabelStatistics(const QString& aName);
    void serviceShowImageStatus(const QString aName, const QString& aChannel, QImage aImage, const QString& aImagePath);
    void serviceSelectFirstImageIndex(const QString aName);
    void sortImagesByTime(QJsonArray& aImageList, const QJsonObject& aImageAbstracts);
    virtual QJsonObject getImageAbstracts() = 0;
    virtual int getChannelCount() = 0;
    virtual QJsonObject getImageShow() = 0;
protected:
    QJsonArray m_transform;
    QString m_project_id = "";
    QJsonObject m_image;
    QString m_current_image = "";
    int m_first_image_index = 0; //the first grid image channel
    QJsonObject m_selects_cache;
    struct labelStatisticRec{
        int count = 0;
        QSet<QString> images;
    };
    using groupStatisticRec = QHash<QString, labelStatisticRec>;
private:
    QJsonObject m_show_selects_cache;
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
