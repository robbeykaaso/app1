import QtQuick 2.12
import QtQuick.Controls 2.5
import Pipeline2 1.0

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

    onClicked: {
        var svs = "modifyLabelColor"
        Pipeline2.find("_selectColor")
        .next(function(aInput){
            return {out: [{out: [text, aInput], next: svs},
                          {out: [], next: "project_label_listViewSelected"}]}
        }, {tag: svs}, {name: "getButtonLabelText", vtype: ""})
        .nextB(svs)
        .next("project_label_listViewSelected", {tag: svs})
        Pipeline2.run("_selectColor", {tag: {tag: svs}})
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
        onClicked: {
            Pipeline2.run("deleteLabel", parent.text)
        }
    }
}
