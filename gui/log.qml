import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Controls.Universal 2.3
import Pipe2 1.0
import Pipeline2 1.0

Column{
    property string current_task: ""
    property string log_type: "system"
    property var messages: {"train": {}, "system": {}}
    Rectangle{
        width: parent.width
        height: parent.height / 5
        color: "green"
        Row{
            anchors.fill: parent
            Row{
                width: parent.width * 0.9 - parent.height
                height: parent.height
                spacing: -1
                Button{
                    id: system
                    width: parent.width * 0.1
                    height: parent.height
                    text: qsTr("System")
                    onClicked: {
                        log_type = "system"
                        switchLog()
                    }
                }
                Button{
                    id: train
                    width: parent.width * 0.1
                    height: parent.height
                    text: qsTr("Training")
                    onClicked: {
                        log_type = "train"
                        switchLog()
                    }
                }
            }
            Rectangle{
                height: parent.height
                width: parent.width * 0.1
                color: "transparent"
                border.color: "black"
                ComboBox{
                    id: loglevel
                    anchors.fill: parent
                    currentIndex: 0
                    model: ["Info", "Warning", "Error"]
                    font.pixelSize: 14

                    function actValue(){
                        if (currentIndex == 0)
                            return "info"
                        else if (currentIndex == 1)
                            return "warning"
                        else
                            return "error"
                    }

                   /* onCaptionChanged: function(aValue){
                        value = aValue
                        switchLog()
                    }*/
                }
            }
            Button{
                width: parent.height
                height: parent.height
                text: "X"
                onClicked: {
                   // UIManager.setCommand({signal2: "showLogPanel", type: "nullptr", param: {visible: false}}, null)
                }
            }
        }
    }
    ListView{
        id: loglist
        width: parent.width * 0.98
        height: parent.height * 0.8
        x: parent.width * 0.02
        spacing: 2
        clip: true
        model: loglistmodel

        delegate: Text{
            text: msg
            wrapMode: Text.WordWrap
            width: parent.width
        }
        ScrollBar.vertical: ScrollBar {
            onVisualPositionChanged: function(){
               // var start = Math.ceil(imagelistmodel.count * visualPosition),
              //  end = Math.ceil(imagelistmodel.count * (visualPosition + visualSize))
                //console.log(start + ';' + end)
            }
        }
        MouseArea{
            anchors.fill: parent
            acceptedButtons: Qt.RightButton
            onClicked: {
                if (mouse.button === Qt.RightButton){
                    logmenu.x = mouse.x
                    logmenu.y = mouse.y
                    logmenu.open()
                }
            }
            Menu{
                id: logmenu
                MenuItem{
                    text: qsTr("Clear")
                    onClicked: {
                        clearLog()
                    }
                }
            }
        }
    }
    ListModel{
        id: loglistmodel
        ListElement{
            msg: "hello"
        }
        ListElement{
            msg: "world"
        }
    }

    function addNewStatus(aTask, aLevel, aStatus, aType){
        if (aType === "system")
            aTask = ""
        if (messages[aType][aLevel] === undefined)
            messages[aType][aLevel] = {}
        if (messages[aType][aLevel][aTask] === undefined)
            messages[aType][aLevel][aTask] = []
        messages[aType][aLevel][aTask].push(aStatus)

        if ((aType === "system" || aTask === current_task) && aLevel === loglevel.actValue() && aType === log_type){
            loglistmodel.append({msg: aStatus})
            loglist.currentIndex = loglist.count - 1
        }
    }

    function clearLog(){
        loglistmodel.clear()
        var tsk = current_task
        if (log_type == "system")
            tsk = ""
        if (messages[log_type][loglevel.actValue()] && messages[log_type][loglevel.actValue()][tsk]){
            messages[log_type][loglevel.actValue()][tsk] = []
        }
    }

    function switchLog(){
        loglistmodel.clear()
        var tsk = current_task
        if (log_type == "system")
            tsk = ""
        if (messages[log_type][loglevel.actValue()] && messages[log_type][loglevel.actValue()][tsk]){
            for (var i in messages[log_type][loglevel.actValue()][tsk])
                loglistmodel.append({msg: messages[log_type][loglevel.actValue()][tsk][i]})
            loglist.currentIndex = loglistmodel.count - 1
        }
    }

    Pipe2{
        param: {"name": "switchLogType"}
        func: function(aInput){
            current_task = aInput["task"]
            switchLog()
        }
    }

    Pipe2{
        param: {"name": "showLogPanel"}
        func: function(aInput){
            visible = aInput["visible"]
        }
    }

    Pipe2{
        param: {"name": "addLogRecord"}
        func: function(aInput){
            addNewStatus(aInput["task_id"], aInput["log_level"], aInput["log_msg"], aInput["log_type"])
        }
    }
}
