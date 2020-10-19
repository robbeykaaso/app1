#ifndef REAL_APPLICATION2_Task_H_
#define REAL_APPLICATION2_Task_H_

#include "model.h"
#include "../socket/client.h"
#include "storage/storage.h"

class task;

class ITaskFriend{
public:
    ITaskFriend(task* aTask){
        m_task = aTask;
    }
protected:
    QJsonObject getShapeLabels(const QJsonObject& aLabels);
    bool modifyImage(const QJsonArray& aModification, QJsonObject& aImage, QString& aPath);
    QString getTaskJsonPath();
    QString getImageJsonPath();
    QString getImageResultJsonPath();
    QJsonObject& getImageData();
    QString getImageID();
    QString getTaskID();
    QString getProjectID();
    QJsonObject getTaskLabels();
    QJsonObject getProjectLabels();
    QJsonObject getImageShow();
    bool belongThisMode(const QString& aMode, const QString& aPath);
    bool isCurrentMode(const QString& aMode);
    int getShowCount();
    void updateCurrentImage();
    void serviceShowPosStatus(const QString aName, const QString& aChannel, QImage aImage);
    task* m_task;
private:
    QHash<QString, bool> m_paths;
};

class taskMode : public shapeModel, public ITaskFriend{
public:
    taskMode(task* aTask) : ITaskFriend(aTask){}
    virtual ~taskMode(){}
    virtual void initialize();
protected:
    std::function<void(void)> showQSGModel(int aChannel, stgCVMat& aImage);
    virtual std::function<void(void)> prepareQSGObjects(QJsonObject& aObjects);
    virtual void updateShowConfig(QJsonObject& aConfig) {}
};

class task : public imageModel{
private:
    friend ITaskFriend;
protected:
    QJsonObject getImageAbstracts() override{
        return *m_project_images;
    }
    int getChannelCount() override{
        return m_channel_count;
    }
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
    QString m_task_type = "";
    QJsonObject m_project_labels;
    QJsonObject* m_project_images;
    QString getJobsJsonPath();
    QString getTaskJsonPath();
    QString getImageResultJsonPath();
    void taskManagement();
    void labelManagement();
private:
    QString m_current_mode;
    QSet<QString> m_custom_modes;
    int m_channel_count;
    int m_show_count = 1;
    QJsonObject m_image_show;
    const QJsonObject selectTaskImage = rea::Json("tag", "selectTaskImage");
    void imageManagement();
    bool isCurrentMode(const QString& aMode);
private:
    void guiManagement();
private:
    normalClient m_client;
    void serverManagement();
private:
    QJsonObject m_jobs;
    QString m_current_job = "";
    QJsonObject m_image_result;
    QString m_current_request = "";
    QHash<QString, QJsonArray> m_log_cache;

    void insertJob(const QString& aID);
    QJsonObject prepareTrainingData(const QJsonArray& aImages);
    QString getJobState(const QJsonObject& aJob);
    void setJobState(QJsonObject& aJob, const QString& aState);
    QJsonArray getLossList(const QJsonObject& aStatistics);
    QJsonObject getHistogramData(const QJsonObject& aStatistics);
    QJsonArray getPredictShapes(const QJsonObject& aImageResult);
    QJsonArray updateResultObjects(const QJsonObject& aImageResult, int aIndex);

    void jobManagement();
private:
    void updateStatisticsModel(const QJsonObject& aStatistics);
    int getIOUIndex(double aIOU);
    int getThresholdIndex(double aThreshold);
    int calcThresholdIndex();
    QString getImagePredict(const QString& aImageID, const QJsonObject& aImageResult);
    void prepareTrainParam(QJsonObject& aParam);
    QJsonObject m_statistics;
    double m_min_threshold = 0, m_max_threshold = 0.7;
    double m_iou = 0;
    bool m_for_image = true;
    std::vector<double> m_threshold_list;
    std::vector<double> m_iou_list;
    QMap<QString, QMap<QString, QJsonArray>> m_image_level_statistics;
    QMap<QString, QMap<QString, QJsonArray>> m_instance_level_statistics;
    QMap<QString, QJsonObject> m_images_statistics;
    QMap<QString, QJsonObject> m_instances_statistics;
public:
    task();
};


#endif
