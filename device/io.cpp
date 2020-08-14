#include "reactive2.h"
#include <ds_rx_io.h>
#include <QHash>
#include <QSet>
#include <QJsonDocument>

using ioCommandSubject = rx::subjects::subject<ds::DsIOCommand>;
using ioDataSubject = rx::subjects::subject<std::string>;

class rxIOs{
private:
    class IOInfo{
    public:
        IOInfo(){}
        IOInfo(std::shared_ptr<ds::DsRxIO> aIO, std::shared_ptr<ioCommandSubject> aCommandTrigger, std::shared_ptr<ioDataSubject> aDataTrigger){
            m_io = aIO;
            m_command_trigger = aCommandTrigger;
            m_data_trigger = aDataTrigger;
            m_io->SetCommandObservable(m_command_trigger->get_observable().observe_on(rxcpp::synchronize_new_thread()));
            m_io->SetTransferOutObservable(m_data_trigger->get_observable().observe_on(rxcpp::synchronize_new_thread()));
        }
    private:
        std::shared_ptr<ioCommandSubject> m_command_trigger;
        std::shared_ptr<ioDataSubject> m_data_trigger;
        std::shared_ptr<ds::DsRxIO> m_io;
        friend rxIOs;
    };
public:
    rxIOs();
private:
    QHash<QString, IOInfo> m_ios;
    QSet<QString> m_model_init;
};

rxIOs::rxIOs(){
    rea::pipeline::add<QJsonObject>([this](rea::stream<QJsonObject>* aInput){
        auto cfg = aInput->data();
        auto nm = cfg.value("name").toString();
        if (m_ios.contains(nm))
            return;

        QString strJson(QJsonDocument(cfg).toJson(QJsonDocument::Compact));

        rapidjson::Document config;
        if (config.Parse(strJson.toStdString().c_str()).HasParseError()) {
            return;
        }

        auto io = std::make_shared<ds::DsRxIO>(config["initial"]);
        m_ios.insert(nm, IOInfo(io, std::make_shared<ioCommandSubject>(), std::make_shared<ioDataSubject>()));

        io->GetStateObservable().subscribe([nm](ds::DsIOState aState) {
            rea::pipeline::run<QString>(nm + "_IOStated", QString::fromStdString(ds::DsIOStateTypeString[aState.type]));
        });

        io->GetTransferInObservable().subscribe([nm](std::string aIn) {
            rea::pipeline::run<QString>(nm + "_IOReceived", QString::fromStdString(aIn));
        });

        rea::pipeline::add<QString>([](rea::stream<QString>* aInput){
            aInput->out();
        }, rea::Json("name", nm + "_IOStated"));

        rea::pipeline::add<QString>([](rea::stream<QString>* aInput){
            aInput->out();
        }, rea::Json("name", nm + "_IOReceived"));

        rea::pipeline::add<QJsonObject>([this, nm](rea::stream<QJsonObject>* aInput){
            auto dt = aInput->data();
            if (m_ios.contains(nm)){
                auto inf = m_ios.value(nm);
                if (dt.value("on").toBool()){
                    inf.m_command_trigger->get_subscriber().on_next(ds::DsIOCommand{ds::DsIOCommandOpen});
                }else
                    inf.m_command_trigger->get_subscriber().on_next(ds::DsIOCommand{ds::DsIOCommandClose});
            }
            aInput->out();
        }, rea::Json("name", nm + "_turnIO"));

        rea::pipeline::add<QJsonObject, rea::pipeDelegate>([this, nm](rea::stream<QJsonObject>* aInput){
            auto dt = aInput->data();
            if (m_ios.contains(nm)){
                auto inf = m_ios.value(nm);
                inf.m_data_trigger->get_subscriber().on_next(QJsonDocument(dt).toJson().toStdString());
            }
        }, rea::Json("name", nm + "_sendIO", "param", rea::Json("delegate", nm + "_IOReceived")));
    }, rea::Json("name", "addIO"));
}

static rxIOs ios;
