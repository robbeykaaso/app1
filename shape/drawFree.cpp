#include "qsgShow/qsgBoard.h"

class drawFree : public qsgPluginTransform{
public:
     drawFree(const QJsonObject& aConfig) : qsgPluginTransform(aConfig){

     }
};

static rea::regPip<QJsonObject, rea::pipePartial> command_draw_free([](rea::stream<QJsonObject>* aInput){
    aInput->out<std::shared_ptr<qsgBoardPlugin>>(std::make_shared<drawFree>(aInput->data()));
}, rea::Json("name", "create_qsgboardplugin_drawfree"));
