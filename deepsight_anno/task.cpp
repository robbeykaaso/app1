#include "model.h"

class task : public model{
private:
    void labelManagement(){

    }
public:
    task(){
        labelManagement();
    }
};

static rea::regPip<QQmlApplicationEngine*> init_task([](rea::stream<QQmlApplicationEngine*>* aInput){
    static task cur_task;
    aInput->out();
}, QJsonObject(), "regQML");
