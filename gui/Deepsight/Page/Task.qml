import QtQuick 2.12
import QtQuick.Controls 1.2
import QtQuick.Controls.Styles 1.4
import "../../Component"
import "../Component"
import "../../Basic"
import Pipeline2 1.0
import QSGBoard 1.0

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
        title: qsTr("Labels")
        active: true
        Row{
            anchors.fill: parent
            Rectangle{
                width: 180
                height: parent.height
                Column{
                    anchors.fill: parent
                    List{
                        name: "task_label"
                        selectSuffix: "task_"
                        width: parent.width
                        height: parent.height
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
                            text: qsTr("Candidate")
                            font.pixelSize: 14
                            padding: 5
                            width: parent.width
                            height: 30
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }
                        Gridder{
                            id: candidatelabelgroup
                            name: "candidatelabels2"
                            size: [2, 2]
                            com: LabelButton{
                                headbutton: {
                                    "cap": "+",
                                    "func": function(aLabel){
                                        Pipeline2.run("addTaskLabel", {label: aLabel, add: true})
                                    }
                                }
                            }
                            width: parent.width
                            height: 70
                        }

                        Component.onCompleted: {
                            Pipeline2.add(function(aInput){
                                var lbls = aInput["val"] || {}
                                var cnt = Object.keys(lbls).length
                                candidatelabelgroup.updateViewCount(cnt)
                                var idx = 0
                                for (var i in lbls){
                                    candidatelabelgroup.children[idx].clr = lbls[i]["color"] || "steelblue"
                                    candidatelabelgroup.children[idx++].text = i
                                }
                            }, {name: "updateCandidateLabelGUI"})
                        }
                    }
                }
                Rectangle{
                    width: parent.width
                    height: parent.height - 200
                    color: "steelblue"
                    Label{
                        text: "v\nv\nv\nv\n"
                        font.pixelSize: 20
                        anchors.horizontalCenter: parent.horizontalCenter
                        anchors.verticalCenter: parent.verticalCenter
                    }
                }
                Rectangle{
                    width: parent.width
                    height: 100
                    Column{
                        anchors.fill: parent
                        Gridder{
                            id: tasklabelgroup
                            name: "tasklabels"
                            size: [2, 2]
                            com: LabelButton{
                                headbutton: {
                                    "cap": "X",
                                    "func": function(aLabel){
                                        Pipeline2.run("addTaskLabel", {label: aLabel, add: false})
                                    }
                                }
                            }
                            width: parent.width
                            height: 70
                        }

                        Component.onCompleted: {
                            Pipeline2.add(function(aInput){
                                var lbls = aInput["val"] || {}
                                var cnt = Object.keys(lbls).length
                                tasklabelgroup.updateViewCount(cnt)
                                var idx = 0
                                for (var i in lbls){
                                    tasklabelgroup.children[idx].clr = lbls[i]["color"] || "steelblue"
                                    tasklabelgroup.children[idx++].text = i
                                }
                            }, {name: "updateTaskLabelGUI"})
                        }
                        Label{
                            text: qsTr("Used")
                            font.pixelSize: 14
                            padding: 5
                            width: parent.width
                            height: 30
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
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
                                /*if (aLabel === "all"){
                                    Pipeline2.run("filterProjectImages", {type: label})
                                    search.hint = ""
                                    search.text = ""
                                }else if (aLabel === "time")
                                    search.hint = "input time"
                                else if (aLabel === "name")
                                    search.hint = "input name"*/
                            }
                            Component.onCompleted: {
                                updateMenu({filter: {"all": "", "stage": ""}})
                                /*Pipeline2.find("projectimage_Searched")
                                .next(function(aInput){
                                    if (label !== "all")
                                        return {out: [{out: {type: label, value: aInput}, next: "filterProjectImages"}]}
                                }, {tag: "manual"}, {vtype: ""})
                                .next("filterProjectImages")*/
                            }
                        }
                        Search {
                            id: search
                            name: "taskimage"
                            //text: qsTr("filter")
                            hint: qsTr("filter")
                            height: parent.height
                            width: parent.width - 50
                        }
                        Component.onCompleted: {
                           /* Pipeline2.add(function(aInput){
                                children[0].label = aInput["type"]
                                search.text = aInput["value"]
                            }, {name: "updateProjectImageFilterGUI"})*/
                        }
                    }
                    PageList{
                        name: "task_image"
                        width: parent.width
                        height: parent.height - 60
                    }
                    Row{
                        width: parent.width
                        height: 30
                        Button{
                            text: qsTr("use")
                            height: 30
                            width: parent.width * 0.5
                        }
                        Button{
                            text: qsTr("remove")
                            height: 30
                            width: parent.width * 0.5
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
                            property var proj_lbls
                            property var tsk_lbls
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
                                        //Pipeline2.run("modifyImageLabel", {group: group, label: aLabel})
                                    }
                                }
                            }

                            function getActLabels(){
                                var ret = {}
                                for (var i in proj_lbls)
                                    if (tsk_lbls[i] !== undefined){
                                        var lbl = {}
                                        for (var j in proj_lbls[i])
                                            if (tsk_lbls[i][j] !== undefined)
                                                lbl[j] = proj_lbls[i][j]
                                        ret[i] = lbl
                                    }
                                return ret
                            }

                            Component.onCompleted: {
                                proj_lbls = {}
                                tsk_lbls = {}
                                Pipeline2.find("taskLabelChanged").next(function(aInput){
                                    for (var i = 1; i < children.length; ++i)
                                        children[i].destroy()
                                    tsk_lbls = aInput
                                    var lbls = getActLabels()
                                    for (var j in lbls)
                                        if (j !== "shape"){
                                            lbledt.createObject(imglbls, {group: j, label: labels[j]}).updateMenu(lbls)
                                        }
                                })
                                Pipeline2.find("projectLabelChanged").next(function(aInput){
                                    for (var i = 1; i < children.length; ++i)
                                        children[i].destroy()
                                    proj_lbls = aInput
                                    var lbls = getActLabels()
                                    for (var j in lbls)
                                        if (j !== "shape"){
                                            lbledt.createObject(imglbls, {group: j, label: labels[j]}).updateMenu(lbls)
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
                            }, {name: "updateTaskImageGUI"})
                        }
                    }
                }
                Row{
                    width: parent.width
                    height: parent.height - 100

                    Component.onCompleted: {
                        Pipeline2.find("title_updateStatus").next(function(aInput){
                            for (var i = 0; i < taskimage.children.length; ++i)
                                taskimage.children[i].children[0].beforeDestroy()

                            return {out: {}}
                        }, {}, {name: "removeWholeTaskQSGNodes", vtype: []})

                       /* Pipeline2.add(function(aInput){
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
                        }, {name: "updateQSGSelects_projectimage_gridder0"})*/
                    }
                    Gridder{
                        id: taskimage
                        property var selects
                        name: "taskimage"
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
                                sync: parent.name === "taskimage_gridder0"
                                visible: false
                                onUpdatelabel: function(aLabel){
                                    if (taskimage.selects)
                                        for (var j = 0; j < taskimage.children.length; ++j)
                                            for (var i in taskimage.selects)
                                                Pipeline2.run("updateQSGAttr_" + taskimage.children[j].name, {obj: i, key: ["caption"], val: aLabel, cmd: true})
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
                              //  onClicked: Pipeline2.run("updateQSGCtrl_projectimage_gridder0", [{type: "select"}])
                            }
                            Button{
                                text: qsTr("free")
                                height: 30
                                width: parent.width
                              //  onClicked:  Pipeline2.run("updateQSGCtrl_projectimage_gridder0", [{type: "drawfree"}])
                            }
                            Button{
                                text: qsTr("rect")
                                height: 30
                                width: parent.width
                              //  onClicked: Pipeline2.run("updateQSGCtrl_projectimage_gridder0", [{type: "drawrect"}])
                            }
                            Button{
                                text: qsTr("ellipse")
                                height: 30
                                width: parent.width
                              // onClicked: Pipeline2.run("updateQSGCtrl_projectimage_gridder0", [{type: "drawellipse"}])
                            }
                            Button{
                                text: qsTr("circle")
                                height: 30
                                width: parent.width
                              //  onClicked: Pipeline2.run("updateQSGCtrl_projectimage_gridder0", [{type: "drawcircle"}])
                            }
                            Button{
                                text: qsTr("node")
                                height: 30
                                width: parent.width
                              //  onClicked: Pipeline2.run("updateQSGCtrl_projectimage_gridder0", [{type: "editnode"}])
                            }
                            Button{
                                text: qsTr("delete")
                                height: 30
                                width: parent.width
                              //  onClicked: Pipeline2.run("projectimage_gridder0_deleteShapes", {})
                            }
                            Button{
                                text: qsTr("copy")
                                height: 30
                                width: parent.width
                              //  onClicked: Pipeline2.run("projectimage_gridder0_copyShapes", {})
                            }
                            Button{
                                text: qsTr("paste")
                                height: 30
                                width: parent.width
                              //  onClicked: Pipeline2.run("projectimage_gridder0_pasteShapes", {})
                            }
                            Button{
                                text: qsTr("undo")
                                height: 30
                                width: parent.width
                              //  onClicked: Pipeline2.run("doCommand", - 1, {tag: "manual"})
                            }
                            Button{
                                text: qsTr("redo")
                                height: 30
                                width: parent.width
                              //  onClicked: Pipeline2.run("doCommand", 1, {tag: "manual"})
                            }
                            Button{
                                text: qsTr("scatter")
                                height: 30
                                width: parent.width
                                onClicked: Pipeline2.run("scatterTaskImageShow", {})
                            }
                            Button{
                                text: qsTr("fit")
                                height: 30
                                width: parent.width
                               // onClicked: Pipeline2.run("fitProjectImageShow", {})
                            }
                            Button{
                                text: qsTr("image")
                                height: 30
                                width: parent.width
                               // onClicked: transformimg.show()
                            }
                            Button{
                                text: qsTr("roi")
                                height: 30
                                width: parent.width
                            }
                            Button{
                                text: qsTr("stage")
                                height: 30
                                width: parent.width
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
    Tab{
        title: qsTr("Job")
        active: true
        Row{
            anchors.fill: parent
            Rectangle{
                width: 180
                height: parent.height
                List{
                    anchors.fill: parent
                }
            }
        }
    }
}
