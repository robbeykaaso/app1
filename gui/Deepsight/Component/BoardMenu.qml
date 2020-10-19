import QtQuick 2.12
import QtQuick.Controls 2.5
import Pipeline2 1.0

Menu{
    id: root
    property var cmenu
    property string parentname
    visible: false
    width: 120
    z: 9

    MenuSeparator{
        id: sep
        visible: false
    }

    MenuItem{
        text: qsTr("select")
        onClicked: Pipeline2.run("updateQSGCtrl_" + parentname, [{"type": "select"}])
    }
    MenuItem{
        text: qsTr("poly")
        onClicked: Pipeline2.run("updateQSGCtrl_" + parentname, [{"type": "drawpoly"}])
    }
    MenuItem{
        text: qsTr("rect")
        onClicked: Pipeline2.run("updateQSGCtrl_" + parentname, [{"type": "drawrect"}])
    }
    MenuItem{
        text: qsTr("free")
        onClicked: Pipeline2.run("updateQSGCtrl_" + parentname, [{"type": "drawfree"}])
    }
    MenuItem{
        text: qsTr("ellipse")
        onClicked: Pipeline2.run("updateQSGCtrl_" + parentname, [{"type": "drawellipse"}])
    }
    MenuItem{
        text: qsTr("circle")
        onClicked: Pipeline2.run("updateQSGCtrl_" + parentname, [{"type": "drawcircle"}])
    }

    function executeCommand(aCaption){
        Pipeline2.run(cmenu[aCaption]["cmd"], cmenu[aCaption]["param"])
    }

    /*return rea::Json("menu", rea::JArray(
                                 rea::Json("cap", "select", "cmd", "updateQSGCtrl_" + getParentName(), "param", rea::JArray(rea::Json("type", "select"))),
                                 rea::Json("cap", "rect", "cmd", "updateQSGCtrl_" + getParentName(), "param", rea::JArray(rea::Json("type", "drawrect"))),
                                 rea::Json("cap", "free", "cmd", "updateQSGCtrl_" + getParentName(), "param", rea::JArray(rea::Json("type", "drawfree"))),
                                 rea::Json("cap", "ellipse", "cmd", "updateQSGCtrl_" + getParentName(), "param", rea::JArray(rea::Json("type", "drawellipse"))),
                                 rea::Json("cap", "circle", "cmd", "updateQSGCtrl_" + getParentName(), "param", rea::JArray(rea::Json("type", "drawcircle")))
                                 ));*/

    Component.onCompleted: {
        cmenu = {}
        Pipeline2.find("updateQSGMenu_" + parentname).next(function(aInput){
            visible = aInput["x"] !== undefined && aInput["y"] !== undefined
            if (aInput["menu"]){
                var mn = aInput["menu"]
                for (var i = count - 8; i >= 0; --i)
                    removeItem(itemAt(i))
                sep.visible = false
                for (var j in mn){
                    sep.visible = true
                    cmenu[mn[j]["cap"]] = mn[j]
                    var src = "import QtQuick 2.12; import QtQuick.Controls 2.5;"
                    src += "MenuItem{"
                    src += "text: '" + qsTr(mn[j]["cap"]) + "';"
                    //src += "font.pixelSize: " + fontsize + ";"
                    src += "onClicked: {"
                    src += "executeCommand('" + mn[j]["cap"] + "');"
                    src += "}}"
                    insertItem(0, Qt.createQmlObject(src, root))
                }
            }
            if (visible){
                x = aInput["x"]
                y = aInput["y"]
            }
        })
    }
}
