import QtQuick 2.12
import QtQuick.Controls 2.5
import Pipeline2 1.0

Button{
    property string name
    property var headbutton
    property string clr: "steelblue"
    width: parent.width / parent.columns
    height: parent.height / parent.rows
    text: name
    background: Rectangle{
        color: clr
        opacity: hovered ? 0.5 : 1
        border.color: "white"
    }

    Button{
        height: 14
        width: 14
        contentItem: Text{
            text: headbutton ? headbutton["cap"] : ""
            color: "white"
            font.pixelSize: 14
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }
        background: Item{

        }
        anchors.right: parent.right
        anchors.top: parent.top
        onClicked: {
            if (headbutton)
                headbutton["func"](parent.text)
        }
    }
}
