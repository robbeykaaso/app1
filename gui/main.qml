import QtQuick 2.12
import QtQuick.Window 2.12
import QtQuick.Controls 2.5
import QtQuick.Controls.Universal 2.3
import "Basic"
import "Component"
import "TreeNodeView"
import QSGBoard 1.0
import Pipeline2 1.0

ApplicationWindow {
    id: mainwindow
    property var view_cfg
    visible: true
    width: 800
    height: 600
    //width: Screen.desktopAvailableWidth
    //height: Screen.desktopAvailableHeight
    //visibility: Window.Maximized

    Universal.theme: Universal.Dark
    //Universal.accent: Universal.Green
    //Universal.background: Universal.Cyan

    menuBar: MenuBar{
        Menu{
            title: qsTr("qsgShow")

            delegate: MenuItem {
                id: menuItem
                implicitWidth: 200
                implicitHeight: 40

                arrow: Canvas {
                    x: parent.width - width
                    implicitWidth: 40
                    implicitHeight: 40
                    visible: menuItem.subMenu
                    onPaint: {
                        var ctx = getContext("2d")
                        ctx.fillStyle = menuItem.highlighted ? "#ffffff" : "#21be2b"
                        ctx.moveTo(15, 15)
                        ctx.lineTo(width - 15, height / 2)
                        ctx.lineTo(15, height - 15)
                        ctx.closePath()
                        ctx.fill()
                    }
                }

                indicator: Item {
                    implicitWidth: 40
                    implicitHeight: 40
                    Rectangle {
                        width: 14
                        height: 14
                        anchors.centerIn: parent
                        visible: menuItem.checkable
                        border.color: "#21be2b"
                        radius: 3
                        Rectangle {
                            width: 8
                            height: 8
                            anchors.centerIn: parent
                            visible: menuItem.checked
                            color: "#21be2b"
                            radius: 2
                        }
                    }
                }

                contentItem: Text {
                    leftPadding: menuItem.indicator.width
                    rightPadding: menuItem.arrow.width
                    text: menuItem.text
                    font: menuItem.font
                    opacity: enabled ? 1.0 : 0.3
                    color: menuItem.highlighted ? "#ffffff" : "#21be2b"
                    horizontalAlignment: Text.AlignLeft
                    verticalAlignment: Text.AlignVCenter
                    elide: Text.ElideRight
                }

                background: Rectangle {
                    implicitWidth: 200
                    implicitHeight: 40
                    opacity: enabled ? 1 : 0.3
                    color: menuItem.highlighted ? "#21be2b" : "transparent"
                }
            }

            Menu{
                title: qsTr("updateModel")
                MenuItem {
                    text: qsTr("show")
                    onClicked: {
                        Pipeline2.run("testQSGShow", view_cfg)
                    }
                }

                Action {
                    text: qsTr("face")
                    checkable: true
                    shortcut: "Ctrl+V"
                    onTriggered: {
                        view_cfg["face"] = 100 - view_cfg["face"]
                        Pipeline2.run("testQSGShow", view_cfg)
                    }
                }
                Action {
                    text: qsTr("arrow")
                    checkable: true
                    shortcut: "Ctrl+A"
                    onTriggered: {
                        view_cfg["arrow"]["visible"] = !view_cfg["arrow"]["visible"]
                        Pipeline2.run("testQSGShow", view_cfg)
                    }
                }
                Action {
                    text: qsTr("text")
                    checkable: true
                    shortcut: "Ctrl+T"
                    onTriggered: {
                        view_cfg["text"]["visible"] = !view_cfg["text"]["visible"]
                        Pipeline2.run("testQSGShow", view_cfg)
                    }
                }
            }

            Menu{
                title: qsTr("updateWholeAttr")

                MenuItem{
                    checkable: true
                    text: qsTr("wholeArrowVisible")
                    onClicked:{
                        checked != checked
                        Pipeline2.run("updateQSGAttr_testbrd", {key: ["arrow", "visible"], val: checked})
                    }
                }
                MenuItem{
                    checkable: true
                    text: qsTr("wholeArrowPole")
                    onClicked:{
                        checked != checked
                        Pipeline2.run("updateQSGAttr_testbrd", {key: ["arrow", "pole"], val: checked})
                    }
                }
                MenuItem{
                    checkable: true
                    text: qsTr("wholeFaceOpacity")
                    onClicked: {
                        checked != checked
                        Pipeline2.run("updateQSGAttr_testbrd", {key: ["face"], val: checked ? 200 : 0})
                    }
                }
                MenuItem{
                    checkable: true
                    text: qsTr("wholeTextVisible")
                    onClicked: {
                        checked != checked
                        Pipeline2.run("updateQSGAttr_testbrd", {key: ["text", "visible"], val: checked})
                    }
                }
                MenuItem{
                    checkable: true
                    text: qsTr("wholeColor")
                    onClicked: {
                        checked != checked
                        Pipeline2.run("updateQSGAttr_testbrd", {key: ["color"], val: checked ? "yellow" : "green"})
                    }
                }

                MenuItem{
                    checkable: true
                    text: qsTr("wholeObjects")
                    onClicked: {
                        checked != checked
                        Pipeline2.run("updateQSGAttr_testbrd", {key: ["objects"], type: checked ? "add" : "del", tar: "shp_3", val: {
                                                                                                         type: "poly",
                                                                                                         points: [500, 300, 700, 300, 700, 500, 500, 300],
                                                                                                         color: "pink",
                                                                                                         caption: "new_obj",
                                                                                                         face: 200
                                                                                                     }})
                    }
                }

            }
            Menu{
                title: qsTr("updateLocalAttr")

                MenuItem{
                    checkable: true
                    text: qsTr("polyArrowVisible")
                    onClicked:{
                        checked != checked
                        Pipeline2.run("updateQSGAttr_testbrd", {obj: "shp_0", key: ["arrow", "visible"], val: checked})
                    }
                }
                MenuItem{
                    checkable: true
                    text: qsTr("polyArrowPole")
                    onClicked:{
                        checked != checked
                        Pipeline2.run("updateQSGAttr_testbrd", {obj: "shp_0", key: ["arrow", "pole"], val: checked})
                    }
                }
                MenuItem{
                    checkable: true
                    text: qsTr("polyFaceOpacity")
                    onClicked: {
                        checked != checked
                        Pipeline2.run("updateQSGAttr_testbrd", {obj: "shp_0", key: ["face"], val: checked ? 200 : 0})
                    }
                }
                MenuItem{
                    checkable: true
                    text: qsTr("polyTextVisible")
                    onClicked: {
                        checked != checked
                        Pipeline2.run("updateQSGAttr_testbrd", {obj: "shp_0", key: ["text", "visible"], val: checked})
                    }
                }
                MenuItem{
                    checkable: true
                    text: qsTr("polyColor")
                    onClicked: {
                        checked != checked
                        Pipeline2.run("updateQSGAttr_testbrd", {obj: "shp_0", key: ["color"], val: checked ? "yellow" : "green"})
                    }
                }
                MenuItem{
                    checkable: true
                    text: qsTr("polyCaption")
                    onClicked: {
                        checked != checked
                        Pipeline2.run("updateQSGAttr_testbrd", {obj: "shp_0", key: ["caption"], val: checked ? "poly_new" : "poly"})
                    }
                }
                MenuItem{
                    checkable: true
                    text: qsTr("polyWidth")
                    onClicked: {
                        checked != checked
                        Pipeline2.run("updateQSGAttr_testbrd", {obj: "shp_0", key: ["width"], val: checked ? 0 : 10})
                    }
                }
                MenuItem{
                    checkable: true
                    text: qsTr("polyAngle")
                    onClicked: {
                        checked != checked
                        Pipeline2.run("updateQSGAttr_testbrd", {obj: "shp_0", key: ["angle"], val: checked ? 90 : 20})
                    }
                }
                MenuItem{
                    checkable: true
                    text: qsTr("polyPoints")
                    onClicked: {
                        checked != checked
                        Pipeline2.run("updateQSGAttr_testbrd", {obj: "shp_0", key: ["points"], val: checked ? [[50, 50, 200, 50, 200, 200, 50, 200, 50, 50]] : [[50, 50, 200, 200, 200, 50, 50, 50]]})
                    }
                }
                MenuItem{
                    checkable: true
                    text: qsTr("ellipseCenter")
                    onClicked: {
                        checked != checked
                        Pipeline2.run("updateQSGAttr_testbrd", {obj: "shp_1", key: ["center"], val: checked ? [600, 400] : [400, 400]})
                    }
                }
                MenuItem{
                    checkable: true
                    text: qsTr("ellipseRadius")
                    onClicked: {
                        checked != checked
                        Pipeline2.run("updateQSGAttr_testbrd", {obj: "shp_1", key: ["radius"], val: checked ? [200, 400] : [300, 200]})
                    }
                }
                MenuItem{
                    checkable: true
                    text: qsTr("ellipseCCW")
                    onClicked: {
                        checked != checked
                        Pipeline2.run("updateQSGAttr_testbrd", {obj: "shp_1", key: ["ccw"], val: checked})
                    }
                }
                MenuItem{
                    checkable: true
                    text: qsTr("imagePath")
                    onClicked: {
                        checked != checked
                        Pipeline2.run("updateQSGAttr_testbrd", {obj: "img_2", key: ["path"], val: checked ? "F:/3M/B4DT/DF Mark/V1-1.bmp" : "F:/3M/B4DT/DF Mark/V1-2.bmp"})
                    }
                }
                MenuItem{
                    checkable: true
                    text: qsTr("imageRange")
                    onClicked: {
                        checked != checked
                        Pipeline2.run("updateQSGAttr_testbrd", {obj: "img_2", key: ["range"], val: checked ? [0, 0, 600, 800] : [0, 0, 400, 300]})
                    }
                }
            }
        }

        Menu{
            title: qsTr("gui")
            Menu{
                title: qsTr("Log")
                MenuItem{
                    text: qsTr("addLogRecord")
                    onClicked:{
                        Pipeline2.run("addLogRecord", {type: "train", level: "info", msg: "train_info"})
                        Pipeline2.run("addLogRecord", {type: "train", level: "warning", msg: "train_warning"})
                        Pipeline2.run("addLogRecord", {type: "train", level: "error", msg: "train_error"})
                        Pipeline2.run("addLogRecord", {type: "system", level: "info", msg: "system_info"})
                        Pipeline2.run("addLogRecord", {type: "system", level: "warning", msg: "system_warning"})
                        Pipeline2.run("addLogRecord", {type: "system", level: "error", msg: "system_error"})
                    }
                }
                MenuItem{
                    text: qsTr("showLogPanel")
                    onClicked:{
                        Pipeline2.run("showLogPanel")
                    }
                }
            }
            MenuItem{
                text: qsTr("TWindow")
                onClicked: {
                    twin.show()
                }
            }
            MenuItem{
                text: qsTr("MsgDialog")
                onClicked:
                    Pipeline2.run("popMessage", {title: "hello4", text: "world"})
            }
            MenuItem{
                text: qsTr("baseCtrl")
                onClicked: {
                    basectrl.show()
                }
            }
            MenuItem{
                text: qsTr("treeNodeView")
                onClicked: {
                    treeview.show()
                }
            }

        }

        Component.onCompleted: {
            view_cfg = {
                face: 0,
                arrow: {
                    visible: false,
                    pole: true
                },
                text: {
                    visible: false,
                    location: "middle"
                }
            }
        }
    }
    contentData:
        Column{
        anchors.fill: parent
            QSGBoard{
                id: testbrd
                name: "testbrd"
                plugins: [{type: "transform"}]
                width: parent.width
                height: parent.height * 0.7
                Component.onDestruction: {
                    beforeDestroy()
                }
            }

            Log{
                width: parent.width
                height: parent.height * 0.3
            }
        }
    TWindow{
        id: twin
        caption: qsTr("TWindow")
        content: Column{
            anchors.fill: parent
            Rectangle{
                width: parent.width
                height: parent.height
                color: "gray"
            }
        }
        titlebuttons: [{cap: "O", func: function(){console.log("hello")}},
                        {cap: "W", func: function(){console.log("world")}}]
        footbuttons: [{cap: "OK", func: function(){close()}},
                       {cap: "Cancel", func: function(){close()}}]
    }

    TWindow{
        id: basectrl
        caption: qsTr("baseCtrl")
        content: Rectangle{
            anchors.fill: parent
            color: "gray"
            Column{
                /*Repeater{
                    delegate: T{

                    }
                }*/
                id: clm
                leftPadding: 5
                Edit{
                    width: 180
                    caption.text: qsTr("attr1") + ":"
                    ratio: 0.4
                }
                Check{
                    width: 180
                    caption.text: qsTr("attribu2") + ":"
                    ratio: 0.4
                }
                Combo{
                    width: 180
                    caption.text: qsTr("attribute3") + ":"
                    ratio: 0.4
                }
                Radio{
                    text: qsTr("attribute4")
                }

                AutoSize{

                }
            }
        }
        footbuttons: [{cap: "OK", func: function(){close()}}]
    }

    TWindow{
        id: treeview
        property var sample: {
            "hello": "world",
            "he": [true, false],
            "hi": {
                "hi1": "hi2",
                "ge": {
                    "too": 3,
                    "heww": {
                        "ll": [3, 3, 4],
                        "dd": "dddd",
                        "ff": false
                    }
                }
            },
            "hi20": true,
            "hello2": "world",
            "he2": [0, {"kao": "gege"}, 1],
            "hi2": [
                {"hello": "world"},
                {"no": [true, false]}
            ],
            "hi22": 3
        }

        caption: qsTr("treeview")
        content: Rectangle{
            color: "gray"
            anchors.fill: parent
            TreeNodeView{
                anchors.fill: parent
            }
        }
        footbuttons: [
            {cap: "modify", func: function(){
                Pipeline2.run("modifyTreeViewGUI", {key: ["hi2", "1", "no", "1"], val: true}, {tag: "modifyTreeView"})}},
            {cap: "add", func: function(){
                Pipeline2.run("modifyTreeViewGUI", {key: ["hi2", "1", "no", "2"], type: "add", val: 14}, {tag: "modifyTreeView"})}},
            {cap: "delete", func: function(){
                Pipeline2.run("modifyTreeViewGUI", {key: ["hi"], type: "del"}, {tag: "modifyTreeView"})}},
            {cap: "load", func: function(){
                Pipeline2.run("loadTreeView", {data: sample}, {tag: "testTreeView"})}},
            {cap: "save", func: function(){
                Pipeline2.run("saveTreeView", {}, {tag: "testTreeView"})}},
            {cap: "style", func: function(){
                Pipeline2.run("saveTreeView", {}, {tag: "styleTreeView"})}}
            ]

        function sameObject(aTarget, aRef){
            for (var i in aTarget)
                if (typeof aRef[i] === "object"){
                    if (!sameObject(aTarget[i], aRef[i])){
                        //console.log(i + ";" + aTarget[i] + ";" + aRef[i])
                        return false
                    }
                }else if (aTarget[i] !== aRef[i]){
                    //console.log(i + ";" + aTarget[i] + ";" + aRef[i])
                    return false
                }
            return true
        }

        Component.onCompleted: {
            Pipeline2.find("loadTreeView")
            .nextL("saveTreeView", {tag: "testTreeView"})
            .next(function(aInput){
                console.assert(sameObject(aInput, sample))
                return {out: [{out: "Pass: save/load TreeView", next: "testSuccess"}]}
            })
            .next("testSuccess")

            Pipeline2.find("treeViewGUIModified")
            .next(function(aInput){
                console.log("treeViewGUIModified;" + aInput["key"] + ";" + aInput["val"] + ";" + aInput["type"])
            })

            Pipeline2.find("saveTreeView")
            .nextL("styleTreeView", {tag: "styleTreeView"})
            .nextL("loadTreeView")
            .nextL("saveTreeView")
            .next(function(aInput){
                return {data: {data: sample, style: aInput}, out: {}}
            })
            .nextL("loadTreeView")
            .next(function(aInput){
                return {out: [{
                            out: {data: aInput["style"], path: "style.json"},
                        }]}
            })
            .nextL("json2stg")
            .nextL("writeJson")
        }
    }

    MsgDialog{

    }
}
