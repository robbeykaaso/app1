import QtQuick 2.12
import QtQuick.Controls 1.2
import QtQuick.Controls.Styles 1.4
import "../../Component"
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
                        Label{
                            text: qsTr("Name: ")
                            font.pixelSize: 16
                            leftPadding: 15
                            topPadding: 15
                        }
                        Label{
                            text: qsTr("Type: ")
                            font.pixelSize: 16
                            leftPadding: 15
                            topPadding: 15
                        }
                    }
                }
                Row{
                    width: parent.width
                    height: parent.height - 100
                    spacing: parent.width * 0.15
                    leftPadding: spacing
                    Button{
                        width: parent.width * 0.3
                        height: width
                        text: qsTr("Open")
                        anchors.verticalCenter: parent.verticalCenter
                        onClicked: {
                            Pipeline2.run("project_task_listViewSelected", [], {tag: "openTask"})
                        }
                    }
                    Button{
                        width: parent.width * 0.3
                        height: width
                        text: qsTr("Delete")
                        anchors.verticalCenter: parent.verticalCenter
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
                    anchors.fill: parent
                }
            }
        }
    }
}
