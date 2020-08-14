import QtQuick 2.12
import QtQuick.Controls 2.5
import "../Basic"
import "../TreeNodeView"
import Pipeline2 1.0

TWindow{
    id: io
    property string name: "io1"

    property var param:{
                        "config": "",
                        "id": "USB_7230,0",
                        "in_descriptor": {
                                             "channel_in0": 1,
                                             "channel_in1": 2,
                                             "channel_in10": 1024,
                                             "channel_in11": 2048,
                                             "channel_in12": 4096,
                                             "channel_in13": 8192,
                                             "channel_in14": 16384,
                                             "channel_in15": 32768,
                                             "channel_in2": 4,
                                             "channel_in3": 8,
                                             "channel_in4": 16,
                                             "channel_in5": 32,
                                             "channel_in6": 64,
                                             "channel_in7": 128,
                                             "channel_in8": 256,
                                             "channel_in9": 512},
                        "name": "Digital IO controller",
                        "out_descriptor": {
                                              "channel_out0": 1,
                                              "channel_out1": 2,
                                              "channel_out10": 1024,
                                              "channel_out11": 2048,
                                              "channel_out12": 4096,
                                              "channel_out13": 8192,
                                              "channel_out14": 16384,
                                              "channel_out15": 32768,
                                              "channel_out2": 4,
                                              "channel_out3": 8,
                                              "channel_out4": 16,
                                              "channel_out5": 32,
                                              "channel_out6": 64,
                                              "channel_out7": 128,
                                              "channel_out8": 256,
                                              "channel_out9": 512},
                        "type": "io_adlink_usb"
                    }

    width: 700
    height: 600
    caption: qsTr("io")
    content: Rectangle{
        anchors.fill: parent
        color: "gray"
        Row{
            anchors.fill: parent
            Column{
                width: parent.width * 0.7
                height: parent.height
                Text{
                    text: qsTr("receive:")
                    width: parent.width
                    height: parent.height * 0.1
                    color: "white"
                    font.pixelSize: 16
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
                Rectangle{
                    border.color: "black"
                    width: parent.width
                    height: parent.height * 0.4
                    ListView{
                        anchors.fill: parent
                        clip: true
                        model: ListModel{

                            Component.onCompleted: {
                                Pipeline2.add(function(aInput){
                                    append({cap: aInput})
                                }, {vtype: ""}).previous(io.name + "_IOReceived")
                            }
                        }
                        delegate: Text{
                            topPadding: 5
                            leftPadding: 5
                            text: cap
                            font.pixelSize: 14
                        }
                        ScrollBar.vertical: ScrollBar{

                        }
                    }
                }
                Text{
                    text: qsTr("send:")
                    width: parent.width
                    height: parent.height * 0.1
                    color: "white"
                    font.pixelSize: 16
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
                Rectangle{
                    border.color: "black"
                    width: parent.width
                    height: parent.height * 0.4
                    TextEdit{
                        id: send
                        padding: 5
                        anchors.fill: parent
                        font.pixelSize: 14
                    }
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
                            }, {vtype: ""}).previous(io.name + "_IOStated")
                        }
                    }
                    Rectangle{
                        width: parent.width
                        height: parent.height - 30
                        color: "honeydew"
                        TreeNodeView{
                            root: io.name
                            anchors.fill: parent
                        }
                    }
                }

            }
        }
    }
    footbuttons:  [
        {cap: "start", func: function(){
            Pipeline2.run("addIO", {name: name, initial: param})
            Pipeline2.run(name + "_turnIO", {on: true})
        }},
        {cap: "stop", func: function(){
            Pipeline2.run(name + "_turnIO", {on: false})
        }},
        {cap: "send", func: function(){
            Pipeline2.run(name + "_sendIO", JSON.parse(send.text))
            Pipeline2.run(name + "_IOReceived", "hello")
        }}
    ]
    onVisibleChanged: {
        if (visible){
            Pipeline2.run(name + "loadTreeView", {data: io.param})
        }
    }
}
