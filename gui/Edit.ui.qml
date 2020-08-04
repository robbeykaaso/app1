import QtQuick 2.12

Row {
    property alias caption: caption
    property alias background: bak
    property alias input: val
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

    Rectangle {
        id: bak
        color: "white"
        border.color: "gray"
        width: parent.width * 0.6
        height: parent.height * 0.8
        anchors.verticalCenter: parent.verticalCenter
        TextInput {
            id: val
            clip: true
            anchors.fill: parent
            anchors.leftMargin: 5
            selectByMouse: true
            //focus: true
            text: ""
            horizontalAlignment: Text.AlignLeft
            verticalAlignment: Text.AlignVCenter
            font.pixelSize: 12
        }
    }
}
