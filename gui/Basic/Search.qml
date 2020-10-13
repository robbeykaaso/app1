import QtQuick 2.12
import Pipe2 1.0
import Pipeline2 1.0

FocusScope {
    id: wrapper
    property string name
    property alias text: input.text
    property alias hint: hint.text
    property alias prefix: prefix.text

    signal accepted

    Rectangle {
        anchors.fill: parent
        border.color: "#707070"
        color: "white"//"#c1c1c1"
        radius: 4

        Text {
            id: hint
            anchors { fill: parent; leftMargin: 14 }
            verticalAlignment: Text.AlignVCenter
            text: "Enter word"
            font.pixelSize: 18
            color: "#707070"
            opacity: input.displayText.length ? 0 : 1
        }

        Text {
            id: prefix
            anchors { left: parent.left; leftMargin: 14; verticalCenter: parent.verticalCenter }
            verticalAlignment: Text.AlignVCenter
            font.pixelSize: 18
            color: "#707070"
            opacity: !hint.opacity
        }

        TextInput {
            id: input
            focus: true
            anchors { left: prefix.right; right: parent.right; top: parent.top; bottom: parent.bottom }
            verticalAlignment: Text.AlignVCenter
            font.pixelSize: 18
            color: "#707070"
            onAccepted: {
                wrapper.accepted()
                focus = false
                Pipeline2.run(name + "_Searched", "", {tag: "manual"})
            }
        }

        Image {
            source: "../../resource/icon-search.png"
            anchors.right: parent.right
            anchors.rightMargin: 12
            anchors.verticalCenter: parent.verticalCenter
            MouseArea {
                anchors { fill: parent; margins: -10 }
                onClicked: {
                    wrapper.accepted()
                    Pipeline2.run(name + "_Searched", "", {tag: "manual"})
                }
            }
        }

        Pipe2{
            param: {
                "name": name + "_Searched",
                "type": "Partial",
                "vtype": ""
            }
            func: function(aInput){
                return {data: input.text, out: {}}
            }
        }
    }
}
