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
                                  {
                                      width: 600,
                                      height: 600,
                                      objects: {"shp_0": {type: "poly",
                                                   points: [50, 50, 200, 200, 200, 50, 50, 50],
                                                   color: "red",
                                                   width: 5
                                                 },
                                                 "shp_1": {type: "ellipse",
                                                   center: [400, 400],
                                                   radius: [300, 200],
                                                   color: "blue",
                                                   width: 5
                                                 },
                                                 "img_2": {type: "image",
                                                    path: "F:/3M/B4DT/DF Mark/V1-1.bmp"
                                                 }}
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
        plugins: [{type: "transform"}]
        anchors.fill: parent
    }
}
