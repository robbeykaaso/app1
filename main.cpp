#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QApplication>
#include <QResource>
#include <QTranslator>
#include <QIcon>
#include <iostream>
#include "reactive2.h"
#include <Windows.h>
#include <QWindow>
#include <DbgHelp.h>

LONG ApplicationCrashHandler(EXCEPTION_POINTERS *pException)
{
    //创建 Dump 文件
    HANDLE hDumpFile = CreateFile("crash.dmp", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hDumpFile != INVALID_HANDLE_VALUE)
    {
        //Dump信息
        MINIDUMP_EXCEPTION_INFORMATION dumpInfo;
        dumpInfo.ExceptionPointers = pException;
        dumpInfo.ThreadId = GetCurrentThreadId();
        dumpInfo.ClientPointers = TRUE;

        //写入Dump文件内容
        MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hDumpFile, MiniDumpNormal, &dumpInfo, NULL, NULL);
    }
    rea::pipeline::run<double>("crashDump", 0);
    return EXCEPTION_EXECUTE_HANDLER;
}

int main(int argc, char *argv[])
{
    SetUnhandledExceptionFilter(ApplicationCrashHandler);
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication app(argc, argv);
    app.setWindowIcon(QIcon(rea::getCWD("/favicon.png")));
    app.setOrganizationName("somename");
    app.setOrganizationDomain("somename");

    std::vector<QObject*> objs;
    QQmlApplicationEngine engine;
    engine.connect(&engine, &QQmlApplicationEngine::objectCreated, [&objs](QObject *object, const QUrl &url){
        if (object)
            objs.push_back(object);
    });
    engine.rootContext()->setContextProperty("applicationDirPath", QGuiApplication::applicationDirPath()); //https://recalll.co/ask/v/topic/qt-QML%3A-how-to-specify-image-file-path-relative-to-application-folder/55928bae7d3563c7088b7db1

    rea::pipeline::run<QQmlApplicationEngine*>("regQML", &engine);

    QString mode;
    QHash<QString, QString> prm;
    for (auto i = 0; i < argc; i++){
        auto key = QString::fromStdString(argv[i]);
        if (key.indexOf("-") == 0){
            i++;
            if (i > argc - 1)
                prm.insert(key, "TRUE");
            else
                prm.insert(key, QString::fromStdString(argv[i]));
        }
    }
    if (prm.value("-d") == "TRUE")
        rea::pipeline::run<int>("unitTest", 0);
    if (prm.value("-m") == "GUI")
        rea::pipeline::run<QQmlApplicationEngine*>("loadGUIMain", &engine);
    else{
        rea::pipeline::run<QQmlApplicationEngine*>("loadMain", &engine);
        if (prm.value("-md") == "TRUE"){
            rea::pipeline::add<QJsonObject>([](rea::stream<QJsonObject>* aInput){ aInput->out();}, rea::Json("name", "setDebugMode"));
            rea::pipeline::run<QJsonObject>("setDebugMode", QJsonObject());
        }
    }
    if (engine.rootObjects().isEmpty())
        return -1;

    for (int i = 0; i < objs.size() - 1; ++i){
        objs[i]->setParent(objs[objs.size() - 1]);
        if (objs[i]->isWindowType()){
            auto w = reinterpret_cast<QWindow*>(objs[i]);
            w->setTransientParent(reinterpret_cast<QWindow*>(objs[objs.size() - 1]));
        }
    }
    //dst::streamManager::instance()->emitSignal("setFocus", std::make_shared<dst::streamJson>(dst::Json("board", "panel0")));

    //re::pipeManager::run("unitTest2");
//    re2::testReactive2();
    return app.exec();
}
