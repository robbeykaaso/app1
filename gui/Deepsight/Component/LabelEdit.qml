import QtQuick 2.12
import QtQuick.Controls 2.5
import Pipeline2 1.0

Rectangle{
    property bool sync: false
    property string group: "shape"
    property string label
    property int fontsize: 16
    signal updatelabel(string aLabel)
    width: 80
    height: 30
    color: "white"
    border.color: "black"
    Label{
        text: label
        font.pixelSize: fontsize
        anchors.verticalCenter: parent.verticalCenter
        anchors.horizontalCenter: parent.horizontalCenter
    }
    MouseArea{
        anchors.fill: parent
        onClicked: {
            lblmenu.open()
        }
    }
    Menu{
        id: lblmenu
        y: parent.height
        width: parent.width
        /*MenuItem{
            text: qsTr("world")
            onClicked: {
                label = text
                updatelabel(label)
            }
        }
        MenuItem{
            text: qsTr("hello")
        }*/
    }
    /*Component{
        id: lbl
        MenuItem{
            onClicked: {
                label = text
                updatelabel(label)
            }
        }
    }*/

    Component.onDestruction: {
        if (sync)
            Pipeline2.find("projectLabelChanged").removeNext("projectLabelChanged_UI_" + group)
    }

    function updateMenu(aLabels){
        var lbls = aLabels[group]
        for (var i = lblmenu.count - 1; i >= 0; --i)
            lblmenu.removeItem(lblmenu.itemAt(i))
        for (var j in lbls){
            var src = "import QtQuick 2.12; import QtQuick.Controls 2.5;"
            src += "MenuItem{"
            src += "text: '" + j + "';"
            src += "font.pixelSize: " + fontsize + ";"
            src += "onClicked: {"
            src += "label = text;"
            src += "updatelabel(label);"
            src += "}}"
            lblmenu.addItem(Qt.createQmlObject(src, lblmenu))
            //lblmenu.addItem(lbl.createObject(lblmenu, {text: j}))
        }
    }

    Component.onCompleted: {
        if (sync){
            Pipeline2.find("projectLabelChanged").next(function(aInput){
                updateMenu(aInput)
            }, {}, {name: "projectLabelChanged_UI_" + group})
        }
    }
}