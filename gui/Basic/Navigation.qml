import QtQuick 2.12
import QtQuick.Layouts 1.12
import Pipeline2 1.0

Rectangle {
    property string name
    property var customfont: {"size": 14}

    property var menu: ['1', '2', '3']

    RowLayout {
        spacing: 1
        height: parent.height

        Repeater {
            model: menu
            Row {
                height: parent.height
                leftPadding: 5
                focus: true
                id: _row
                Text {
                    id: control
                    color: '#000'
                    height: 30
                    font.pixelSize: customfont.size
                    verticalAlignment: Text.AlignVCenter
                    text: modelData

                    MouseArea {
                        anchors.fill : parent
                        enabled: index < menu.length - 1
                        hoverEnabled: true
                        onEntered: {
                            control.color = 'blue'
                            control.font.underline = true
                            cursorShape = Qt.PointingHandCursor
                        }
                        onHoveredChanged: {
                            control.color = '#000'
                            control.font.underline = false
                            cursorShape = Qt.ArrowCursor
                        }

                        onClicked:  {
                            menu.splice(index+1, menu.length - index - 1)
                            Pipeline2.run(name + "_updateNavigation", menu, {tag: "manual"})
                        }
                    }
                }

                Text {
                    id: text
                    height: 30
                    width: 15
                    text: index < menu.length - 1 ? qsTr(">") : ''
                    font.pixelSize: customfont.size
                    verticalAlignment: Text.AlignVCenter
                    horizontalAlignment: Text.AlignHCenter
                }
            }
        }
    }
    Component.onCompleted: {
        Pipeline2.add(function(aInput){
            menu = aInput
            return {out: {}}
        }, {name: name + "_updateNavigation", type: "Partial", vtype: []})}
}
