import QtQuick 2.12
import QtQuick.Controls 2.5

Row {
    property alias caption: caption
    property alias combo: cmb
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
        leftPadding: 5
        width: parent.width * 0.3
        height: parent.height * 0.8
    }

    ComboBox {
        id: cmb
        width: parent.width * 0.6
        height: parent.height * 0.8
        anchors.verticalCenter: parent.verticalCenter
        model: ["hello", "world"]
        font.pixelSize: 12
        background: Rectangle {
            color: "white"
            border.color: "transparent"
            border.width: 1
        }
    }
}
