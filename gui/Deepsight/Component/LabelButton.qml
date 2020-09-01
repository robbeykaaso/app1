import QtQuick 2.12
import QtQuick.Controls 2.5

Button{
    property string name
    property string clr: "steelblue"
    width: parent.width / parent.columns
    height: parent.height / parent.rows
    text: name
    background: Rectangle{
        color: clr
        opacity: hovered ? 0.5 : 1
        border.color: "white"
    }
    Component.onCompleted: {
        //console.log(name)
    }
    Button{
        height: 14
        width: 14
        contentItem: Text{
            text: "X"
            color: "white"
            font.pixelSize: 14
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }
        background: Item{

        }
        anchors.right: parent.right
        anchors.top: parent.top
    }
}
