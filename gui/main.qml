import QtQuick 2.12
import QtQuick.Window 2.12
import QtQuick.Controls 1.4
import QtQuick.Controls 2.5
import QtQuick.Layouts 1.12
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
        id: mainmenu
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
                    shortcut: "Ctrl+F"
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
                                                                                                         points: [[500, 300, 700, 300, 700, 500, 500, 300]],
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
                    text: qsTr("polyPoints")
                    onClicked: {
                        checked != checked
                        Pipeline2.run("updateQSGAttr_testbrd", {obj: "shp_0", key: ["points"], val: checked ? [[50, 50, 200, 50, 200, 200, 50, 200, 50, 50], [80, 70, 120, 100, 120, 70, 80, 70]] : [[50, 50, 200, 200, 200, 50, 50, 50]]})
                    }
                }
                MenuItem{
                    checkable: true
                    text: qsTr("ellipseAngle")
                    onClicked: {
                        checked != checked
                        Pipeline2.run("updateQSGAttr_testbrd", {obj: "shp_1", key: ["angle"], val: checked ? 90 : 20})
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
                title: qsTr("log")
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
                        Pipeline2.run("showLogPanel", {})
                    }
                }
            }
            Menu{
                title: qsTr("list")
                MenuItem{
                    text: qsTr("updateListView")
                    onClicked: {
                        Pipeline2.run("_updateListView", {title: ["cat", "dog", "sheep", "rat"],
                                                          selects: [1, 3, 5],
                                                          data: [
                                                            {entry: [4, 6, 2, 3]},
                                                            {entry: [4, 6, 2, 3]},
                                                            {entry: [4, 6, 2, 3]},
                                                            {entry: [4, 6, 2, 3]},
                                                            {entry: [4, 6, 2, 3]},
                                                            {entry: [4, 6, 2, 3]}
                                                          ]})
                    }
                }
                MenuItem{
                    text: qsTr("modifyListView")
                    onClicked: {
                        Pipeline2.run("_updateListView", {index: [2, 4, 5],
                                                          fontclr: "red",
                                                          data: [
                                                            {entry: [1, 3, 2, 3]},
                                                            {},
                                                            {entry: [2, 3, 2, 3]}
                                                          ]})
                    }
                }

                Component.onCompleted: {
                    Pipeline2.find("_listViewSelected").next(function(aInput){
                        console.log(aInput)
                    }, {tag: "manual"}, {vtype: []})
                }
            }

            Menu{
                title: qsTr("container")
                MenuItem{
                    text: qsTr("TWindow")
                    onClicked: {
                        twin.show()
                    }
                }
                MenuItem{
                    text: qsTr("swipe")
                    onClicked: swipe.show()
                }
                MenuItem{
                    text: qsTr("gridder")
                    onClicked: gridder.show()
                }
                MenuItem{
                    text: qsTr("flip")
                    onClicked: flip.show()
                }
                MenuItem{
                    text: qsTr("nest")
                    onClicked:
                        nest.show()
                }
            }

            Menu{
                title: qsTr("dialog")

                Menu{
                    title: qsTr("file")
                    MenuItem{
                        text: qsTr("files")
                        onClicked: {
                            Pipeline2.run("_selectFile", {folder: false, filter: ["Image files (*.jpg *.png *.jpeg *.bmp)"]})
                        }
                    }
                    MenuItem{
                        text: qsTr("directory")
                        onClicked: {
                            Pipeline2.run("_selectFile", {folder: true, tag: {tag: "manual2"}})
                        }
                    }
                    Component.onCompleted: {
                        Pipeline2.find("_selectFile").next(function(aInput){
                            console.log(aInput)
                        }, {tag: "manual2"}, {vtype: []})
                    }
                }

                MenuItem{
                    text: qsTr("color")
                    onClicked: {
                        Pipeline2.run("_selectColor", {tag: {tag: "manual2"}})
                    }
                    Component.onCompleted: {
                        Pipeline2.find("_selectColor").next(function(aInput){
                            console.log(aInput)
                        }, {tag: "manual2"}, {vtype: ""})
                    }
                }

                MenuItem{
                    text: qsTr("MsgDialog")
                    onClicked:
                        Pipeline2.run("popMessage", {title: "hello4", text: "world"})
                }

            }

            Menu{
                title: qsTr("diagram")

                MenuItem{
                    text: qsTr("linechart")
                    onClicked: {
                        linechart.show()
                    }
                }

                MenuItem{
                    text: qsTr("histogram")
                    onClicked: {
                        histogram.show()
                    }
                }

                MenuItem{
                    text: qsTr("thistogram")
                    onClicked: {
                        thistogram.show()
                    }
                }
            }

            MenuItem{
                text: qsTr("matrix")
                onClicked: {
                    matrix.show()
                }
            }

            MenuItem{
                text: qsTr("status")
                onClicked:
                    Pipeline2.run("_updateStatus", ["hello", "world"])
            }

            MenuItem{
                text: qsTr("search")
                onClicked:
                    Pipeline2.run("_Searched", "", {tag: "manual"})
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
            MenuItem{
                property var tag: {"tag": "testProgress"}
                property int cnt: 0
                property double hope
                text: qsTr("progress")
                onClicked: {
                    if (cnt % 10 == 0){
                        hope = 0.0
                        Pipeline2.run("updateProgress", {title: "demo: ", sum: 10}, tag)
                        hope = 0.1
                        Pipeline2.run("updateProgress", {}, tag)
                        ++cnt
                    }else if (cnt % 10 == 1){
                        hope = 0.2
                        Pipeline2.run("updateProgress", {}, tag)
                        ++cnt
                    }else if (cnt % 10 == 2){
                        hope = 0.9
                        Pipeline2.run("updateProgress", {step: 7}, tag)
                        cnt += 7
                    }else if (cnt % 10 == 9){
                        hope = 1.0
                        Pipeline2.run("updateProgress", {}, tag)
                        ++cnt
                    }
                }
                Component.onCompleted: {
                    Pipeline2.find("updateProgress").next(function(aInput){
                        console.assert(aInput === hope)
                    }, tag, {vtype: 0.1})
                }
            }
        }

        Menu{
            title: qsTr("device")
            Action{
                text: qsTr("camera")
                onTriggered: camera.show()
            }
            Action{
                text: qsTr("io")
                onTriggered: io.show()
            }
        }

        Menu{
            title: qsTr("integrate")
            Action{
                text: qsTr("pack")
                onTriggered:
                    //Pipeline2.run("showPackWindow", {})
                    Pipeline2.run("packExe", {})
            }
        }

        /*Loader{
            source: "file:D:/mywork/build-app-Desktop_Qt_5_12_2_MSVC2015_64bit-Default/plugin/menu/shape.qml"
            onLoaded: {
                mainmenu.addMenu(item)
            }
        }*/

        DynamicQML{
            name: "menu"
            onLoaded: function(aItem){
                mainmenu.addMenu(aItem)
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
            Row{
                width: parent.width
                height: parent.height - 30
                Column{
                    width: parent.width * 0.3
                    height: parent.height
                    Search {
                        text: qsTr("search")
                        width: parent.width
                        height: 30
                        prefix: "#"
                    }
                    Rectangle{
                        width: parent.width
                        height: parent.height - 30
                        color: "white"
                        List{
                            anchors.fill: parent
                        }
                    }
                    Component.onCompleted: {
                        Pipeline2.find("_Searched")
                        .next(function(aInput){
                            console.assert(aInput === qsTr("search"))
                            console.log(aInput + " is searched")
                        }, {tag: "manual"}, {vtype: ""})
                    }
                }
                Column{
                    width: parent.width * 0.7
                    height: parent.height
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
            }
            Status{
                width: parent.width
                height: 30
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
                Track{
                    width: 180
                    caption.text: qsTr("attri4") + ":"
                    ratio: 0.4
                    onIndexChanged: function(aIndex){
                        console.log("track index: " + aIndex)
                    }
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
        id: gridder
        caption: qsTr("gridder")
        content: Gridder{
            id: gridder_cld

            name: "demo"
            size: [2, 2]
            com: Component{
                Rectangle{
                    property string name
                    width: parent.width / parent.columns
                    height: parent.height / parent.rows

                    color: "transparent"
                    border.color: "red"
                    Component.onCompleted: {
                        console.log(name)
                    }
                }
            }

            padding: parent.width * 0.05
            width: parent.width * 0.9
            height: parent.height * 0.9
        }
        footbuttons: [
            {
                cap: "6",
                func: function(){
                    Pipeline2.run(gridder_cld.name + "_updateViewCount", {size: 6})
                }
            },
            {
                cap: "10",
                func: function(){
                    gridder_cld.updateViewCount(10)
                }
            },
            {
                cap: "1",
                func: function(){
                    Pipeline2.run(gridder_cld.name + "_updateViewCount", {size: 1})
                }
            },
            {
                cap: "5x5",
                func: function(){
                    Pipeline2.run(gridder_cld.name + "_updateViewCount", {size: [5, 5]})
                }
            }

        ]
    }

    TWindow{
        id: swipe
        caption: qsTr("swipeview")
        content: TabView{
            anchors.fill: parent
            Tab{
                title: qsTr("Red")
                Rectangle{
                    color: "red"
                }
            }
            Tab{
                title: qsTr("Blue")
                Rectangle{
                    color: "blue"
                }
            }
            Tab{
                title: qsTr("Swipe")
                Swipe{

                }
            }
        }
    }

    TWindow{
        id: flip
        caption: qsTr("flipView")
        content: Flip{
            id: flipview
            anchors.fill: parent
            front: Rectangle{
                anchors.fill: parent
                color: "red"
                MouseArea{
                    anchors.fill: parent
                    onClicked: flipview.flipUp()
                }
            }
            back: Rectangle{
                anchors.fill: parent
                color: "blue"
                MouseArea{
                    anchors.fill: parent
                    onClicked: flipview.flipDown()
                }
            }
        }
    }

    TWindow{
        id: nest
        caption: qsTr("nestView")
        content: Nest{
            rows: 10
            columns: 10
            size: [1, 2, 7, 8, 8, 2, 2, 8, 1, 10]
            Rectangle {
                color: "red"
            }
            Rectangle {
                color: "green"
            }
            Rectangle {
                color: "blue"
            }
            Rectangle {
                color: "yellow"
            }
            Rectangle {
                color: "black"
            }
        }
    }

    TWindow{
        id: matrix
        caption: qsTr("matrix")
        content: Matrix{
            id: mtx
            rowcap.text: "hello"
            colcap.text: "world"
            content: [[1, 2], [3, 4], [5, 6]]
        }
        footbuttons: [
            {
                cap: "3x3",
                func: function(){
                    mtx.content = [[1, 2, 3], [4, 5, 6], [7, "8", 9]]
                    mtx.updateGUI(true)
                }
            },
            {
                cap: "2x2",
                func: function(){
                    mtx.content = [[1, 2], [3, 4]]
                    mtx.updateGUI(true)
                }
            },
            {
                cap: "5x4",
                func: function(){
                    Pipeline2.run("_updateMatrix", {rowcap: "hello2",
                                                    colcap: "world2",
                                                    content: [[1, 2, 3, 4],
                                                              [5, 6, 7, 8],
                                                              [9, 10, 11, 12],
                                                              [13, 14, 15, 16],
                                                              [17, 18, 19, 20]]})
                }
            }

        ]
        Component.onCompleted: {
            Pipeline2.find("_matrixSelected").next(function(aInput){
                console.log("_matrixSelected: " + aInput)
            }, {tag: "manual"}, {vtype: 0})
        }
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

    TWindow{
        id: linechart
        caption: qsTr("lineChart")
        content: LineChart{
            anchors.fill: parent
        }
        footbuttons: [
            {
                cap: "test",
                func: function(){
                    Pipeline2.run("_updateLineChart", [20, 30, 100, 125, 30, 10, 12, 30, 50])
                }
            }
        ]
    }

    TWindow{
        id: histogram
        caption: qsTr("histogram")
        content: Histogram{
            anchors.fill: parent
        }
        footbuttons: [
            {
                cap: "test",
                func: function(){
                    Pipeline2.run("_updateHistogramGUI", {histogram: [40, 20, 15, 25, 14, 16, 13, 30]})
                }
            }

        ]
    }

    TWindow{
        id: thistogram
        caption: qsTr("thistogram")
        content: Column{
            anchors.fill: parent
            spacing: 5
            THistogram{
                id: histo
                width: parent.width
                height: parent.height * 0.9
                //oneThreshold: true
            }
            Track{
                property var intervals: []
                property var histogramdata: []
                property double value: 0.1
                property int idx: 0
                anchors.horizontalCenter: parent.horizontalCenter
                height: parent.height * 0.05
                width: parent.width * 0.8
                interval: 100
                caption.text: qsTr("Interval") + ":"
                ratio: 0.2

                function updateInterval(){
                    if (intervals.length > 1 && histogramdata.length > 1){
                        histo.drawHistoGram(histogramdata[idx])
                    }
                }

                onIndexChanged: function(aIndex){
                    idx = aIndex
                    updateInterval()
                }
                Component.onCompleted: {
                    Pipeline2.add(function(aInput){
                        intervals = []
                        histogramdata = []
                        interval = Object.keys(aInput["histogram"]).length - 1
                        for (var i in aInput["histogram"]){
                            intervals.push(i)
                            histogramdata.push(aInput["histogram"][i])
                        }
                        updateInterval()
                        return {out: {}}
                    }, {name: "_updateTHistogramGUI"})
                }
            }

            Track{
                property double value: 0
                anchors.horizontalCenter: parent.horizontalCenter
                height: parent.height * 0.05
                width: parent.width * 0.8
                interval: 100
                caption.text: qsTr("IOU") + ":"
                ratio: 0.2

                onIndexChanged: function(aIndex){
                    value = aIndex / 100
                    //thresholdChanged(maxthreshold.x, minthreshold.x)
                }
            }
        }
        footbuttons: [
            {
                cap: "test",
                func: function(){
                    Pipeline2.run("_updateTHistogramGUI", {histogram: {
                                      5: [10, 20, 15, 30, 25],
                                      10: [10, 20, 15, 30, 25, 20, 21, 23, 42, 12],
                                      20: [10, 20, 15, 30, 25, 20, 21, 23, 42, 12, 12, 10, 20, 42, 30, 15, 25, 20, 21, 23]
                                  }})
                }
            }

        ]
    }

    IO{
        id: io
        name: "io1"
    }
    Camera{
        id: camera
        name: "camera1"
    }
    Progress{

    }
    MsgDialog{

    }
    File{

    }
    ColorSelect{

    }
}
