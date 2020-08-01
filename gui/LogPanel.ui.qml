import QtQuick 2.12
import QtQuick.Controls 2.5

Column {
    id: column
    height: 180
    property alias loglist: loglist
    property alias closepanel: closepanel
    property alias loglevel: loglevel
    property alias train: train
    property alias system: system
    width: 540
    Row {
        id: row
        width: parent.width
        height: parent.height * 0.3
        Row {
            width: parent.width * 0.8 - parent.height
            height: parent.height
            spacing: -1
            Button {
                id: system
                width: parent.width * 0.2
                height: parent.height
                text: qsTr("System")
                background: Rectangle {
                    border.color: "gray"
                    color: "transparent"
                }
            }
            Button {
                id: train
                width: parent.width * 0.2
                height: parent.height
                text: qsTr("Train")
                background: Rectangle {
                    border.color: "gray"
                    color: "transparent"
                }
            }
        }
        Rectangle {
            height: parent.height
            width: parent.width * 0.2
            color: "transparent"
            border.color: "gray"
            ComboBox {
                id: loglevel
                font.weight: Font.ExtraLight
                font.family: "Courier"
                anchors.fill: parent
                currentIndex: 0
                model: [qsTr("Info"), qsTr("Warning"), qsTr("Error")]
                font.pixelSize: 13
            }
        }
        Button {
            id: closepanel
            width: parent.height
            height: parent.height
            text: "X"
            background: Rectangle {
                border.color: "gray"
                color: "transparent"
            }
        }
    }

    ListView {
        width: parent.width * 0.98
        height: parent.height * 0.7
        x: parent.width * 0.02
        spacing: 2
        topMargin: spacing
        clip: true
        model: ListModel {
            id: loglist
        }

        delegate: Text {
            text: msg
            color: "white"
            font.pixelSize: 16
            wrapMode: Text.WordWrap
            width: parent.width
        }
        ScrollBar.vertical: ScrollBar {
        }
        MouseArea {
            anchors.fill: parent
            acceptedButtons: Qt.RightButton
            Menu {
                id: logmenu
                MenuItem {
                    text: qsTr("Clear")
                }
            }
        }
    }
}
