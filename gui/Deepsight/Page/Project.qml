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
                             Pipeline2.run("_newObject", {title: qsTr("new task"), content: {name: "", type: ""}, tag: {tag: "newTask"}})
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
                            name: "labels"
                            size: [2, 2]
                            com: LabelButton{
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
                            }, {name: "updateLabelGUI"})
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
                        Button{
                            text: qsTr("All")
                            height: parent.height
                            width: 60
                        }
                        Search {
                            text: qsTr("filter")
                            height: parent.height
                            width: parent.width - 60
                        }
                    }
                    List{
                        name: "project_image"
                        width: parent.width
                        height: parent.height - 90
                    }
                    Row{
                        anchors.horizontalCenter: parent.horizontalCenter
                        width: 90
                        height: 30
                        Edit{
                            caption.text: qsTr("Page: ")
                            input.text: "2"
                            background.color: "lightskyblue"
                            width: 60
                            ratio: 0.5
                        }
                        Label{
                            text: "/10"
                            anchors.verticalCenter: parent.verticalCenter
                        }
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
                            text: qsTr("export")
                            height: 30
                            width: parent.width / 3
                        }
                        Button{
                            text: qsTr("save")
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
                                font.pixelSize: 16
                                leftPadding: 15
                                topPadding: 15
                            }
                        }

                        Component.onCompleted: {

                        }
                    }
                }
                Row{
                    width: parent.width
                    height: parent.height - 100
                    QSGBoard{
                        name: "projbrd"
                        plugins: [{type: "transform"}]
                        width: parent.width - 60
                        height: parent.height
                        Component.onDestruction: {
                            beforeDestroy()
                        }
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
                            }
                            Button{
                                text: qsTr("free")
                                height: 30
                                width: parent.width
                            }
                            Button{
                                text: qsTr("rect")
                                height: 30
                                width: parent.width
                            }
                            Button{
                                text: qsTr("ellipse")
                                height: 30
                                width: parent.width
                            }
                            Button{
                                text: qsTr("scatter")
                                height: 30
                                width: parent.width
                            }
                        }
                    }
                }
            }
        }
    }
}
