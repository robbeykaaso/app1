#include "qsgBoard.h"
#include "util/cv.h"
#include "imagePool.h"
#include "command.h"
#include <QPainter>

class qsgPluginDrawFree : public rea::qsgPluginTransform{
public:
    qsgPluginDrawFree(const QJsonObject& aConfig) : qsgPluginTransform(aConfig){

    }
private:
    void updateMask(){
        auto bnd = m_handles[0]->getBoundBox();
        auto inv = m_transnode->matrix().inverted();
        auto lt = inv.map(bnd.topLeft()).toPoint(),
             rb = inv.map(bnd.bottomRight()).toPoint();

        auto img = QImage(rb.x() - lt.x() + 1, rb.y() - lt.y() + 1, QImage::Format_ARGB32);
        img.fill(QColor("transparent"));
        QPainter pt(&img);
        pt.setPen(QPen(QColor("transparent")));
        pt.setBrush(QBrush(QColor(255, 0, 0, 125)));
        pt.drawEllipse(0, 0, img.width(), img.height());

        if (m_img.isNull()){
            m_shape = "shp_" + rea::generateUUID();
            m_img = img;
            m_lt = lt;
            m_rb = rb;
            rea::imagePool::cacheImage("drawFree", m_img);
            rea::pipeline::run("updateQSGAttr_" + getParentName(), rea::Json("key", rea::JArray("objects"),
                                                                             "type", "add",
                                                                             "tar", m_shape,
                                                                             "val", rea::Json(
                                                                                        "type", "image",
                                                                                        "path", "drawFree",
                                                                                        "range", rea::JArray(m_lt.x(), m_lt.y(), m_rb.x(), m_rb.y())
                                                                                            )));
        }else{
            auto n_lt = QPoint(std::min(m_lt.x(), lt.x()), std::min(m_lt.y(), lt.y())), n_rb = QPoint(std::max(m_rb.x(), rb.x()), std::max(m_rb.y(), rb.y()));
            auto n_img = QImage(n_rb.x() - n_lt.x() + 1, n_rb.y() - n_lt.y() + 1, QImage::Format_ARGB32);
            n_img.fill(QColor("transparent"));
            QPainter pt2(&n_img);
            pt2.setCompositionMode(QPainter::CompositionMode_Xor);
            pt2.drawImage(QRect(m_lt - n_lt, m_rb - n_lt), m_img);
            pt2.drawImage(QRect(lt - n_lt, rb - n_lt), img);
            m_img = n_img;
            m_lt = n_lt;
            m_rb = n_rb;
            rea::imagePool::cacheImage("drawFree", m_img);
            rea::pipeline::run("updateQSGAttr_" + getParentName(), rea::Json("key", rea::JArray("range"),
                                                                             "obj", m_shape,
                                                                             "val", rea::JArray(m_lt.x(), m_lt.y(), m_rb.x() + 1, m_rb.y() + 1),
                                                                             "force", true));
        }
    }
    QPoint m_lt, m_rb;
    QString m_shape;
protected:
    void mousePressEvent(QMouseEvent *event) override {
        if (event->button() == Qt::LeftButton){
            //std::cout << "st" << std::endl;
            updateMask();
        }
    }
    void mouseReleaseEvent(QMouseEvent *event) override {
        if (event->button() == Qt::LeftButton){
            //std::cout << "ed" << std::endl;
            rea::pipeline::run("updateQSGAttr_" + getParentName(), rea::Json("key", rea::JArray("objects"),
                                                                             "type", "del",
                                                                             "tar", m_shape));
            auto pts = extractCounter({m_lt.x(), m_lt.y()}, m_img, 1);
            QJsonArray arrs;
            for (auto i : pts){
                QJsonArray arr;
                for (auto j : i){
                    arr.push_back(j.x);
                    arr.push_back(j.y);
                }
                arrs.push_back(arr);
            }

            auto m_shape = "shp_" + rea::generateUUID();
            auto nm = getParentName();

            auto redo = [nm, m_shape, arrs](){
                rea::pipeline::run("updateQSGAttr_" + nm,
                                   rea::Json("key", rea::JArray("objects"),
                                             "type", "add",
                                             "tar", m_shape,
                                             "val", rea::Json(
                                                        "type", "poly",
                                                        "points", arrs,
                                                        "face", 125)
                                                                                                                                    ));
            };
            redo();
            rea::pipeline::run<rea::ICommand>("addCommand",
                                              rea::ICommand(redo, [nm, m_shape](){
                                                   rea::pipeline::run("updateQSGAttr_" + nm,
                                                                       rea::Json("key", rea::JArray("objects"),
                                                                                 "type", "del",
                                                                                 "tar", m_shape));
                                              }));
            m_img = QImage();
        }
    }
    void mouseMoveEvent(QMouseEvent *event) override {
        qsgPluginTransform::mouseMoveEvent(event);
        updateHandlePos(0, event->pos());
        if (event->buttons().testFlag(Qt::LeftButton)){
            //std::cout << "st" << std::endl;
            updateMask();
        }

    }
    void hoverMoveEvent(QHoverEvent *event) override {
        qsgPluginTransform::hoverMoveEvent(event);
        updateHandlePos(0, event->pos());
    }
    QString getName(rea::qsgBoard* aParent = nullptr) override{
        qsgPluginTransform::getName(aParent);
        updateParent([this](QSGNode* aBackground){
            createEllipseHandle(aBackground, 10, 125);
            m_transnode = reinterpret_cast<QSGTransformNode*>(aBackground);
        });
        return m_name;
    }
private:
    QSGTransformNode* m_transnode;
    QImage m_img;
};

static rea::regPip<QJsonObject, rea::pipePartial> plugin_draw_free([](rea::stream<QJsonObject>* aInput){
    aInput->out<std::shared_ptr<rea::qsgBoardPlugin>>(std::make_shared<qsgPluginDrawFree>(aInput->data()));
}, rea::Json("name", "create_qsgboardplugin_drawfree"));
