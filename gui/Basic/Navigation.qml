import QtQuick 2.0
import QtQuick.Layouts 1.1

Rectangle {
    property var menu: [
        {"menuName": '一级菜单一级菜单一级菜单一级菜单'},
        {'menuName': '二级'},
        {'menuName': '三级菜单'}]

    RowLayout {
        spacing: 1

        Repeater {
            model: menu
            Row {
                height: 30
                focus: true
                id: _row
                Text {
                    id: control
                    color: '#000'
                    font.pixelSize: 10
                    verticalAlignment: Text.AlignVCenter
                    text: modelData.menuName

                    MouseArea {
                        anchors.fill : parent
                        hoverEnabled: true
                        onEntered: {
                            control.color = 'blue'
                            control.font.underline = true
                        }
                        onHoveredChanged: {
                            control.color = '#000'
                            control.font.underline = false
                        }

                        onClicked:  {
                            menu.splice(index+1, menu.length - index - 1)
                            menu = menu
                        }
                    }
                }

                Text {
                    id: text
                    width: 15
                    text: index < menu.length - 1 ? qsTr(">") : ''
                    font.pixelSize: 10
                    verticalAlignment: Text.AlignVCenter
                    horizontalAlignment: Text.AlignHCenter
                }
            }
        }
    }
}
