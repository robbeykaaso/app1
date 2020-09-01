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
        title: qsTr("Project")
        Row{
            anchors.fill: parent
            Rectangle{
                width: 180
                height: parent.height
                List{
                    name: "user"
                    anchors.fill: parent
                    //width: parent.width
                    //height: parent.height - 30
                }
                /*Row{
                    width: parent.width
                    height: 30
                    anchors.bottom: parent.bottom
                    Button{
                        text: qsTr("import")
                        height: 30
                        width: parent.width
                    }
                }*/
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
                                children[2].text = qsTr("Channel: ") + (aInput["channel"] || "")
                            }, {name: "updateProjectGUI"})
                        }
                    }
                }
                Grid{
                    id: operation
                    property var buttons: [
                        {cap: qsTr("New"), func: function(){
                            Pipeline2.run("_newObject", {title: qsTr("new project"), content: {name: "", channel: ""}, tag: {tag: "newProject"}})
                        }},
                        {cap: qsTr("Import"), func: function(){
                            Pipeline2.run("_newObject", {title: qsTr("import project"), content: {id: ""}, tag: {tag: "newProject"}})
                        }},
                        {cap: qsTr("Delete"), func: function(){
                            Pipeline2.run("_makeSure", {title: qsTr("delete project"), content: "Make sure to delete?", tag: {tag: "deleteProject"}})
                        }},
                        {cap: qsTr("Open"), func: function(){
                            Pipeline2.run("user_listViewSelected", [], {tag: "openProject"})
                        }}
                    ]
                    width: parent.width
                    height: parent.height - 100
                    rows: 2
                    columns: 2
                    Repeater{
                        model: 4
                        delegate: Item{
                            width: parent.width / parent.columns
                            height: parent.height / parent.rows
                            Button{
                                width: parent.width * 0.4
                                height: width
                                text: operation.buttons[index].cap
                                anchors.verticalCenter: parent.verticalCenter
                                anchors.horizontalCenter: parent.horizontalCenter
                                onClicked: operation.buttons[index].func()
                            }
                        }
                    }
                }
            }
        }
    }
}
