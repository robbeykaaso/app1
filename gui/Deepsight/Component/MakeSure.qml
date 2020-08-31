import QtQuick 2.12
import QtQuick.Controls 2.5
import "../../Basic"
import Pipeline2 1.0

TWindow{
    property var service_tag
    width: 200
    height: 130
    caption: qsTr("make sure")

    content:
        Rectangle{
            anchors.fill: parent
            color: "lightskyblue"
            Label{
                id: advise
                anchors.verticalCenter: parent.verticalCenter
                anchors.horizontalCenter: parent.horizontalCenter
            }
        }
    footbuttons: [
        {cap: qsTr("OK"), func: function(){
            Pipeline2.run("_makeSured", [], service_tag)
            close()
        }},
        {cap: qsTr("Cancel"), func: function(){
            close()
        }}
    ]
    Component.onCompleted: {
        Pipeline2.add(function(aInput){
            return {out: {}}
        }, {name: "_makeSured", type: "Partial", vtype: []})

        Pipeline2.add(function(aInput){
            caption = aInput["title"] || qsTr("")
            advise.text = aInput["content"] || qsTr("")
            service_tag = aInput["tag"] || {tag: "manual"}
            show()
        }, {name: "_makeSure", type: "Delegate", param: {delegate: "_makeSured"}})
    }
}
