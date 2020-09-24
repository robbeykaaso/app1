#ifndef REAL_APPLICATION2_Task_H_
#define REAL_APPLICATION2_Task_H_

#include "model.h"
#include "../socket/client.h"
#include "storage/storage.h"

class task;

class taskMode{
public:
    taskMode(task* aParent){
        m_parent = aParent;
    }
    virtual ~taskMode(){}
    std::function<void(void)> showQSGModel(int aChannel, stgCVMat& aImage);
    virtual bool showResult(){return true;}
    virtual void tryModifyCurrentModel(rea::stream<QJsonArray>* aInput);
    virtual void modifyRemoteModel(rea::stream<stgJson>* aInput);
protected:
    virtual std::function<void(void)> prepareQSGObjects(QJsonObject& aObjects);
    virtual void updateShowConfig(QJsonObject& aConfig) {}
    task* m_parent;
};

class roiMode : public taskMode{
public:
    roiMode(task* aParent) : taskMode(aParent){

    }
    ~roiMode() override;
    bool showResult() override {return false;}
    void tryModifyCurrentModel(rea::stream<QJsonArray>* aInput) override;
    void modifyRemoteModel(rea::stream<stgJson>* aInput) override;
protected:
    std::function<void(void)> prepareQSGObjects(QJsonObject& aObjects) override;
    void updateShowConfig(QJsonObject& aConfig) override;
};


class task : public imageModel{
private:
    friend taskMode;
    friend roiMode;
private:
    void setLabels(const QJsonObject& aLabels);
    QString getImageStage(const QJsonObject& aImage);
    void setImageStage(QJsonObject& aImage, const QString& aStage);
    QJsonObject getImages();
    void setImages(const QJsonObject& aImages);
    QJsonArray getImageList();
    void setImageList(const QJsonArray& aList, bool aRemove = false);
    QJsonObject prepareLabelListGUI(const QJsonObject& aLabels);
    QJsonObject prepareImageListGUI(const QJsonObject& aImages);
    QJsonObject prepareJobListGUI();
private:
    QString m_task_id = "";
    QJsonObject m_project_labels;
    QJsonObject* m_project_images;
    QString getJobsJsonPath();
    QString getTaskJsonPath();
    void taskManagement();
    void labelManagement();
private:
    std::shared_ptr<taskMode> m_task_mode;
    int m_channel_count;
    int m_show_count = 1;
    QJsonObject m_image_show;
    const QJsonObject selectTaskImage = rea::Json("tag", "selectTaskImage");
    void getResultShapeObjects(QJsonObject& aObjects);
    QJsonObject getDefaultROI();
    QJsonObject getROI(const QJsonObject& aTask);
    void setROI(QJsonObject& aTask, const QJsonObject& aROI);
    bool modifyROI(const QJsonArray& aModification, QJsonObject& aROI, QString& aPath);

    void imageManagement();
private:
    void guiManagement();
private:
    normalClient m_client;
    void serverManagement();
private:
    QJsonObject m_jobs;
    QString m_current_job = "";
    QJsonObject m_image_result;
    QJsonObject m_current_param;
    QString m_current_request = "";
    QHash<QString, QJsonArray> m_log_cache;

    double m_min_threshold = 0, m_max_threshold = 0.7;
    void insertJob(const QString& aID);

    QJsonObject prepareTrainingData();
    QString getJobState(const QJsonObject& aJob);
    void setJobState(QJsonObject& aJob, const QString& aState);
    QJsonArray getLossList(const QJsonObject& aStatistics);
    QJsonObject getHistogramData(const QJsonObject& aStatistics);
    QJsonObject prepareTrainParam();
    QJsonArray getPredictShapes(const QJsonObject& aImageResult);
    QJsonArray updateResultObjects(const QJsonObject& aImageResult, int aIndex);

    void jobManagement();
public:
    task();
};


#endif
