import QtQuick 2.12
import QtQuick.Controls 2.5

Row {
    property alias caption: caption
    property alias check: chk
    width: 120
    height: 30

    spacing: width * 0.05

    Text {
        id: caption
        text: "hello:"
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        anchors.verticalCenter: parent.verticalCenter
        font.pixelSize: 12
        width: parent.width * 0.3
        height: parent.height * 0.8
        leftPadding: 5
    }

    CheckBox {
        id: chk
        width: parent.width * 0.6
        height: parent.height * 0.8
        anchors.verticalCenter: parent.verticalCenter
        indicator: Rectangle {
            implicitWidth: parent.height * 0.6
            implicitHeight: parent.height * 0.6
            anchors.verticalCenter: parent.verticalCenter
            anchors.horizontalCenter: parent.horizontalCenter
            radius: width / 4
            color: "white"
            border.color: "gray"

            Rectangle {
                width: parent.height * 0.55
                height: parent.height * 0.55
                anchors.verticalCenter: parent.verticalCenter
                anchors.horizontalCenter: parent.horizontalCenter
                radius: width / 4
                color: "gray"
                visible: parent.parent.checked
            }
        }
    }
}
