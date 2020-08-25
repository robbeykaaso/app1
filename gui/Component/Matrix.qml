import QtQuick 2.12
import QtQuick.Controls 2.5
import QtQuick.Layouts 1.12
import Pipeline2 1.0
import "../Basic"

Nest{
    property string name
    property alias rowcap: rowcap
    property alias colcap: colcap
    property var content: []
    rows: 5
    columns: 5
    size: [1, 1, 1, 4, 4, 1, 2, 2, 2, 2, 2, 2, 2, 2]
    Item {

    }

    Label {
        id: rowcap
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
    }

    Label {
        id: colcap
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
    }

    Repeater{
        model: ListModel{
            id: mdl
            ListElement{
                cap: "0"
            }
            ListElement{
                cap: "1"
            }
            ListElement{
                cap: "2"
            }
            ListElement{
                cap: "3"
            }
        }
        delegate: Label{
            text: cap
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            MouseArea{
                anchors.fill: parent
                onClicked: {
                    Pipeline2.run(name + "_matrixSelected", index, {tag: "manual"})
                }
            }
        }
    }

    function updateGUI(aFit){
        size = [1, 1, 1]
        columns = 2 * content[0].length + 1
        size.push(2 * content[0].length)
        rows = 2 * content.length + 1
        size.push(2 * content.length)
        size.push(1)
        var sum = content[0].length * content.length
        for (var i = 0; i < sum; ++i){
            size.push(2)
            size.push(2)
        }
        mdl.clear()
        for (var j in content)
            for (var k in content[j]){
                mdl.append({cap: content[j][k].toString()})
            }
        if (aFit)
            fitLayout()
    }

    Component.onCompleted: {
        updateGUI(false)

        Pipeline2.add(function(aInput){
            return {out: {}}
        }, {name: name + "_matrixSelected", type: "Partial", vtype: 0})

        Pipeline2.add(function(aInput){
            if (aInput["rowcap"])
                rowcap.text = aInput["rowcap"]
            if (aInput["colcap"])
                colcap.text = aInput["colcap"]
            if (aInput["content"]){
                content = aInput["content"]
                updateGUI(true)
            }
        }, {name: name + "_updateMatrix"})
    }
}