import QtQuick 2.12
import Pipe2 1.0

LogPanel{
    property var cls1: ["system", "train"]
    property var cls2: ["info", "warning", "error"]
    property var content
    property string cur_cls1: "system"
    property string cur_cls2: "info"

    function updateLogList(){
        loglist.clear()
        var cur = content[cur_cls1][cur_cls2]
        for (var i in cur)
            loglist.append({msg: cur[i]})
    }

    loglevel.onCurrentIndexChanged: {
        var idx = loglevel.currentIndex
        if (cls2[idx] !== cur_cls2){
            cur_cls2 = cls2[idx]
            updateLogList()
        }
    }

    train.onClicked: {
        train.background.color = "red"
        system.background.color = "transparent"
        if (cur_cls1 != "train"){
            cur_cls1 = "train"
            updateLogList()
        }
    }

    system.onClicked: {
        train.background.color = "transparent"
        system.background.color = "red"
        if (cur_cls1 != "system"){
            cur_cls1 = "system"
            updateLogList()
        }
    }

    closepanel.onClicked: {

    }

    Pipe2{
        param: {"name": "addLogRecord"}
        func: function(aInput){
            var tp = aInput["type"], lev = aInput["level"]
            if (cls1.indexOf(tp) >= 0 && cls2.indexOf(lev) >= 0){
                content[tp][lev].push(aInput["msg"])
                if (tp === cur_cls1 && lev === cur_cls2)
                    loglist.append({msg: aInput["msg"]})
            }
        }
    }

    Component.onCompleted: {
        content = {}
        for (var i in cls1){
            content[cls1[i]] = {}
            for (var j in cls2)
                content[cls1[i]][cls2[j]] = []
        }
        system.background.color = "red"
    }
}
