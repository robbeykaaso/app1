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
    bool modifyImage(const QJsonArray& aModification, QJsonObject& aImage, QString& aPath);
    QString getTaskJsonPath();
    QString getImageJsonPath();
    QJsonObject& getImageData();
    QString getImageID();
    QString getTaskID();
    QJsonObject getTaskLabels();
    QJsonObject getProjectLabels();
    QJsonObject getImageShow();
    task* m_task;
};

class taskMode : public shapeModel, public ITaskFriend{
public:
    taskMode(task* aTask) : ITaskFriend (aTask){
    }
    virtual ~taskMode(){}
    std::function<void(void)> showQSGModel(int aChannel, stgCVMat& aImage);
    virtual bool showResult(){return true;}
    virtual void tryModifyCurrentModel(rea::stream<QJsonArray>* aInput);
    virtual void modifyRemoteModel(rea::stream<stgJson>* aInput);
protected:
    virtual std::function<void(void)> prepareQSGObjects(QJsonObject& aObjects);
    virtual void updateShowConfig(QJsonObject& aConfig) {}
};

class roiMode : public taskMode{
public:
    roiMode(task* aParent);
    bool showResult() override {return false;}
    void tryModifyCurrentModel(rea::stream<QJsonArray>* aInput) override;
    void modifyRemoteModel(rea::stream<stgJson>* aInput) override;
protected:
    std::function<void(void)> prepareQSGObjects(QJsonObject& aObjects) override;
    void updateShowConfig(QJsonObject& aConfig) override;
private:
    QJsonObject getROI(const QJsonObject& aTask);
    void setROI(QJsonObject& aTask, const QJsonObject& aROI);
    QJsonObject getDefaultROI();
    bool modifyROI(const QJsonArray& aModification, QJsonObject& aROI, QString& aPath);
};


class task : public imageModel{
private:
    friend ITaskFriend;
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
    std::shared_ptr<taskMode> getCurrentMode();
    QHash<QString, std::shared_ptr<taskMode>> m_modes;
    QString m_current_mode;
    int m_channel_count;
    int m_show_count = 1;
    QJsonObject m_image_show;
    const QJsonObject selectTaskImage = rea::Json("tag", "selectTaskImage");
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
private:
    void updateStatisticsModel(const QJsonObject& aStatistics);
    int getIOUIndex(double aIOU);
    int getThresholdIndex(double aThreshold);
    int calcThresholdIndex();
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
