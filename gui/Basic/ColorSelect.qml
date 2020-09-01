import QtQuick 2.12
import QtQuick.Dialogs 1.2
import Pipeline2 1.0

ColorDialog{
    property string name
    property var service_tag

    onAccepted: {
        Pipeline2.run(name + "_colorSelected", "", service_tag)
        close()
    }
    onRejected: {
        close()
    }
    Component.onCompleted: {
        Pipeline2.add(function(aInput){
            return {data: color.toString(), out: {}}
        }, {name: name + "_colorSelected", type: "Partial", vtype: ""})

        Pipeline2.add(function(aInput){
            service_tag = aInput["tag"] || {tag: "manual"}
            open()
        }, {name: name + "_selectColor", type: "Delegate", param: {delegate: name + "_colorSelected"}})
    }
}
