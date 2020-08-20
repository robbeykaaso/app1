#include "reactive2.h"
#include <QDir>
#include <fstream>
#include <Windows.h>

const auto cfg = rea::Json(
                           //"source", "D:/deepinspection",
                           "app", "D:/mywork/build-app-Desktop_Qt_5_12_2_MSVC2015_64bit-Default/plugin/REA_PLUGIN.dll",
                           "depends", "G:/dependency_walker/depends.exe",
                           "filter", rea::JArray("windows")
                           //"qmladdition", rea::JArray("D:/mydll3", "D:/mydll2"),
                           //"addition", rea::JArray("D:/build-deepinspection-Desktop_Qt_5_12_2_MSVC2015_64bit-Default/plugin")
                               );

bool checkFilter(const std::vector<std::string> aFilters, const std::string& aName) {
    for (auto i : aFilters)
        if (int(aName.find(i)) >= 0)
            return false;
    return true;
}

void parseDependsAndCopy(const std::string& aDir, const std::vector<std::string>& aFilters){
    auto ifs = std::ifstream(aDir + "/depends");
    if (!ifs.is_open())
        return;
    std::vector<std::string> lst;
    std::string inputLine;
    while (std::getline(ifs, inputLine)) {
        lst.push_back(inputLine);
    }

    std::vector<std::string> depends;
    for (auto i = lst.rbegin(); i != lst.rend(); ++i) {
        if (int(i->find("Module List")) >= 0)
            break;
        if (int(i->find("[")) >= 0) {
            int st = i->find("["), ed = i->find("]") + 1;
            auto entry = i->substr(st, ed - st);
            if (int(entry.find("?")) < 0 && int(entry.find("E")) < 0) {
                auto ret = i->substr(ed + 2, i->length());
                if (checkFilter(aFilters, ret))
                    depends.push_back(ret.substr(0, ret.find("     ")));
            }
        }
    }
    for (auto i : depends) {
        auto nm = aDir + i.substr(i.find_last_of("\\"), i.length());
        CopyFile(i.data(), nm.data(), FALSE);
    }
}

void packExe(const QJsonObject& aConfig){
    auto app = aConfig.value("app").toString();
    auto dir = app.mid(0, app.lastIndexOf("/"));

    std::vector<std::string> filters;
    auto filters0 = aConfig.value("filter").toArray();
    for (auto i : filters0)
        filters.push_back(i.toString().toStdString());
    QString cmd;

    auto src = aConfig.value("source").toString();
    if (src != ""){
        cmd = "windeployqt --qmldir " + src + " " + app;
        system(cmd.toStdString().data());
    }

    cmd = aConfig.value("depends").toString() + " /c /ot:" + dir + "/depends" + " /f:1 /sm:15 \"" + app + "\"";
    system(cmd.toStdString().data());
    parseDependsAndCopy(dir.toStdString(), filters);

    auto qmladd = aConfig.value("qmladdition").toArray();
    for (auto i : qmladd){
        cmd = "windeployqt --qmldir " + i.toString() + " " + app;
        system(cmd.toStdString().data());
    }

    auto add = aConfig.value("addition").toArray();
    for (auto i : add){
        QDir dirs(i.toString());
        auto list = dirs.entryList();
        for (auto j : list)
            if (j.endsWith(".dll")){
                auto dll = i.toString() + "/" + j;
                cmd = aConfig.value("depends").toString() + " /c /ot:" + dir + "/depends" + " /f:1 /sm:15 \"" + dll + "\"";
                system(cmd.toStdString().data());
                parseDependsAndCopy(dir.toStdString(), filters);
            }
    }

    std::cout << "pack completed!" << std::endl;
}

static rea::regPip<QJsonObject> pack_exe([](rea::stream<QJsonObject>* aInput){
    packExe(cfg);
    aInput->out();
}, rea::Json("name", "packExe", "thread", 2));

/*static rea::regPip<QQmlApplicationEngine*> reg_plugin_gui([](rea::stream<QQmlApplicationEngine*>* aInput){
    aInput->data()->load(QUrl(QStringLiteral("qrc:/gui/pack.qml")));
    aInput->out();
}, QJsonObject(), "regQML");*/
