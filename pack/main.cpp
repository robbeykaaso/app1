#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <fstream>
#include <Windows.h>

/*const auto cfg = dst::Json("source", "D:/deepinspection",
                           "app", "D:/build-deepinspection-Desktop_Qt_5_12_2_MSVC2015_64bit-Default/DeepInspection.exe",
                           "depends", "G:/dependency_walker/depends.exe",
                           "filter", dst::JArray("windows"),
                           "qmladdition", dst::JArray("D:/mydll3", "D:/mydll2"),
                           "addition", dst::JArray("D:/build-deepinspection-Desktop_Qt_5_12_2_MSVC2015_64bit-Default/plugin"));*/

bool checkWhite(const std::vector<std::string> aWhite, const std::string& aName) {
    for (auto i : aWhite)
        if (int(aName.find(i)) >= 0)
            return true;
    return false;
}

bool checkFilter(const std::vector<std::string> aFilters, const std::string& aName) {
    for (auto i : aFilters)
        if (int(aName.find(i)) >= 0)
            return false;
    return true;
}

void parseDependsAndCopy(const std::string& aDir, const std::vector<std::string>& aFilters, const std::vector<std::string>& aWhite){
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
            auto ret = i->substr(ed + 2, i->length());
            auto entry = i->substr(st, ed - st);
            if (checkWhite(aWhite, ret))
                depends.push_back(ret.substr(0, ret.find("     ")));
            else if((int(entry.find("?")) < 0 && int(entry.find("E")) < 0)) {
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
    std::vector<std::string> white;
    auto spec = aConfig.value("specific").toArray();
    for (auto i : spec)
        white.push_back(i.toString().toStdString());
    QString cmd;

    cmd = "windeployqt --qmldir " + aConfig.value("source").toString() + " " + app;
    system(cmd.toStdString().data());

    cmd = aConfig.value("depends").toString() + " /c /ot:" + dir + "/depends" + " /f:1 /sm:15 \"" + app + "\"";
    system(cmd.toStdString().data());
    parseDependsAndCopy(dir.toStdString(), filters, white);

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
                parseDependsAndCopy(dir.toStdString(), filters, white);
            }
    }

    /*auto spec = aConfig.value("specific").toArray();
    for (auto j : spec) {
        auto i = j.toString();
        auto nm = dir.toStdString() + "/" +  i.mid(i.lastIndexOf("/") + 1, i.length()).toStdString();
        CopyFile(i.toStdString().data(), nm.data(), FALSE);
    }*/
}

int main(int argc, char *argv[])
{
    QDir dir;
    if (dir.exists("config_.json")){
        QFile fl("config_.json");
        if (fl.open(QFile::ReadOnly)){
            QJsonDocument doc = QJsonDocument::fromJson(fl.readAll());
            packExe(doc.object());
            fl.close();
        }
    }
    return 0;
}
