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

void myMessageOutput(QtMsgType type, const QMessageLogContext& context, const QString& msg)
{
    //https://stackoverflow.com/questions/54307407/why-am-i-getting-qwindowswindowsetgeometry-unable-to-set-geometry-warning-wit
    if (type != QtWarningMsg || !msg.startsWith("QWindowsWindow::setGeometry")) {
        QByteArray localMsg = msg.toLocal8Bit();
        fprintf(stdout, localMsg.constData());
    }
}

int main(int argc, char *argv[])
{
    qInstallMessageHandler(myMessageOutput);
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

    //rea::pipeline::run<int>("unitTest", 0);

    /*QJsonObject lan;
    dst::configObject::loadJsonFileConfig("lanInfo", lan);
    QTranslator translator;
    QString ln = lan.value("name").toString();
        translator.load(ln);
    QCoreApplication::installTranslator(&translator);*/

    engine.load(QUrl(QStringLiteral("qrc:/gui/deepsight_anno.qml")));
    //engine.load(QUrl(QStringLiteral("qrc:/gui/main.qml")));
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
    rea::pipeline::run<double>("loadUser", 0);
    return app.exec();
}
