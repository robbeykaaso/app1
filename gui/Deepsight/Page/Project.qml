import QtQuick 2.12
import QtQuick.Controls 1.2
import QtQuick.Controls.Styles 1.4
import "../../Component"
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
        Row{
            anchors.fill: parent
            Rectangle{
                width: 180
                height: parent.height
                List{
                    name: "project_label"
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

                        }
                    }
                }
                Grid{
                    id: label_operation
                    property var buttons: [
                        {cap: qsTr("New"), func: function(){
                            // Pipeline2.run("_newObject", {title: qsTr("new task"), content: {name: "", type: ""}, tag: {tag: "newTask"}})
                        }},
                        {cap: qsTr("Delete"), func: function(){
                            // Pipeline2.run("user_listViewSelected", [], {tag: "deleteProject"})
                        }}

                    ]
                    width: parent.width
                    height: parent.height - 100
                    rows: 1
                    columns: 2
                    Repeater{
                        model: 2
                        delegate: Item{
                            width: parent.width / parent.columns
                            height: parent.height / parent.rows
                            Button{
                                width: parent.width * 0.6
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
        Row{
            anchors.fill: parent
            Rectangle{
                width: 180
                height: parent.height
                List{
                    name: "project_image"
                    width: parent.width
                    height: parent.height - 30
                }
                Row{
                    width: parent.width
                    height: 30
                    anchors.bottom: parent.bottom
                    Button{
                        text: qsTr("import")
                        height: 30
                        width: parent.width / 3
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
