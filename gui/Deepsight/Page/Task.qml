import QtQuick 2.12
import QtQuick.Controls 1.2
import QtQuick.Controls.Styles 1.4
import "../../Component"
import "../Component"
import "../../Basic"

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
        Row{
            anchors.fill: parent
            Rectangle{
                width: 180
                height: parent.height
                List{
                    name: "task_label"
                    anchors.fill: parent
                }
            }
            Column{
                width: parent.width - 360
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
                            name: "usedlabels"
                            size: [2, 2]
                            com: LabelButton{
                            }
                            width: parent.width
                            height: 70
                        }

                        Component.onCompleted: {
                            /*Pipeline2.add(function(aInput){
                                labelgrouptitle.text = qsTr("Group") + ":" + (aInput["key"] || "")
                                var lbls = aInput["val"] || {}
                                var cnt = Object.keys(lbls).length
                                labelgroup.updateViewCount(cnt)
                                var idx = 0
                                for (var i in lbls){
                                    labelgroup.children[idx].clr = lbls[i]["color"] || "steelblue"
                                    labelgroup.children[idx++].text = i
                                }
                            }, {name: "updateLabelGUI"})*/
                        }
                    }
                }
                Grid{
                    id: label_operation
                    property var buttons: [
                        {cap: qsTr("===>"), func: function(){
                            //Pipeline2.run("_newObject", {title: qsTr("new label"), content: {label: ""}, tag: {tag: "newLabel"}})
                        }},
                        {cap: qsTr("<==="), func: function(){
                            //Pipeline2.run("_newObject", {title: qsTr("new label"), content: {label: ""}, tag: {tag: "newLabel"}})
                        }}
                    ]
                    width: parent.width
                    height: parent.height - 100
                    rows: 2
                    columns: 1
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
            }
            Rectangle{
                width: 180
                height: parent.height
                List{
                    name: "used_label"
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
                    name: "task_image"
                    width: parent.width
                    height: 30
                }
                Row{
                    width: parent.width
                    height: 30
                    anchors.bottom: parent.bottom
                    Button{
                        text: qsTr("project")
                        height: 30
                        width: parent.width / 2
                        onClicked: {
                            //Pipeline2.run("_newObject", {title: qsTr("new group"), content: {group: ""}, tag: {tag: "newLabelGroup"}})
                        }
                    }
                    Button{
                        text: qsTr("remove")
                        height: 30
                        width: parent.width / 2
                        onClicked: {
                            //Pipeline2.run("_newObject", {title: qsTr("new group"), content: {group: ""}, tag: {tag: "newLabelGroup"}})
                        }
                    }
                }
            }
        }
    }
    Tab{
        title: qsTr("Job")
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
