import QtQuick 2.12
import QtQuick.Controls 2.5
import "../../Basic"
import "../../Component"
import "../../TreeNodeView"
import Pipeline2 1.0
import QSGBoard 1.0

TWindow{
    width: 800
    height: 700
    caption: qsTr("transform image")

    content:
        Rectangle{
            anchors.fill: parent
            color: "lightskyblue"
            Row{
                anchors.fill: parent
                Rectangle{
                    width: 120
                    height: parent.height
                    List{
                        name: "image_operation"
                        anchors.fill: parent
                    }
                }
                Column{
                    width: parent.width - 240
                    height: parent.height
                    Row{
                        width: parent.width
                        height: 120
                        Column{
                            width: 120
                            height: parent.height
                            Label{
                                text: qsTr("all")
                                anchors.horizontalCenter: parent.horizontalCenter
                            }
                            Button{
                                text: qsTr("add")
                                width: 80
                                height: 30
                                font.pixelSize: 10
                                anchors.horizontalCenter: parent.horizontalCenter
                                onClicked: {
                                    Pipeline2.run("image_operation_listViewSelected", [], {tag: "addOperation"})
                                }
                            }
                        }
                        Column{
                            width: parent.width - 240
                            height: parent.height
                            Label{
                                text: qsTr("current")
                                height: 20
                                anchors.horizontalCenter: parent.horizontalCenter
                            }
                            Button{
                                text: qsTr("showResult")
                                width: 80
                                height: 30
                                font.pixelSize: 10
                                anchors.horizontalCenter: parent.horizontalCenter
                                onClicked: {
                                    Pipeline2.run("transformImage", [])
                                }
                            }
                            Rectangle{
                                width: parent.width
                                height: parent.height - 50
                                color: "steelblue"
                                border.color: "black"
                                TreeNodeView{
                                    root: "imageoperator"
                                    anchors.fill: parent
                                }
                            }
                        }
                        Column{
                            width: 120
                            height: parent.height
                            Label{
                                text: qsTr("used")
                                anchors.horizontalCenter: parent.horizontalCenter
                            }
                            Button{
                                text: qsTr("delete")
                                width: 80
                                height: 30
                                font.pixelSize: 10
                                anchors.horizontalCenter: parent.horizontalCenter
                                onClicked: {
                                    Pipeline2.run("used_operation_listViewSelected", [], {tag: "deleteOperation"})
                                }
                            }
                        }
                    }
                    Row{
                        width: parent.width
                        height: parent.height - 120
                        Column{
                            width: parent.width * 0.5
                            height: parent.height
                            Label{
                                text: qsTr("before")
                                height: 20
                                anchors.horizontalCenter: parent.horizontalCenter
                            }
                            Rectangle{
                                width: parent.width
                                height: parent.height - 20
                                color: "steelblue"
                                border.color: "black"
                                QSGBoard{
                                    name: "imagebefore"
                                    anchors.fill: parent
                                    plugins: [{type: "transform"}]
                                    Component.onDestruction: {
                                        beforeDestroy()
                                    }
                                }
                            }
                        }
                        Column{
                            width: parent.width * 0.5
                            height: parent.height
                            Label{
                                text: qsTr("after")
                                height: 20
                                anchors.horizontalCenter: parent.horizontalCenter
                            }
                            Rectangle{
                                width: parent.width
                                height: parent.height - 20
                                color: "steelblue"
                                border.color: "black"
                                QSGBoard{
                                    name: "imageafter"
                                    anchors.fill: parent
                                    plugins: [{type: "transform"}]
                                    Component.onDestruction: {
                                        beforeDestroy()
                                    }
                                }
                            }
                        }
                    }
                }
                Rectangle{
                    width: 120
                    height: parent.height
                    List{
                        name: "used_operation"
                        anchors.fill: parent
                    }
                }
            }
        }
    footbuttons: [
        {cap: qsTr("OK"), func: function(){
            //Pipeline2.run("_objectNew", {}, service_tag)
            close()
        }},
        {cap: qsTr("Cancel"), func: function(){
            close()
        }}
    ]

    onVisibleChanged: {
        if (visible){
            Pipeline2.run("getProjectCurrentImage", {}, {tag: "transformImage"})
        }
    }
    Component.onCompleted: {
        Pipeline2.run("loadImageOperations", {})
    }
}
