import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Window 2.12
import QtQuick.Controls.Universal 2.3

Window{
    id: root
    //flags: Qt.Dialog
    property string caption
    property alias content: mid.data
    property var titlebuttons
    property var footbuttons
    flags: Qt.FramelessWindowHint
    modality: Qt.WindowModal
    width: 400
    height: 300

    MouseArea{
        property int coor_x
        property int coor_y
        width: parent.width
        height: Screen.desktopAvailableHeight * 0.03
        z: - 1
        onPressed: function(aInput){
            coor_x = aInput["x"]
            coor_y = aInput["y"]
        }
        onPositionChanged: function(aInput){
            root.x += aInput["x"] - coor_x
            root.y += aInput["y"] - coor_y
        }
    }

    Column{
        anchors.fill: parent
        Row{
            width: parent.width
            height: Screen.desktopAvailableHeight * 0.03
            Rectangle{
                id: titbar
                height: parent.height
                width: parent.width - btns.parent.width
                Text{
                    leftPadding: parent.width * 0.05
                    anchors.verticalCenter: parent.verticalCenter
                    text: caption
                }
            }
            Row{
                width: childrenRect.width
                height: parent.height

                Repeater {
                    id: btns
                    model: ListModel {
                    }

                    delegate: Button {
                        width: parent.height
                        height: parent.height
                        text: cap
                        background: Rectangle{
                            border.color: parent.hovered ? "gray" : "transparent"
                        }
                    }
                }

                Component.onCompleted: {
                    if (titlebuttons){
                        for (var i in titlebuttons){
                            btns.model.append({cap: titlebuttons[i]["cap"]})
                            btns.itemAt(btns.count - 1).clicked.connect(titlebuttons[i]["func"])
                        }
                    }
                    btns.model.append({cap: "X"})
                    btns.itemAt(btns.count - 1).clicked.connect(function(){
                        close()
                    })
                }
            }
        }
        Item{
            id: mid
            width: parent.width
            height: parent.height - Screen.desktopAvailableHeight * (0.045 + (footbuttons ? 0.03 : 0))
        }
        Row{
            anchors.right: parent.right
            anchors.rightMargin: height * 0.4
            width: childrenRect.width
            height: footbuttons ? Screen.desktopAvailableHeight * 0.045 : 0
            spacing: 5

            Repeater{
                id: btns2
                model: ListModel{

                }
                delegate: Button{
                    height: parent.height * 0.5
                    width: height * 2.5
                    text: cap
                    font.pixelSize: 12
                    anchors.verticalCenter: parent.verticalCenter
                    background: Rectangle{
                        color: parent.hovered ? "transparent" : "gray"
                        border.color: parent.hovered ? "gray" : "transparent"
                    }
                }
            }

            Component.onCompleted: {
                if (footbuttons)
                    for (var i in footbuttons){
                        btns2.model.append({cap: footbuttons[i]["cap"]})
                        btns2.itemAt(btns2.count - 1).clicked.connect(footbuttons[i]["func"])
                    }
            }
        }
    }
}
