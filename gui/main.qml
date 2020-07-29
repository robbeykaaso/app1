import QtQuick 2.12
import QtQuick.Window 2.12
import QtQuick.Controls 2.5
import QtQuick.Controls.Universal 2.3
import QSGBoard 1.0
import Pipeline2 1.0

ApplicationWindow {
    id: mainwindow
    property var view_cfg
    visible: true
    width: 600
    height: 400
    //width: Screen.desktopAvailableWidth
    //height: Screen.desktopAvailableHeight
    //visibility: Window.Maximized

    Universal.theme: Universal.Dark
    //Universal.accent: Universal.Green
    //Universal.background: Universal.Cyan

    menuBar: MenuBar{
        Menu{
            title: qsTr("file")
            MenuItem {
                text: qsTr("show")
                onClicked: {
                    Pipeline2.run("testQSGShow", view_cfg)
                }
            }
            /*MenuItem{
                text: qsTr("close")
            }*/
        }
        Menu{
            title: qsTr("view")

            delegate: MenuItem {
                id: menuItem
                implicitWidth: 200
                implicitHeight: 40

                arrow: Canvas {
                    x: parent.width - width
                    implicitWidth: 40
                    implicitHeight: 40
                    visible: menuItem.subMenu
                    onPaint: {
                        var ctx = getContext("2d")
                        ctx.fillStyle = menuItem.highlighted ? "#ffffff" : "#21be2b"
                        ctx.moveTo(15, 15)
                        ctx.lineTo(width - 15, height / 2)
                        ctx.lineTo(15, height - 15)
                        ctx.closePath()
                        ctx.fill()
                    }
                }

                indicator: Item {
                    implicitWidth: 40
                    implicitHeight: 40
                    Rectangle {
                        width: 14
                        height: 14
                        anchors.centerIn: parent
                        visible: menuItem.checkable
                        border.color: "#21be2b"
                        radius: 3
                        Rectangle {
                            width: 8
                            height: 8
                            anchors.centerIn: parent
                            visible: menuItem.checked
                            color: "#21be2b"
                            radius: 2
                        }
                    }
                }

                contentItem: Text {
                    leftPadding: menuItem.indicator.width
                    rightPadding: menuItem.arrow.width
                    text: menuItem.text
                    font: menuItem.font
                    opacity: enabled ? 1.0 : 0.3
                    color: menuItem.highlighted ? "#ffffff" : "#21be2b"
                    horizontalAlignment: Text.AlignLeft
                    verticalAlignment: Text.AlignVCenter
                    elide: Text.ElideRight
                }

                background: Rectangle {
                    implicitWidth: 200
                    implicitHeight: 40
                    opacity: enabled ? 1 : 0.3
                    color: menuItem.highlighted ? "#21be2b" : "transparent"
                }
            }

            Action {
                text: qsTr("face")
                checkable: true
                shortcut: "Ctrl+V"
                onTriggered: {
                    view_cfg["face"] = 100 - view_cfg["face"]
                    Pipeline2.run("testQSGShow", view_cfg)
                }
            }
            Action {
                text: qsTr("arrow")
                checkable: true
                shortcut: "Ctrl+A"
                onTriggered: {
                    view_cfg["arrow"]["visible"] = !view_cfg["arrow"]["visible"]
                    Pipeline2.run("testQSGShow", view_cfg)
                }
            }
            Action {
                text: qsTr("text")
                checkable: true
                shortcut: "Ctrl+T"
                onTriggered: {
                    view_cfg["text"]["visible"] = !view_cfg["text"]["visible"]
                    Pipeline2.run("testQSGShow", view_cfg)
                }
            }
        }
        Component.onCompleted: {
            view_cfg = {
                face: 0,
                arrow: {
                    visible: false
                },
                text: {
                    visible: false,
                    location: "bottom"
                }
            }
        }
    }
    contentData:
        QSGBoard{
            name: "testbrd"
            plugins: [{type: "transform"}]
            anchors.fill: parent
            Component.onDestruction: {
                beforeDestroy()
            }
        }
}
