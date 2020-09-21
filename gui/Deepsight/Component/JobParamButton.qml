import QtQuick 2.12
import QtQuick.Controls 2.12
import Pipeline2 1.0

Button{
    text: qsTr("parameter")
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
            text: qsTr("path")
            onTriggered:
                Pipeline2.run("_selectFile", {folder: false, filter: ["Parameter files (*.json)"], tag: {tag: "setJobParameter"}})
        }
        MenuItem{
            text: qsTr("edit")
            onTriggered:
                Pipeline2.run("editJobParam", {})
        }
    }
}
