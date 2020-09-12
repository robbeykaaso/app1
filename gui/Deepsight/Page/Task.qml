import QtQuick 2.12
import QtQuick.Controls 1.2
import QtQuick.Controls.Styles 1.4
import "../../Component"
import "../Component"
import "../../Basic"
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
        title: qsTr("Labels")
        active: true
        Row{
            anchors.fill: parent
            Rectangle{
                width: 180
                height: parent.height
                TabView{
                    anchors.fill: parent
                    Tab{
                        title: qsTr("used")
                        active: true
                        Column{
                            anchors.fill: parent
                            List{
                                name: "task_label"
                                width: parent.width
                                height: parent.height - 30
                            }
                            Row{
                                width: parent.width
                                height: 30
                                Button{
                                    text: qsTr("remove")
                                    height: 30
                                    width: parent.width
                                    onClicked: {
                                        //Pipeline2.run("_newObject", {title: qsTr("new group"), content: {group: ""}, tag: {tag: "newLabelGroup"}})
                                    }
                                }
                            }
                        }
                    }
                    Tab{
                        title: qsTr("candidate")
                        active: true
                        Column{
                            anchors.fill: parent
                            List{
                                name: "candidate_label"
                                width: parent.width
                                height: parent.height - 30
                            }
                            Row{
                                width: parent.width
                                height: 30
                                Button{
                                    text: qsTr("use")
                                    height: 30
                                    width: parent.width
                                    onClicked: {
                                        //Pipeline2.run("_newObject", {title: qsTr("new group"), content: {group: ""}, tag: {tag: "newLabelGroup"}})
                                    }
                                }
                            }
                            Component.onCompleted: {
                                Pipeline2.find("project_label_updateListView").nextL("candidate_label_updateListView")
                            }
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
                            text: qsTr("Candidate")
                            font.pixelSize: 14
                            padding: 5
                            width: parent.width
                            height: 30
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }
                        Gridder{
                            name: "candidatelabels"
                            size: [2, 2]
                            com: LabelButton{
                            }
                            width: parent.width
                            height: 70
                        }

                        Component.onCompleted: {

                        }
                    }
                }
                Grid{
                    id: label_operation
                    property var buttons: [
                        {cap: qsTr("add"), func: function(){
                            //Pipeline2.run("_newObject", {title: qsTr("new label"), content: {label: ""}, tag: {tag: "newLabel"}})
                        }},
                        {cap: qsTr("remove"), func: function(){
                            //Pipeline2.run("_newObject", {title: qsTr("new label"), content: {label: ""}, tag: {tag: "newLabel"}})
                        }}
                    ]
                    width: parent.width
                    height: parent.height - 200
                    rows: 1
                    columns: 2
                    Repeater{
                        model: 2
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
                Rectangle{
                    width: parent.width
                    height: 100
                    Column{
                        anchors.fill: parent
                        Gridder{
                            name: "usedlabels"
                            size: [2, 2]
                            com: LabelButton{
                            }
                            width: parent.width
                            height: 70
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
                        Component.onCompleted: {

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
                TabView{
                    anchors.fill: parent
                    Tab{
                        title: qsTr("used")
                        active: true
                        Column{
                            anchors.fill: parent
                            List{
                                name: "task_image"
                                width: parent.width
                                height: parent.height - 30
                            }
                            Row{
                                width: parent.width
                                height: 30
                                Button{
                                    text: qsTr("remove")
                                    height: 30
                                    width: parent.width
                                    onClicked: {
                                        //Pipeline2.run("_newObject", {title: qsTr("new group"), content: {group: ""}, tag: {tag: "newLabelGroup"}})
                                    }
                                }
                            }
                        }
                    }
                    Tab{
                        title: qsTr("candidate")
                        active: true
                        Column{
                            anchors.fill: parent
                            PageList{
                                name: "candidate_image"
                                width: parent.width
                                height: parent.height - 30
                            }
                            Row{
                                width: parent.width
                                height: 30
                                Button{
                                    text: qsTr("use")
                                    height: 30
                                    width: parent.width
                                    onClicked: {
                                        //Pipeline2.run("_newObject", {title: qsTr("new group"), content: {group: ""}, tag: {tag: "newLabelGroup"}})
                                    }
                                }
                            }
                            Component.onCompleted: {
                                Pipeline2.find("project_image_updateListView").nextL("candidate_image_updateListView")
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
                    anchors.fill: parent
                }
            }
        }
    }
}
