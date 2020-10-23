import QtQuick 2.12
//import QtQuick.Window 2.12
import QtQuick.Controls 2.5
import QtQuick.Controls.Universal 2.3
import "Component"
import "Basic"
import "Deepsight/Page"
import "Deepsight/Component"
import Pipeline2 1.0

ApplicationWindow {
    id: mainwindow
    property int orgx
    property int orgy
    visible: true
    width: 800
    height: 700
    //Universal.theme: Universal.Dark
    title: qsTr("DeepsightInspection V4.0/REA V7.1.1")

    onXChanged: function(aInput){
        Pipeline2.run("mainWindowPositionChanged", {dx: aInput - orgx})
        orgx = aInput
    }
    onYChanged: function(aInput){
        Pipeline2.run("mainWindowPositionChanged", {dy: aInput - orgy})
        orgy = aInput
    }
    Column{
        anchors.fill: parent
        /*Status{
            id: status
            name: "title"
            width: parent.width
            height: 60
            Button{
                visible: false
                anchors.right: parent.right
                text: qsTr("Back")
                height: parent.height
                width: height * 2
                onClicked: {
                    var desp = []
                    for (var i = 0; i < status.statuslist.count - 1; ++i)  //parent is not status here
                        desp.push(status.statuslist.get(i).cap)
                    Pipeline2.run("title_updateNavigation", desp)
                }
                Component.onCompleted: {
                    Pipeline2.find("title_updateNavigation").next(function(aInput){
                        visible = aInput.length > 1
                        return {out: {}}
                    }, {}, {vtype: []})
                }
            }
        }*/
        Navigation{
            name: "title"
            width: parent.width
            height: 60
            color: "lightskyblue"
        }

        Component{
            id: user
            User{

            }
        }
        Component{
            id: project
            Project{

            }
        }
        Component{
            id: task
            Task{

            }
        }

        StackView{
            property var items: [user.createObject(), project.createObject(), task.createObject()]
            property int lastpage: - 1
            width: parent.width
            height: parent.height - 60
            //initialItem: items[0]
            Component.onCompleted: {
                Pipeline2.find("title_updateNavigation")
                .next(function(aInput){
                    var idx = aInput.length - 1
                    if (lastpage !== idx){
                        lastpage = idx
                        replace(items[lastpage])
                    }
                    return {out: {}}
                }, {tag: "manual"}, {name: "switch_page", vtype: []})
            }
        }
    }
    PWindow{

    }
    MakeSure{

    }
    MsgDialog{

    }
    File{

    }
    ColorSelect{

    }
    Progress{

    }
    LabelFilterWindow{

    }
    StageFilterWindow{

    }
}
