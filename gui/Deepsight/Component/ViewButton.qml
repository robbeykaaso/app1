import QtQuick 2.12
import QtQuick.Controls 2.12
import Pipeline2 1.0

Button{
    text: qsTr("view")
    font.pixelSize: 8
    //font.family:
    onClicked: {
        mn.open()
    }

    background: Rectangle{
        border.color: hovered ? "steelblue" : "gray"
        color: "gainsboro"
        //opacity: hovered ? 0.5 : 1
    }

    Menu{
        id: mn
        width: 90
        MenuItem{
            text: qsTr("result_color")
            font.pixelSize: 10
            onTriggered: Pipeline2.run("_selectColor", {tag: {tag: "modifyResultColor"}})
        }
        MenuItem{
            text: qsTr("show_result")
            font.pixelSize: 10
            onTriggered: Pipeline2.run("modifyResultShow", 0)
        }
        MenuItem{
            text: qsTr("show_label")
            font.pointSize: 10
            onTriggered: Pipeline2.run("modifyLabelShow", 0)
        }
    }
}
