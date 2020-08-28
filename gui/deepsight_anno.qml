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
    visible: true
    width: 800
    height: 600
    //Universal.theme: Universal.Dark
    Column{
        anchors.fill: parent
        Status{
            name: "title"
            width: parent.width
            height: 60
            Button{
                property string lastState
                id: back
                visible: false
                anchors.right: parent.right
                text: qsTr("Back")
                height: parent.height
                width: height * 2
                onClicked: {
                    if (lastState == "User")
                        Pipeline2.run("openUser", 0)
                    else
                        Pipeline2.run("user_listViewSelected", [], {tag: "openProject"})
                }
            }
            Component.onCompleted: {
                Pipeline2.find(name + "_updateStatus").next(function(aInput){
                    back.visible = aInput.length > 1
                    //console.log(aInput.length)
                    back.lastState = aInput.length === 2 ? "User" : "Project"
                    return {out: {}}
                }, {}, {vtype: []})
            }
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
                Pipeline2.add(function(aInput){
                    var idx = Math.round(aInput)
                    if (lastpage !== idx){
                        lastpage = idx
                        replace(items[lastpage])
                    }
                }, {name: "switchPage", vtype: 0})
                Pipeline2.run("loadUser", 0)
            }
        }
    }
    NewObject{

    }
    MsgDialog{

    }
}
