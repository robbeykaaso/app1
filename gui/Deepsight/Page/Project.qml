import QtQuick 2.12
import QtQuick.Controls 1.2
import QtQuick.Controls.Styles 1.4
import "../../Component"
import "../Component"
import "../../Basic"
import QSGBoard 1.0
import Pipeline2 1.0

TabView{
    style: TabViewStyle {
        frameOverlap: 1
        tab: Rectangle {
            color: styleData.selected ? "steelblue" :"lightskyblue"
            border.color:  "steelblue"
            implicitWidth: Math.max(text.width + 4, 80)
            implicitHeight: 20
            radius: 2
            Text {
                id: text
                anchors.centerIn: parent
                text: styleData.title
                color: styleData.selected ? "white" : "black"
            }
        }
        frame: Rectangle { color: "steelblue" }
    }

    Tab{
        title: qsTr("Task")
        active: true  //disable lazy loading
        Row{
            anchors.fill: parent
            Rectangle{
                width: 180
                height: parent.height
                List{
                    name: "project_task"
                    anchors.fill: parent
                }
            }
            Column{
                width: parent.width - 180
                height: parent.height
                Rectangle{
                    width: parent.width
                    height: 100
                    Column{
                        anchors.fill: parent
                        Repeater{
                            model: 3
                            delegate: Label{
                                text: ""
                                font.pixelSize: 16
                                leftPadding: 15
                                topPadding: 15
                            }
                        }

                        Component.onCompleted: {
                            Pipeline2.add(function(aInput){
                                children[0].text = qsTr("Name: ") + (aInput["name"] || "")
                                children[1].text = qsTr("Time: ") + (aInput["time"] || "")
                                children[2].text = qsTr("Type: ") + (aInput["type"] || "")
                            }, {name: "updateTaskGUI"})
                        }
                    }
                }
                Grid{
                    id: task_operation
                    property var buttons: [
                        {cap: qsTr("New"), func: function(){
                             Pipeline2.run("_newObject", {title: qsTr("new task"), content: {name: "", type: {model: ["classification", "segmentation", "detection"], index: 0}}, tag: {tag: "newTask"}})
                        }},
                        {cap: qsTr("Open"), func: function(){
                             Pipeline2.run("project_task_listViewSelected", [], {tag: "openTask"})
                        }},
                        {cap: qsTr("Delete"), func: function(){
                             Pipeline2.run("_makeSure", {title: qsTr("delete task"), content: "Make sure to delete?", tag: {tag: "deleteTask"}})
                        }}

                    ]
                    width: parent.width
                    height: parent.height - 100
                    rows: 1
                    columns: 3
                    Repeater{
                        model: 3
                        delegate: Item{
                            width: parent.width / parent.columns
                            height: parent.height / parent.rows
                            Button{
                                width: parent.width * 0.6
                                height: width
                                text: task_operation.buttons[index].cap
                                anchors.verticalCenter: parent.verticalCenter
                                anchors.horizontalCenter: parent.horizontalCenter
                                onClicked: task_operation.buttons[index].func()
                            }
                        }
                    }
                }
            }
        }
    }
    Tab{
        title: qsTr("Labels")
        active: true
        Row{
            anchors.fill: parent
            Rectangle{
                width: 180
                height: parent.height
                List{
                    name: "project_label"
                    selectSuffix: "project_"
                    width: parent.width
                    height: parent.height - 30
                }
                Row{
                    width: parent.width
                    height: 30
                    anchors.bottom: parent.bottom
                    Button{
                        text: qsTr("new")
                        height: 30
                        width: parent.width / 2
                        onClicked: {
                            Pipeline2.run("_newObject", {title: qsTr("new group"), content: {group: ""}, tag: {tag: "newLabelGroup"}})
                        }
                    }
                    Button{
                        text: qsTr("delete")
                        height: 30
                        width: parent.width / 2
                        onClicked: {
                            Pipeline2.run("_makeSure", {title: qsTr("delete group"), content: "Make sure to delete?", tag: {tag: "deleteLabelGroup"}})
                        }
                    }
                }
            }
            Column{
                width: parent.width - 180
                height: parent.height
                Rectangle{
                    width: parent.width
                    height: 100
                    Column{
                        anchors.fill: parent
                        Label{
                            id: labelgrouptitle
                            text: qsTr("Group") + ":"
                            font.pixelSize: 14
                            padding: 5
                            width: parent.width
                            height: 30
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }
                        Gridder{
                            id: labelgroup
                            name: "projectlabels"
                            size: [2, 2]
                            com: LabelButton{
                                headbutton: {
                                    "cap": "X",
                                    "func": function(aLabel){
                                        Pipeline2.run("deleteLabel", aLabel)
                                    }
                                }
                                onClicked: {
                                    var svs = "modifyLabelColor"
                                    Pipeline2.find("_selectColor")
                                    .next(function(aInput){
                                        return {out: [{out: [text, aInput], next: svs},
                                                      {out: [], next: "project_label_listViewSelected"}]}
                                    }, {tag: svs}, {name: "getButtonLabelText", vtype: ""})
                                    .nextB(svs)
                                    .next("project_label_listViewSelected", {tag: svs})
                                    Pipeline2.run("_selectColor", {tag: {tag: svs}})
                                }
                            }
                            width: parent.width
                            height: 70
                        }

                        Component.onCompleted: {
                            Pipeline2.add(function(aInput){
                                labelgrouptitle.text = qsTr("Group") + ":" + (aInput["key"] || "")
                                var lbls = aInput["val"] || {}
                                var cnt = Object.keys(lbls).length
                                labelgroup.updateViewCount(cnt)
                                var idx = 0
                                for (var i in lbls){
                                    labelgroup.children[idx].clr = lbls[i]["color"] || "steelblue"
                                    labelgroup.children[idx++].text = i
                                }
                                return {out: {}}
                            }, {name: "updateProjectLabelGUI"})
                        }
                    }
                }
                Grid{
                    id: label_operation
                    property var buttons: [
                        {cap: qsTr("New"), func: function(){
                            Pipeline2.run("_newObject", {title: qsTr("new label"), content: {label: ""}, tag: {tag: "newLabel"}})
                        }}
                    ]
                    width: parent.width
                    height: parent.height - 100
                    rows: 1
                    columns: 1
                    Repeater{
                        model: 1
                        delegate: Item{
                            width: parent.width / parent.columns
                            height: parent.height / parent.rows
                            Button{
                                width: parent.width * 0.4
                                height: width
                                text: label_operation.buttons[index].cap
                                anchors.verticalCenter: parent.verticalCenter
                                anchors.horizontalCenter: parent.horizontalCenter
                                onClicked: label_operation.buttons[index].func()
                            }
                        }
                    }
                }
            }
        }
    }
    Tab{
        title: qsTr("Image")
        active: true
        Row{
            anchors.fill: parent
            Rectangle{
                width: 180
                height: parent.height
                Column{
                    anchors.fill: parent
                    Row{
                        width: parent.width
                        height: 30
                        LabelEdit{
                            label: "all"
                            group: "filter"
                            fontsize: 8
                            height: parent.height
                            width: 50
                            onUpdatelabel: function(aLabel){
                                if (aLabel === "all"){
                                    Pipeline2.run("filterProjectImages", {type: label})
                                    search.hint = ""
                                    search.text = ""
                                }else if (aLabel === "time")
                                    search.hint = "input time"
                                else if (aLabel === "name")
                                    search.hint = "input name"
                            }
                            Component.onCompleted: {
                                updateMenu({filter: {"all": "", "time": "", "name": ""}})
                                Pipeline2.find("projectimage_Searched")
                                .next(function(aInput){
                                    if (label !== "all")
                                        return {out: [{out: {type: label, value: aInput}, next: "filterProjectImages"}]}
                                }, {tag: "manual"}, {vtype: ""})
                                .next("filterProjectImages")
                            }
                        }
                        Search {
                            id: search
                            name: "projectimage"
                            //text: qsTr("filter")
                            hint: qsTr("filter")
                            height: parent.height
                            width: parent.width - 50
                        }
                        Component.onCompleted: {
                            Pipeline2.add(function(aInput){
                                children[0].label = aInput["type"]
                                search.text = aInput["value"] || ""
                            }, {name: "updateProjectImageFilterGUI"})
                        }
                    }
                    PageList{
                        name: "project_image"
                        width: parent.width
                        height: parent.height - 60
                    }
                    Row{
                        width: parent.width
                        height: 30
                        //anchors.bottom: parent.bottom
                        Button{
                            text: qsTr("import")
                            height: 30
                            width: parent.width / 3
                            onClicked: {
                                Pipeline2.run("_selectFile", {folder: false, filter: ["Image files (*.jpg *.png *.jpeg *.bmp)"], tag: {tag: "importImage"}})
                            }
                        }
                        Button{
                            visible: false
                            text: qsTr("export")
                            height: 30
                            width: parent.width / 3
                        }
                        Button{
                            text: qsTr("setting")
                            height: 30
                            width: parent.width / 3
                            onClicked:
                                Pipeline2.run("setImageShow", {})
                        }
                    }
                }
            }
            Column{
                width: parent.width - 180
                height: parent.height
                Rectangle{
                    width: parent.width
                    height: 100
                    Column{
                        anchors.fill: parent
                        Repeater{
                            model: 3
                            delegate: Label{
                                text: ""
                                height: 20
                                font.pixelSize: 16
                                leftPadding: 10
                                topPadding: 10
                            }
                        }
                        Row{
                            id: imglbls
                            property var labels: ({})
                            height: 30
                            width: parent.width
                            topPadding: 10
                            spacing: 10
                            Label{
                                text: qsTr("Labels: ")
                                height: 20
                                font.pixelSize: 16
                                leftPadding: 10
                            }
                            Component{
                                id: lbledt
                                LabelEdit{
                                    height: 20
                                    onUpdatelabel: function(aLabel){
                                        Pipeline2.run("modifyProjectImageLabel", {group: group, label: aLabel})
                                    }
                                }
                            }
                            Component.onCompleted: {
                                Pipeline2.find("projectLabelChanged").next(function(aInput){
                                    for (var i = 1; i < children.length; ++i)
                                        children[i].destroy()
                                    for (var j in aInput)
                                        if (j !== "shape"){
                                            lbledt.createObject(imglbls, {group: j, label: labels[j]}).updateMenu(aInput)
                                        }
                                })
                            }
                        }
                        Component.onCompleted: {
                            Pipeline2.add(function(aInput){
                                children[0].text = qsTr("Name: ") + (aInput["source"] || "")
                                children[1].text = qsTr("Size: ") + (aInput["width"] || "") + ";" + (aInput["height"] || "")
                                children[2].text = qsTr("Channel: ") + (aInput["channel"] || "")
                                imglbls.labels = aInput["image_label"] || {}
                                for (var i = 1; i < imglbls.children.length; ++i)
                                    imglbls.children[i].label = imglbls.labels[imglbls.children[i].group] || ""
                            }, {name: "updateProjectImageGUI"})
                        }
                    }
                }
                Row{
                    width: parent.width
                    height: parent.height - 100

                    Component.onCompleted: {
                        Pipeline2.find("title_updateStatus").next(function(aInput){
                            for (var i = 0; i < projectimage.children.length; ++i)
                                projectimage.children[i].children[0].beforeDestroy()

                            return {out: {}}
                        }, {}, {name: "removeWholeProjectQSGNodes", vtype: []})

                        Pipeline2.add(function(aInput){
                            for (var i = 0; i < projectimage.children.length; ++i)
                                Pipeline2.run("updateQSGAttr_" + projectimage.children[i].name, {key: ["transform"], type: "zoom"})
                        }, {name: "fitProjectImageShow"})

                        Pipeline2.add(function(aInput){
                            if (aInput["bound"]){
                                projectimage.children[0].children[1].visible = true
                                var bnd = aInput["bound"]
                                projectimage.children[0].children[1].x = bnd[0] + (bnd[2] - bnd[0] - 80) * 0.5
                                projectimage.children[0].children[1].y = bnd[1] - 35
                                var shps = aInput["shapes"]
                                var lbl = ""
                                var idx = 0
                                projectimage.selects = shps
                                for (var i in shps){
                                    if (idx++)
                                        lbl += ";"
                                    lbl += shps[i]["caption"] || ""
                                }
                                projectimage.children[0].children[1].label = lbl
                            }else
                                projectimage.children[0].children[1].visible = false
                        }, {name: "updateQSGSelects_projectimage_gridder0"})
                    }
                    Gridder{
                        id: projectimage
                        property var selects
                        name: "projectimage"
                        size: 1
                        com: Rectangle{
                            property string name
                            width: parent.width / parent.columns
                            height: parent.height / parent.rows
                            color: "transparent"
                            border.color: "black"
                            QSGBoard{
                                name: parent.name
                                anchors.fill: parent
                                plugins: [{type: "transform"}]
                                Component.onDestruction: {
                                    beforeDestroy()
                                }
                            }
                            LabelEdit{
                                visible: false
                                onUpdatelabel: function(aLabel){
                                    if (projectimage.selects)
                                        for (var j = 0; j < projectimage.children.length; ++j)
                                            for (var i in projectimage.selects)
                                                Pipeline2.run("updateQSGAttr_" + projectimage.children[j].name, {obj: i, key: ["caption"], val: aLabel, cmd: true})
                                }
                                Component.onCompleted: {
                                    if (parent.name === "projectimage_gridder0"){
                                        Pipeline2.find("projectLabelChanged").next(function(aInput){
                                            updateMenu(aInput)
                                        })
                                    }
                                }
                            }
                        }
                        width: parent.width - 60
                        height: parent.height
                    }
                    Rectangle{
                        width: 60
                        height: parent.height
                        color: "lightskyblue"
                        Column{
                            anchors.fill: parent
                            Button{
                                text: qsTr("select")
                                height: 30
                                width: parent.width
                                onClicked: Pipeline2.run("updateQSGCtrl_projectimage_gridder0", [{type: "select"}])
                            }
                            Button{
                                text: qsTr("free")
                                height: 30
                                width: parent.width
                                onClicked:  Pipeline2.run("updateQSGCtrl_projectimage_gridder0", [{type: "drawfree"}])
                            }
                            Button{
                                text: qsTr("rect")
                                height: 30
                                width: parent.width
                                onClicked: Pipeline2.run("updateQSGCtrl_projectimage_gridder0", [{type: "drawrect"}])
                            }
                            Button{
                                text: qsTr("ellipse")
                                height: 30
                                width: parent.width
                                onClicked: Pipeline2.run("updateQSGCtrl_projectimage_gridder0", [{type: "drawellipse"}])
                            }
                            Button{
                                text: qsTr("circle")
                                height: 30
                                width: parent.width
                                onClicked: Pipeline2.run("updateQSGCtrl_projectimage_gridder0", [{type: "drawcircle"}])
                            }
                            Button{
                                text: qsTr("node")
                                height: 30
                                width: parent.width
                                onClicked: Pipeline2.run("updateQSGCtrl_projectimage_gridder0", [{type: "editnode"}])
                            }
                            Button{
                                text: qsTr("delete")
                                height: 30
                                width: parent.width
                                onClicked: Pipeline2.run("projectimage_gridder0_deleteShapes", [])
                            }
                            Button{
                                text: qsTr("copy")
                                height: 30
                                width: parent.width
                                onClicked: Pipeline2.run("projectimage_gridder0_copyShapes", {})
                            }
                            Button{
                                text: qsTr("paste")
                                height: 30
                                width: parent.width
                                onClicked: Pipeline2.run("projectimage_gridder0_pasteShapes", {})
                            }
                            Button{
                                text: qsTr("undo")
                                height: 30
                                width: parent.width
                                onClicked: Pipeline2.run("doCommand", - 1, {tag: "manual"})
                            }
                            Button{
                                text: qsTr("redo")
                                height: 30
                                width: parent.width
                                onClicked: Pipeline2.run("doCommand", 1, {tag: "manual"})
                            }
                            Button{
                                text: qsTr("scatter")
                                height: 30
                                width: parent.width
                                onClicked: Pipeline2.run("scatterProjectImageShow", {})
                            }
                            Button{
                                text: qsTr("fit")
                                height: 30
                                width: parent.width
                                onClicked: Pipeline2.run("fitProjectImageShow", {})
                            }
                            Button{
                                text: qsTr("image")
                                height: 30
                                width: parent.width
                                onClicked: Pipeline2.run("showTransformImageWindow", "getProjectCurrentImage")
                            }
                            /*Button{
                                text: qsTr("camera")
                                height: 30
                                width: parent.width
                                onClicked: camera.show()
                            }*/
                        }
                    }
                }
            }

        }
    }
    /*TransformImage{

    }*/
    /*Camera{
        id: camera
        name: "camera1"
    }*/
}
