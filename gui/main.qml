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
                text: qsTr("open...")
                onClicked: {
                    Pipeline2.run("testQSGShow", view_cfg)
                }
            }
            MenuItem{
                text: qsTr("close")
            }
        }
        Menu{
            title: qsTr("view")
            MenuItem {
                text: qsTr("face")
                onClicked: {
                    if (view_cfg["face"] > 100)
                        view_cfg["face"] -= 200
                    else
                        view_cfg["face"] += 200
                    Pipeline2.run("testQSGShow", view_cfg)
                }
            }
            MenuItem {
                text: qsTr("arrow")
                onClicked: {
                    view_cfg["arrow"] = !view_cfg["arrow"]
                    Pipeline2.run("testQSGShow", view_cfg)
                }
            }
            MenuItem {
                text: qsTr("text")
                onClicked: {
                    view_cfg["text"]["visible"] = !view_cfg["text"]["visible"]
                    Pipeline2.run("testQSGShow", view_cfg)
                }
            }
        }
        Component.onCompleted: {
            view_cfg = {
                face: 100,
                arrow: false,
                text: {
                    visible: false
                }
            }
        }
    }
    contentData: QSGBoard{
        name: "testbrd"
        plugins: [{type: "transform"}]
        anchors.fill: parent
        Component.onDestruction: {
            beforeDestroy()
        }
    }
}
