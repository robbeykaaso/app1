import QtQuick 2.12
import QtQuick.Controls 2.12
import Pipeline2 1.0

Button{
    text: qsTr("stage")
    font.pixelSize: 8
    //font.family:
    onClicked: {
        mn.open()
    }

    background: Rectangle{
        border.color: hovered ? "steelblue" : "gray"
        color: "gainsboro"
        opacity: hovered ? 0.5 : 1
    }

    Menu{
        id: mn
        width: 100
        MenuItem{
            text: qsTr("train")
            onTriggered: Pipeline2.run("setImageStage", "train")
        }
        MenuItem{
            text: qsTr("test")
            onTriggered: Pipeline2.run("setImageStage", "test")
        }
        MenuItem{
            text: qsTr("validation")
            onTriggered: Pipeline2.run("setImageStage", "validation")
        }
        MenuItem{
            text: qsTr("none")
            onTriggered: Pipeline2.run("setImageStage", "none")
        }
    }
}
