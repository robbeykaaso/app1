import QtQuick 2.12
import Pipe2 1.0

Status0{
    property string name

    Pipe2{
        param: {"name": name + "_updateStatus",
                "vtype": []}
        func: function(aInput){
            statuslist.clear()
            for (var i in aInput)
                statuslist.append({cap: aInput[i]})
            return {out: {}}
        }
    }
}
