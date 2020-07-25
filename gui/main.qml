import QtQuick 2.12
import QtQuick.Window 2.12
import QtQuick.Controls 2.5
import QtQuick.Controls.Universal 2.3
import QSGBoard 1.0
import Pipeline2 1.0

ApplicationWindow {
    id: mainwindow
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
                    Pipeline2.run("testQSGShow",
                                  {objects: [{type: "poly",
                                                points: [0, 0, 200, 200, 200, 0, 0, 0],
                                                color: "green",
                                                width: 10
                                          }]
                                  })
                }
            }
            MenuItem{
                text: qsTr("close")
            }
        }
        Menu{
            title: qsTr("view")
        }
    }
    contentData: QSGBoard{
        name: "testbrd"
        anchors.fill: parent
    }
}
