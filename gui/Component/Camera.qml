import QtQuick 2.12
import "../Basic"
import "../TreeNodeView"
import QSGBoard 1.0
import Pipeline2 1.0

TWindow{
    id: camera
    property string name: "camera1"

    property string type: "real"
    property var param: {
        "real":{
            "id": "DS",
            "config": "W:/configs/MV-CE200-10GC.mfs",
            "type": "cam_hikvision",
            "name": "Ds MVCE200 10 GC Camera #1"
        },
        "sim":{
            "id": "MV-CE200-10GC",
            "directory": "./image",
            "filepattern": "^((?!\\/\\.).)*\\.(?:png|bmp|jpg|jpeg$)",
            "type": "cam_sim",
            "loopable": true
        },
        "running": {
            "triggermode": 1
        }
    }

    width: 700
    height: 600
    caption: qsTr("camera")
    content: Rectangle{
        anchors.fill: parent
        color: "gray"
        Row{
            anchors.fill: parent
            QSGBoard{
                id: cambrd
                name: camera.name
                plugins: [{type: "transform"}]
                width: parent.width * 0.7
                height: parent.height
                Component.onDestruction: {
                    beforeDestroy()
                }
            }
            Rectangle{
                width: parent.width * 0.3
                height: parent.height
                color: "tomato"
                Column{
                    anchors.fill: parent
                    Text{
                        width: parent.width
                        height: 30
                        text: "state: offline"
                        color: "white"
                        font.pixelSize: 14
                        verticalAlignment: Text.AlignVCenter
                        horizontalAlignment: Text.AlignHCenter

                        Component.onCompleted: {
                            Pipeline2.add(function(aInput){
                                text = aInput
                            }, {vtype: ""}).previous(camera.name + "_cameraStated")
                        }
                    }
                    Combo{
                        width: parent.width
                        height: 30
                        combo.model: ["real", "sim"]
                        caption.text: qsTr("type") + ":"
                        caption.color: "white"
                        ratio: 0.3
                        combo.onCurrentTextChanged: {
                            camera.type = combo.currentText
                            Pipeline2.run(camera.name + "loadTreeView", {data: camera.param[camera.type]})
                        }
                    }
                    Rectangle{
                        width: parent.width
                        height: parent.height - 60
                        color: "honeydew"
                        Column{
                            anchors.fill: parent
                            TreeNodeView{
                                root: camera.name
                                width: parent.width
                                height: parent.height * 0.5
                            }
                            TreeNodeView{
                                root: camera.name + "running"
                                width: parent.width
                                height: parent.height * 0.5
                            }
                        }
                    }
                }

            }
        }
    }
    footbuttons: [
        /*{cap: "trans", func: function(){
            Pipeline2.run("updateQSGCtrl_" + name, [{type: "transform"}])
        }},
        {cap: "draw", func: function(){
            Pipeline2.run("updateQSGCtrl_" + name, [{type: "drawfree"}])
        }},*/
        {cap: "start", func: function(){
            Pipeline2.run("addCamera", {name: name, initial: camera.param[camera.type]})
            Pipeline2.run(name + "_turnCamera", {on: true})
        }},
        {cap: "stop", func: function(){
            Pipeline2.run(name + "_turnCamera", {on: false})
        }},
        {cap: "set", func: function(){
            Pipeline2.run(name + "_setCamera", camera.param["running"]);
        }},
        {cap: "capture", func: function(){
            Pipeline2.run(name + "_captureCamera")
        }}
    ]
    onVisibleChanged: {
        if (visible){
            Pipeline2.run(camera.name + "loadTreeView", {data: camera.param[camera.type]})
            Pipeline2.run(camera.name + "runningloadTreeView", {data: camera.param["running"]})
        }
    }
}
