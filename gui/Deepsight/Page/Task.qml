import QtQuick 2.12
import QtQuick.Controls 1.2
import QtQuick.Controls.Styles 1.4
import "../../Component"
import "../Component"
import "../../Basic"
import Pipeline2 1.0
import QSGBoard 1.0

TabView{
    function getActLabels(aProjectLabels, aTaskLabels){
        var ret = {}
        for (var i in aProjectLabels)
            if (aTaskLabels[i] !== undefined){
                var lbl = {}
                for (var j in aProjectLabels[i])
                    if (aTaskLabels[i][j] !== undefined)
                        lbl[j] = aProjectLabels[i][j]
                ret[i] = lbl
            }
        return ret
    }

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
                                if (aLabel === "all" || aLabel === "used"){
                                    Pipeline2.run("filterTaskImages", {type: label})
                                    search.hint = ""
                                    search.text = ""
                                }else if (aLabel === "stage")
                                    search.hint = "input stage"
                            }
                            Component.onCompleted: {
                                updateMenu({filter: {"all": "", "used": "", "stage": ""}})
                                Pipeline2.find("taskimage_Searched")
                                .next(function(aInput){
                                    if (label !== "all" && label !== "used")
                                        return {out: [{out: {type: label, value: aInput}, next: "filterTaskImages"}]}
                                }, {tag: "manual"}, {vtype: ""})
                                .next("filterTaskImages")
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
                            Pipeline2.add(function(aInput){
                                children[0].label = aInput["type"]
                                search.text = aInput["value"] || ""
                            }, {name: "updateTaskImageFilterGUI"})
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
                            width: parent.width / 3
                            onClicked: Pipeline2.run("useTaskImage", true)
                        }
                        Button{
                            text: qsTr("remove")
                            height: 30
                            width: parent.width / 3
                            onClicked: Pipeline2.run("useTaskImage", false)
                        }
                        StageButton{
                            height: 30
                            width: parent.width / 3
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
                                        Pipeline2.run("modifyTaskImageLabel", {group: group, label: aLabel})
                                    }
                                }
                            }

                            Component.onCompleted: {
                                proj_lbls = {}
                                tsk_lbls = {}
                                Pipeline2.find("taskLabelChanged").next(function(aInput){
                                    for (var i = 1; i < children.length; ++i)
                                        children[i].destroy()
                                    tsk_lbls = aInput
                                    var lbls = getActLabels(proj_lbls, tsk_lbls)
                                    for (var j in lbls)
                                        if (j !== "shape"){
                                            lbledt.createObject(imglbls, {group: j, label: labels[j]}).updateMenu(lbls)
                                        }
                                })
                                Pipeline2.find("projectLabelChanged").next(function(aInput){
                                    for (var i = 1; i < children.length; ++i)
                                        children[i].destroy()
                                    proj_lbls = aInput
                                    var lbls = getActLabels(proj_lbls, tsk_lbls)
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
                Item{
                    width: parent.width
                    height: parent.height - 100
                    Row{
                        anchors.fill: parent
                        Component.onCompleted: {
                            Pipeline2.find("title_updateStatus").next(function(aInput){
                                for (var i = 0; i < taskimage.children.length; ++i)
                                    taskimage.children[i].children[0].beforeDestroy()

                                return {out: {}}
                            }, {}, {name: "removeWholeTaskQSGNodes", vtype: []})

                            Pipeline2.add(function(aInput){
                                for (var i = 0; i < taskimage.children.length; ++i)
                                    Pipeline2.run("updateQSGAttr_" + taskimage.children[i].name, {key: ["transform"], type: "zoom"})
                            }, {name: "fitTaskImageShow"})

                            Pipeline2.add(function(aInput){
                                if (aInput["bound"]){
                                    taskimage.children[0].children[1].visible = true
                                    var bnd = aInput["bound"]
                                    taskimage.children[0].children[1].x = bnd[0] + (bnd[2] - bnd[0] - 80) * 0.5
                                    taskimage.children[0].children[1].y = bnd[1] - 35
                                    var shps = aInput["shapes"]
                                    var lbl = ""
                                    var idx = 0
                                    taskimage.selects = shps
                                    for (var i in shps){
                                        if (idx++)
                                            lbl += ";"
                                        lbl += shps[i]["caption"] || ""
                                    }
                                    taskimage.children[0].children[1].label = lbl
                                }else
                                    taskimage.children[0].children[1].visible = false
                            }, {name: "updateQSGSelects_taskimage_gridder0"})
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
                                    property var proj_lbls
                                    property var tsk_lbls
                                    visible: false
                                    onUpdatelabel: function(aLabel){
                                        if (taskimage.selects)
                                            for (var j = 0; j < taskimage.children.length; ++j)
                                                for (var i in taskimage.selects)
                                                    Pipeline2.run("updateQSGAttr_" + taskimage.children[j].name, {obj: i, key: ["caption"], val: aLabel, cmd: true})
                                    }
                                    Component.onCompleted: {
                                        proj_lbls = {}
                                        tsk_lbls = {}
                                        if (parent.name === "taskimage_gridder0"){
                                            Pipeline2.find("taskLabelChanged").next(function(aInput){
                                                tsk_lbls = aInput
                                                updateMenu(getActLabels(proj_lbls, tsk_lbls))
                                            })
                                            Pipeline2.find("projectLabelChanged").next(function(aInput){
                                                proj_lbls = aInput
                                                updateMenu(getActLabels(proj_lbls, tsk_lbls))
                                            })
                                        }
                                    }
                                }
                                Label{
                                    color: "green"
                                    visible: parent.name == "taskimage_gridder0"
                                    font.pixelSize: 30
                                    padding: 5
                                    font.bold: true
                                    text: ""
                                    Component.onCompleted: {
                                        if (parent.name == "taskimage_gridder0")
                                            Pipeline2.add(function(aInput){
                                                text = aInput
                                            }, {name: "updateImagePredictGUI", vtype: ""})
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
                                    height: 30
                                    width: parent.width
                                    onClicked: {
                                        Pipeline2.run("updateQSGCtrl_taskimage_gridder0", [{type: "roi"}])
                                    }
                                    style: ButtonStyle{
                                        label: Text{
                                            horizontalAlignment: Text.AlignHCenter
                                            verticalAlignment: Text.AlignVCenter
                                            text: qsTr("roi")
                                            color: "red"
                                        }
                                    }
                                }
                                Button{
                                    height: 30
                                    width: parent.width
                                    onClicked: {
                                        Pipeline2.run("_newObject", {title: qsTr("image stage"), content: {train: 0, test: 0, validation: 0}, tag: {tag: "setImageStage"}})
                                    }
                                    style: ButtonStyle{
                                        label: Text{
                                            horizontalAlignment: Text.AlignHCenter
                                            verticalAlignment: Text.AlignVCenter
                                            text: qsTr("stage")
                                            color: "red"
                                        }
                                    }
                                }
                                Button{
                                    text: qsTr("select")
                                    height: 30
                                    width: parent.width
                                    onClicked: Pipeline2.run("updateQSGCtrl_taskimage_gridder0", [{type: "select"}])
                                }
                                Button{
                                    text: qsTr("free")
                                    height: 30
                                    width: parent.width
                                    onClicked:  Pipeline2.run("updateQSGCtrl_taskimage_gridder0", [{type: "drawfree"}])
                                }
                                Button{
                                    text: qsTr("rect")
                                    height: 30
                                    width: parent.width
                                    onClicked: Pipeline2.run("updateQSGCtrl_taskimage_gridder0", [{type: "drawrect"}])
                                }
                                Button{
                                    text: qsTr("ellipse")
                                    height: 30
                                    width: parent.width
                                    onClicked: Pipeline2.run("updateQSGCtrl_taskimage_gridder0", [{type: "drawellipse"}])
                                }
                                Button{
                                    text: qsTr("circle")
                                    height: 30
                                    width: parent.width
                                    onClicked: Pipeline2.run("updateQSGCtrl_taskimage_gridder0", [{type: "drawcircle"}])
                                }
                                Button{
                                    text: qsTr("node")
                                    height: 30
                                    width: parent.width
                                    onClicked: Pipeline2.run("updateQSGCtrl_taskimage_gridder0", [{type: "editnode"}])
                                }
                                Button{
                                    text: qsTr("delete")
                                    height: 30
                                    width: parent.width
                                    onClicked: Pipeline2.run("taskimage_gridder0_deleteShapes", [])
                                }
                                Button{
                                    text: qsTr("copy")
                                    height: 30
                                    width: parent.width
                                    onClicked: Pipeline2.run("taskimage_gridder0_copyShapes", {})
                                }
                                Button{
                                    text: qsTr("paste")
                                    height: 30
                                    width: parent.width
                                    onClicked: Pipeline2.run("taskimage_gridder0_pasteShapes", {})
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
                                    onClicked: Pipeline2.run("scatterTaskImageShow", {})
                                }
                                Button{
                                    text: qsTr("fit")
                                    height: 30
                                    width: parent.width
                                    onClicked: Pipeline2.run("fitTaskImageShow", {})
                                }
                                Button{
                                    text: qsTr("image")
                                    height: 30
                                    width: parent.width
                                    onClicked: Pipeline2.run("showTransformImageWindow", "getTaskCurrentImage")
                                }

                                Button{
                                    text: qsTr("result")
                                    height: 30
                                    width: parent.width
                                    onClicked: {
                                        result.x = mainwindow.x + mainwindow.width
                                        result.y = mainwindow.y
                                        result.show()
                                    }
                                }

                                Button{
                                    text: qsTr("infer")
                                    height: 30
                                    width: parent.width
                                    onClicked: Pipeline2.run("task_image_listViewSelected", [], {tag: "inference"})
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
                    Rectangle{
                        visible: false
                        width: 120
                        height: 180
                        color: "lightskyblue"
                        anchors.left: parent.left
                        anchors.top: parent.top
                        Column{
                            anchors.fill: parent
                            topPadding: 15
                            Spin{
                                width: 80
                                caption.text: qsTr("count") + ":"
                                ratio: 0.4
                                anchors.horizontalCenter: parent.horizontalCenter
                                spin.onValueChanged: {
                                    if (spin.value < 1)
                                        spin.value = 1
                                    else{
                                        Pipeline2.run("updateROICount", parseInt(spin.value))
                                    }
                                }
                            }
                            Edit{
                                id: roi_x
                                width: 80
                                caption.text: qsTr("x") + ":"
                                ratio: 0.4
                                anchors.horizontalCenter: parent.horizontalCenter
                                input.onAccepted: Pipeline2.run("updateROIGeometry", [parseInt(roi_x.input.text),
                                                                                         parseInt(roi_y.input.text),
                                                                                         parseInt(roi_x.input.text) + parseInt(roi_w.input.text),
                                                                                         parseInt(roi_y.input.text) + parseInt(roi_h.input.text)])
                            }
                            Edit{
                                id: roi_y
                                width: 80
                                caption.text: qsTr("y") + ":"
                                ratio: 0.4
                                anchors.horizontalCenter: parent.horizontalCenter
                                input.onAccepted: Pipeline2.run("updateROIGeometry", [parseInt(roi_x.input.text),
                                                                                         parseInt(roi_y.input.text),
                                                                                         parseInt(roi_x.input.text) + parseInt(roi_w.input.text),
                                                                                         parseInt(roi_y.input.text) + parseInt(roi_h.input.text)])
                            }
                            Edit{
                                id: roi_w
                                width: 80
                                caption.text: qsTr("width") + ":"
                                ratio: 0.4
                                anchors.horizontalCenter: parent.horizontalCenter
                                input.onAccepted: Pipeline2.run("updateROIGeometry", [parseInt(roi_x.input.text),
                                                                                         parseInt(roi_y.input.text),
                                                                                         parseInt(roi_x.input.text) + parseInt(roi_w.input.text),
                                                                                         parseInt(roi_y.input.text) + parseInt(roi_h.input.text)])
                            }
                            Edit{
                                id: roi_h
                                width: 80
                                caption.text: qsTr("height") + ":"
                                ratio: 0.4
                                anchors.horizontalCenter: parent.horizontalCenter
                                input.onAccepted: Pipeline2.run("updateROIGeometry", [parseInt(roi_x.input.text),
                                                                                         parseInt(roi_y.input.text),
                                                                                         parseInt(roi_x.input.text) + parseInt(roi_w.input.text),
                                                                                         parseInt(roi_y.input.text) + parseInt(roi_h.input.text)])
                            }
                            AutoSize{

                            }
                            Component.onCompleted: {
                                Pipeline2.add(function(aInput){
                                    if (aInput["visible"]){
                                        parent.visible = true
                                        if (aInput["count"])
                                            children[0].spin.value = aInput["count"]
                                        if (aInput["x"] !== undefined)
                                            children[1].input.text = Math.round(aInput["x"])
                                        if (aInput["y"] !== undefined)
                                            children[2].input.text = Math.round(aInput["y"])
                                        if (aInput["w"] !== undefined)
                                            children[3].input.text = Math.round(aInput["w"])
                                        if (aInput["h"] !== undefined)
                                            children[4].input.text = Math.round(aInput["h"])
                                    }else
                                        parent.visible = false
                                }, {name: "updateROIGUI"})
                            }
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
                    name: "task_job"
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
                            model: 2
                            delegate: Label{
                                text: ""
                                font.pixelSize: 16
                                leftPadding: 15
                                topPadding: 15
                                height: 25
                            }
                        }
                        Row{
                            leftPadding: 15
                            Label{
                                topPadding: 15
                                text: qsTr("Progress") + ":"
                                font.pixelSize: 16
                            }
                            ProgressBar{
                                height: 10
                                width: 100
                                y: 17
                                minimumValue: 0
                                maximumValue: 100
                                value: 50
                                Component.onCompleted: {
                                    Pipeline2.add(function(aInput){
                                        visible = aInput["type"] !== undefined || (aInput["progress"] === 100)
                                        if (visible)
                                            value = aInput["progress"]
                                    }, {name: "updateTaskJobProgress"})
                                }
                            }
                        }

                        Component.onCompleted: {
                            Pipeline2.add(function(aInput){
                                children[0].text = qsTr("Start time: ") + (aInput["time"] || "")
                                children[1].text = qsTr("State: ") + (aInput["state"] || "")
                            }, {name: "updateTaskJobGUI"})
                        }
                    }
                    Rectangle{
                        anchors.top: parent.top
                        anchors.right: parent.right
                        anchors.rightMargin: 5
                        width: 60
                        height: 30
                        color: "red"
                        radius: 15
                        Label{
                            text: qsTr("server")
                            font.pixelSize: 14
                            anchors.verticalCenter: parent.verticalCenter
                            anchors.horizontalCenter: parent.horizontalCenter
                        }
                        MouseArea{
                            anchors.fill: parent
                            onClicked: {
                                Pipeline2.run("_newObject", {title: qsTr("set server"), content: {ip: "", port: "", auto: false}, tag: {tag: "setServer"}})
                            }
                        }
                        Component.onCompleted: {
                            Pipeline2.find("clientBoardcast")
                            .next(function(aInput){
                                if (aInput["value"] === "socket is connected")
                                    color = "green"
                                else if (aInput["value"] === "socket is unconnected")
                                    color = "red"
                                else if (aInput["value"] === "finding server...")
                                    color = "yellow"
                            })
                        }
                    }
                }
                TabView{
                    width: parent.width
                    height: parent.height - 100
                    Tab{
                        title: qsTr("control")
                        active: true
                        Column{
                            anchors.fill: parent
                            Rectangle{
                                width: parent.width
                                height: 60
                                color: "steelblue"
                                Row{
                                    anchors.fill: parent
                                    leftPadding: 5
                                    /*Edit{
                                        width: 180
                                        caption.text: qsTr("parameter") + ":"
                                        ratio: 0.3
                                        anchors.verticalCenter: parent.verticalCenter
                                    }
                                    Button{
                                        text: qsTr("...")
                                        width: 26
                                        height: width
                                        anchors.verticalCenter: parent.verticalCenter
                                    }

                                    Item{
                                        width: 50
                                        height: parent.height
                                    }*/
                                    Button{
                                        height: 30
                                        width: 60
                                        anchors.verticalCenter: parent.verticalCenter
                                        onClicked: {
                                            Pipeline2.run("startJob", {})
                                        }
                                        style: ButtonStyle{
                                            label: Text{
                                                horizontalAlignment: Text.AlignHCenter
                                                verticalAlignment: Text.AlignVCenter
                                                text: qsTr("train")
                                                color: "red"
                                            }
                                        }
                                    }
                                    Button{
                                        text: qsTr("infer")
                                        width: 60
                                        height: 30
                                        anchors.verticalCenter: parent.verticalCenter
                                        onClicked: Pipeline2.run("task_job_listViewSelected", [], {tag: "startJob"})
                                    }
                                    Button{
                                        text: qsTr("stop")
                                        width: 60
                                        height: 30
                                        anchors.verticalCenter: parent.verticalCenter
                                        onClicked: Pipeline2.run("task_job_listViewSelected", [], {tag: "stopJob"})
                                    }
                                    Button{
                                        text: qsTr("continue")
                                        width: 60
                                        height: 30
                                        anchors.verticalCenter: parent.verticalCenter
                                        onClicked: Pipeline2.run("task_job_listViewSelected", [], {tag: "continueJob"})
                                    }
                                    Button{
                                        text: qsTr("delete")
                                        width: 60
                                        height: 30
                                        anchors.verticalCenter: parent.verticalCenter
                                        onClicked: Pipeline2.run("task_job_listViewSelected", [], {tag: "deleteJob"})
                                    }
                                    Item{
                                        width: parent.width - 370
                                        height: parent.height
                                    }
                                    JobParamButton{
                                        width: 60
                                        height: 30
                                        anchors.verticalCenter: parent.verticalCenter
                                    }
                                }
                            }
                            JobLogPanel{
                                width: parent.width
                                height: parent.height - 60
                            }
                        }
                        onVisibleChanged: {
                            if (visible)
                                result.close()
                        }
                    }
                    Tab{
                        title: qsTr("result")
                        active: true
                        onVisibleChanged: {
                            if (visible){
                                result.x = mainwindow.x + 180 + 1
                                result.y = mainwindow.y + 200
                                result.width = width
                                result.height = height - 3
                                result.show()
                            }else
                                result.close()
                        }
                    }
                }
            }
        }
    }
    ResultWindow{
        id: result
    }
}
