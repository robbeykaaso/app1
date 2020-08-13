#include "qsgShow/qsgBoard.h"
#include "util/cv.h"
#include "imagePool.h"
#include "command.h"
#include <QPainter>

class qsgPluginDrawFree : public qsgPluginTransform{
public:
    qsgPluginDrawFree(const QJsonObject& aConfig) : qsgPluginTransform(aConfig){

    }
private:
    void updateMask(){
        auto ct = m_seal->value("center").toArray();
        auto inv = m_transnode->matrix().inverted();
        auto lt = inv.map(QPoint(ct[0].toDouble() - m_radius, ct[1].toDouble() - m_radius)),
             rb = inv.map(QPoint(ct[0].toDouble() + m_radius, ct[1].toDouble() + m_radius));

        auto img = QImage(rb.x() - lt.x() + 1, rb.y() - lt.y() + 1, QImage::Format_ARGB32);
        img.fill(QColor("transparent"));
        QPainter pt(&img);
        pt.setPen(QPen(QColor("transparent")));
        pt.setBrush(QBrush(QColor(255, 0, 0, 125)));
        pt.drawEllipse(0, 0, img.width(), img.height());

        if (m_img.isNull()){
            m_shapes.push_back("shp_" + rea::generateUUID());
            m_img = img;
            m_lt = lt;
            m_rb = rb;
            rea::imagePool::cacheImage("drawFree", m_img);
            rea::pipeline::run("updateQSGAttr_" + getParentName(), rea::Json("key", rea::JArray("objects"),
                                                                             "type", "add",
                                                                             "tar", m_shapes.back(),
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
                                                                             "obj", m_shapes.back(),
                                                                             "val", rea::JArray(m_lt.x(), m_lt.y(), m_rb.x() + 1, m_rb.y() + 1),
                                                                             "force", true));
        }
    }
    QPoint m_lt, m_rb;
    const int m_radius = 10;
    std::vector<QString> m_shapes;
protected:
    void beforeDestroy() override{
        m_seal->removeQSGNodes();
        m_seal = nullptr;
    }
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
                                                                             "tar", m_shapes.back()));
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
            m_shapes.pop_back();

            auto shp = "shp_" + rea::generateUUID();
            auto nm = getParentName();
            m_shapes.push_back(shp);
            rea::pipeline::run<rea::ICommand>("addCommand",
                                              rea::ICommand([nm, shp, arrs](){
                                                  rea::pipeline::run("updateQSGAttr_" + nm,
                                                                      rea::Json("key", rea::JArray("objects"),
                                                                                "type", "add",
                                                                                "tar", shp,
                                                                                "val", rea::Json(
                                                                                          "type", "poly",
                                                                                          "points", arrs,
                                                                                          "face", 125)
                                                                                   ));
                                              }, [nm, shp](){
                                                   rea::pipeline::run("updateQSGAttr_" + nm,
                                                                       rea::Json("key", rea::JArray("objects"),
                                                                                 "type", "del",
                                                                                 "tar", shp));
                                              }));
            m_img = QImage();
        }
    }
    void mouseMoveEvent(QMouseEvent *event) override {
        qsgPluginTransform::mouseMoveEvent(event);
        updateSeal(event->pos());
        if (event->buttons().testFlag(Qt::LeftButton)){
            //std::cout << "st" << std::endl;
            updateMask();
        }

    }
    void hoverMoveEvent(QHoverEvent *event) override {
        qsgPluginTransform::hoverMoveEvent(event);
        updateSeal(event->pos());
    }
    QString getName(qsgBoard* aParent = nullptr) override{
        qsgPluginTransform::getName(aParent);
        updateParent([this](QSGNode* aBackground){
            m_seal = std::make_shared<ellipseObject>(rea::Json(
                "type", "ellipse",
                "center", rea::JArray(0, 0),
                "radius", rea::JArray(m_radius, m_radius),
                "width", 0,
                "face", 125));
            m_seal->getQSGNodes(m_parent->window(), &m_mdl, aBackground->parent());
            m_transnode = reinterpret_cast<QSGTransformNode*>(aBackground);
        });
        return m_name;
    }
private:
    void updateSeal(const QPoint& aPos){
        m_seal->insert("center", rea::JArray(aPos.x(), aPos.y()));
        updateParent(m_seal->updateQSGAttr("center_"));
    }
    std::shared_ptr<ellipseObject> m_seal;
    qsgModel m_mdl;
    QSGTransformNode* m_transnode;
    QImage m_img;
};

static rea::regPip<QJsonObject, rea::pipePartial> plugin_draw_free([](rea::stream<QJsonObject>* aInput){
    aInput->out<std::shared_ptr<qsgBoardPlugin>>(std::make_shared<qsgPluginDrawFree>(aInput->data()));
}, rea::Json("name", "create_qsgboardplugin_drawfree"));
