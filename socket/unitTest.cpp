#include "reactive2.h"
#include "protocal.h"
#include "server.h"
#include "client.h"

void testSocket(){
    using namespace rea;

    pipeline::find("receiveFromClient")  //server end
    ->next(FUNC(clientMessage,
        auto dt = aInput->data();
        assert(dt.value("key") == "hello");
        aInput->setData(protocal.value(protocal_test).toObject().value("res").toObject())->out();
    ), rea::Json("tag", protocal_test))
    ->next("callClient");

    pipeline::find("clientBoardcast")  //client end
    ->next(FUNC(QJsonObject,
        if (aInput->data().value("value") == "socket is connected"){
            aInput->out<QJsonObject>(protocal.value(protocal_test).toObject().value("req").toObject(), "callServer");
        }
    ))
    ->next("callServer")
    ->next(FUNC(QJsonObject,
                auto dt = aInput->data();
                assert(dt.value("key") == "world");
                aInput->out<QString>("Pass: testSocket ", "testSuccess");
                ), rea::Json("tag", protocal_test))
    ->next("testSuccess");

    pipeline::run<QJsonObject>("tryLinkServer", rea::Json("ip", "127.0.0.1",
                                                          "port", "8081",
                                                          "id", "hello"));
}

static rea::regPip<int> unit_test([](rea::stream<int>* aInput){
    static normalServer sev;
    static normalClient clt;
    testSocket();
    aInput->out();
}, QJsonObject(), "unitTest");
