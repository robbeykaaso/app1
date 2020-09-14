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
        TabView{
            anchors.fill: parent
            Tab{
                title: qsTr("used")
                active: true
                Row{
                    anchors.fill: parent
                    Column{
                        width: 180
                        height: parent.height
                        List{
                            name: "task_label"
                            selectSuffix: "task_"
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
                                    Pipeline2.run("task_label_listViewSelected", [], {tag: "removeTaskLabelGroup"})
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
                                    id: candidatelabelgroup
                                    name: "candidatelabels2"
                                    size: [2, 2]
                                    com: LabelButton{
                                        enabled: false
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
                            color: "lightskyblue"
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
                                    name: "usedlabels"
                                    size: [2, 2]
                                    com: LabelButton{
                                        headbutton: {
                                            "cap": "X",
                                            "func": function(aLabel){

                                            }
                                        }
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
                title: qsTr("candidate")
                active: true
                Row{
                    anchors.fill: parent
                    Column{
                        width: 180
                        height: parent.height
                        List{
                            name: "candidate_label"
                            selectSuffix: "candidate_"
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
                                    Pipeline2.run("candidate_label_listViewSelected", [], {tag: "addTaskLabelGroup"})
                                }
                            }
                        }
                        Component.onCompleted: {
                            Pipeline2.find("project_label_updateListView").nextL("candidate_label_updateListView")
                        }
                    }
                    Column{
                        width: parent.width - 180
                        height: 100
                        Label{
                            id: candidatelabelgrouptitle
                            text: qsTr("Group") + ":"
                            font.pixelSize: 14
                            padding: 5
                            width: parent.width
                            height: 30
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }
                        Gridder{
                            id: candidatelabelgroup2
                            name: "candidatelabels2"
                            size: [2, 2]
                            com: LabelButton{
                                headbutton: {
                                    "cap": "+",
                                    "func": function(aLabel){

                                    }
                                }
                            }
                            width: parent.width
                            height: 70
                        }

                        Component.onCompleted: {
                            Pipeline2.add(function(aInput){
                                candidatelabelgrouptitle.text = qsTr("Group") + ":" + (aInput["key"] || "")
                                var lbls = aInput["val"] || {}
                                var cnt = Object.keys(lbls).length
                                candidatelabelgroup2.updateViewCount(cnt)
                                var idx = 0
                                for (var i in lbls){
                                    candidatelabelgroup2.children[idx].clr = lbls[i]["color"] || "steelblue"
                                    candidatelabelgroup2.children[idx++].text = i
                                }
                            }, {name: "updateCandidateLabelGUI2"})
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
