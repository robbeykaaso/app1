#include "../util/cv.h"
#include "storage/storage.h"
#include "imagePool.h"

//P.S. "param"只支持json对象，不支持其他类型

class optSrc : public QJsonObject{
public:
    optSrc(std::vector<cv::Mat> aImages, const QJsonObject& aParam = QJsonObject()) : QJsonObject(aParam) {images = aImages;}
    optSrc(){}
    std::vector<cv::Mat> images;
};

class imageOperations{
private:
    const QString binarization = "binarization_demo";
    const QString crop = "crop_demo";
private:
    QJsonObject getOperationAbstract(){
        return rea::Json(binarization,
                         rea::Json("caption", "binarization",
                                   "param", rea::Json("threshold", 125)),
                         crop,
                         rea::Json("caption", "crop",
                                   "param", rea::Json("range", rea::JArray(0, 0, 100, 100))));
    }
public:
    imageOperations(){
        rea::pipeline::find("loadImageOperations")
            ->next(rea::pipeline::add<QJsonObject>([this](rea::stream<QJsonObject>* aInput){
                aInput->out<QJsonObject>(getOperationAbstract(), "imageOperationsLoaded");
            }))
            ->next("imageOperationsLoaded");

        if (!rea::pipeline::find(binarization, false))
            rea::pipeline::add<optSrc>([](rea::stream<optSrc>* aInput){
                cv::Mat ret;
                auto th = aInput->data().value("threshold").toDouble();
                auto imgs = aInput->data().images;
                threshold(imgs[0], ret, th, 255, cv::THRESH_BINARY);
                imgs[0] = ret;
                aInput->setData(optSrc(imgs))->out();
            }, rea::Json("name", binarization));

        if (!rea::pipeline::find(crop, false))
            rea::pipeline::add<optSrc>([](rea::stream<optSrc>* aInput){
                auto rg = aInput->data().value("range").toArray();
                auto imgs = aInput->data().images;
                imgs[0] = imgs[0](cv::Rect(rg[0].toDouble(), rg[1].toDouble(), rg[2].toDouble(), rg[3].toDouble()));
                aInput->setData(optSrc(imgs))->out();
            }, rea::Json("name", crop));
    }
};

class imageTransform{
private:
    QJsonObject m_image_operations;
    QStringList m_used_operations;
    QJsonArray m_current_operations;
    QJsonArray m_current_operations_cfg;
    int m_select_operator = - 1;
    rea::pipe0* m_image_src;
    rea::pipe0* m_map_image;
    QJsonObject m_image_show;
    QJsonArray m_images;
    std::vector<cv::Mat> m_show_cache;
    QJsonObject prepareImageOperationListGUI(const QString& aTitle, const QJsonObject& aOperations){
        QJsonArray data;
        for (auto i : aOperations.keys())
            data.push_back(rea::Json("entry", rea::JArray(aOperations.value(i).toObject().value("caption"))));
        return rea::Json("title", rea::JArray(aTitle),
                         "selects", aOperations.size() > 0 ? rea::JArray("0") : QJsonArray(),
                         "data", data);
    }

    QJsonObject prepareUsedOperationListGUI(){
        QJsonArray data;
        for (auto i : m_current_operations)
            data.push_back(rea::Json("entry", rea::JArray(m_image_operations.value(i.toString()).toObject().value("caption"))));
        return rea::Json("title", rea::JArray("used"),
                         "selects", m_current_operations.size() > 0 ? rea::JArray("0") : QJsonArray(),
                         "data", data);
    }

    QJsonObject matToShow(const QString& aPath, const cv::Mat& aMat){
        auto img = cvMat2QImage(aMat);
        rea::imagePool::cacheImage(aPath, img);

        auto objs = rea::Json("img_background", rea::Json("type", "image",
                                                "range", rea::JArray(0, 0, img.width(), img.height()),
                                                "path", aPath,
                                                "transform", m_image_show));
        return rea::Json("width", img.width(),
                         "height", img.height(),
                         "objects", objs);
    }

    QJsonValue modifyOperator(const QJsonValue& aTarget, const QStringList& aKeys, const int aIndex, const QJsonValue aValue){
        if (aTarget.isObject()){
            auto tar = aTarget.toObject();
            if (aIndex == aKeys.size() - 1)
                tar.insert(aKeys[aIndex], aValue);
            else
                tar.insert(aKeys[aIndex], modifyOperator(tar.value(aKeys[aIndex]), aKeys, aIndex + 1, aValue));
            return tar;
        }else if (aTarget.isArray()){
            auto tar = aTarget.toArray();
            if (aIndex == aKeys.size() - 1)
                tar[aKeys[aIndex].toInt()] = aValue;
            else
                tar[aKeys[aIndex].toInt()] = modifyOperator(tar[aKeys[aIndex].toInt()], aKeys, aIndex + 1, aValue);
            return tar;
        }else
            assert(0);
    }
public:
    imageTransform(){
        const QString transformImage_nm = "transformImage";

        //trig show
        m_image_src =
            rea::pipeline::add<QJsonObject>([this](rea::stream<QJsonObject>* aInput){
                           auto dt = aInput->data();
                           m_image_show = dt.value("show").toObject();
                           m_images = dt.value("images").toArray();
                           for (auto i : m_images)
                               aInput->out<stgCVMat>(stgCVMat(cv::Mat(), i.toString()));
                       }, rea::Json("name", "showTransformImage"))
                ->next(rea::local("deepsightreadCVMat", rea::Json("thread", 10)))
                ->next(rea::pipeline::add<stgCVMat>([this](rea::stream<stgCVMat>* aInput){
                    auto dt = aInput->data();
                    m_show_cache.push_back(dt.getData());
                    if (m_show_cache.size() == m_images.size()){
                        aInput->out<QJsonObject>(matToShow("transformImage0", m_show_cache[0]), "updateQSGModel_imagebefore");
                        aInput->out<optSrc>(optSrc(m_show_cache));
                        m_show_cache.clear();
                    }
                }))
                ->nextB(0, "updateQSGModel_imagebefore", QJsonObject());

        //map mat to show
        m_map_image = rea::pipeline::add<optSrc>([this](rea::stream<optSrc>* aInput){
                          aInput->out<QJsonObject>(matToShow("img_result", aInput->data().images[0]), "updateQSGModel_imageafter");
                      })
                          ->nextB(0, "updateQSGModel_imageafter", QJsonObject());

        //add operators
        rea::pipeline::find("image_operation_listViewSelected")
            ->next(rea::pipeline::add<QJsonArray>([this](rea::stream<QJsonArray>* aInput){
                       auto sels = aInput->data();
                       for (auto i : sels){
                           m_current_operations.push_back(m_image_operations.keys()[i.toInt()]);
                           m_current_operations_cfg.push_back((m_image_operations.begin() + i.toInt()).value().toObject().value("param").toObject());
                       }
                       aInput->out<QJsonObject>(prepareUsedOperationListGUI(), "used_operation_updateListView");
                   }), rea::Json("tag", "addOperation"))
            ->next("used_operation_updateListView");

        //select operator
        rea::pipeline::find("used_operation_listViewSelected")
            ->next(rea::pipeline::add<QJsonArray>([this](rea::stream<QJsonArray>* aInput){
                       auto dt = aInput->data();
                       if (dt.size() > 0){
                           m_select_operator = dt[0].toInt();
                           aInput->out<QJsonObject>(rea::Json("data",
                                                              (m_current_operations_cfg.begin() + m_select_operator)->toObject()),
                                                    "imageoperatorloadTreeView");
                       }else
                           m_select_operator = - 1;
                   }), rea::Json("tag", "manual"))
            ->next("imageoperatorloadTreeView");

        //modify operators
        rea::pipeline::find("imageoperatortreeViewGUIModified")
            ->next(rea::pipeline::add<QJsonObject>([this](rea::stream<QJsonObject>* aInput){
                auto dt = aInput->data();
                if (m_select_operator >= 0){
                    auto prm = (m_current_operations_cfg.begin() + m_select_operator)->toObject();
                    auto keys = dt.value("key").toString().split(";");
                    m_current_operations_cfg[m_select_operator] = modifyOperator(prm, keys, 1, dt.value("val"));
                }
            }));

        //delete operators
        rea::pipeline::find("used_operation_listViewSelected")
            ->next(rea::pipeline::add<QJsonArray>([this](rea::stream<QJsonArray>* aInput){
                       auto dt = aInput->data();
                       std::vector<int> idxes;
                       for (auto i : dt)
                           idxes.push_back(i.toInt());
                       std::sort(idxes.begin(), idxes.end(), std::greater<int>());
                       for (auto i : idxes){
                           m_current_operations.removeAt(i);
                           m_current_operations_cfg.removeAt(i);
                       }
                       aInput->out<QJsonObject>(prepareUsedOperationListGUI(), "used_operation_updateListView");
                   }), rea::Json("tag", "deleteOperation"))
            ->next("used_operation_updateListView");

        //connect operators
        rea::pipeline::add<QJsonArray>([this](rea::stream<QJsonArray>* aInput){
            for (int i = m_used_operations.size() - 2; i >= 0; --i)
                rea::pipeline::find(m_used_operations[i])->removeNext(m_used_operations[i + 1]);
            for (int i = 1; i < m_used_operations.size() - 1; ++i)
                rea::pipeline::remove(m_used_operations[i]);
            m_used_operations.clear();
            m_used_operations.push_back(m_image_src->actName());
            for (int i = 0; i < m_current_operations.size(); ++i){
                auto prm = (m_current_operations_cfg.begin() + i)->toObject();
                auto pip = rea::pipeline::add<optSrc>([prm](rea::stream<optSrc>* aInput){
                    aInput->setData(optSrc(aInput->data().images, prm))->out();
                });
                m_used_operations.push_back(pip->actName());
                m_used_operations.push_back(rea::local(m_current_operations[i].toString())->actName());
            }
            m_used_operations.push_back(m_map_image->actName());
            for (int i = 0; i < m_used_operations.size() - 1; ++i)
                rea::pipeline::find(m_used_operations[i])->next(m_used_operations[i + 1]);
            aInput->out<QJsonObject>(rea::Json("images", m_images, "show", m_image_show), "showTransformImage");
        }, rea::Json("name", "transformImage"))
            ->next("showTransformImage");

        //load operators
        rea::pipeline::add<QJsonObject>([this](rea::stream<QJsonObject>* aInput){
            aInput->out();
        }, rea::Json("name", "loadImageOperations"));

        rea::pipeline::add<QJsonObject>([this](rea::stream<QJsonObject>* aInput){
            auto dt = aInput->data();
            for (auto i : dt.keys())
                m_image_operations.insert(i, dt.value(i));
            aInput->out<QJsonObject>(prepareImageOperationListGUI("operators", m_image_operations), "image_operation_updateListView");
        }, rea::Json("name", "imageOperationsLoaded"))
            ->nextB(0, "image_operation_updateListView", QJsonObject());
    }
};

/*static rea::regPip<QQmlApplicationEngine*> init_trans_img([](rea::stream<QQmlApplicationEngine*>* aInput){
    static imageTransform img_transform;
    static imageOperations img_operations;
    aInput->out();
}, QJsonObject(), "regQML");*/
