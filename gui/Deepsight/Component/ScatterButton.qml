import QtQuick 2.12
import QtQuick.Controls 2.5
import Pipeline2 1.0

Button{
    property string name
    text: qsTr("scatter")
    font.pixelSize: 8
    onClicked: mn.open()

    background: Rectangle{
        border.color: hovered ? "steelblue" : "gray"
        color: "gainsboro"
        opacity: hovered ? 0.5 : 1
    }

    Menu{
        id: mn
        width: 80
        MenuItem{
            text: qsTr("all")
            font.pixelSize: 10
            onClicked: Pipeline2.run("scatter" + name + "ImageShow", {})
        }
        Component.onCompleted: {
            Pipeline2.find("updateProjectChannelCountGUI")
            .next(function(aInput){
                for (var i = children.length - 1; i > 0; --i)
                    removeItem(itemAt(i))
                for (var j = 0; j < aInput; ++j){
                    var src = "import QtQuick 2.12; import QtQuick.Controls 2.5; import Pipeline2 1.0;"
                    src += "MenuItem{"
                    src += "text: '" + j + "';"
                    src += "font.pixelSize: 10;"
                    src += "onClicked: {"
                    src += "Pipeline2.run('switch" + name + "FirstImageIndex', {index: " + j + "})"
                    src += "}}"
                    addItem(Qt.createQmlObject(src, mn))
                }
            }, {}, {vtype: 0})
        }
    }
}
