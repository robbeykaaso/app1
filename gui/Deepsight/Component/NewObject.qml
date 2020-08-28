import QtQuick 2.12
import "../../Basic"
import Pipeline2 1.0

TWindow{
    property var service_tag
    width: 200
    height: 100
    caption: qsTr("new object")

    Component{
        id: entry
        Edit{
            property string key
            anchors.horizontalCenter: parent.horizontalCenter
            width: 180
            caption.text: qsTr(key) + ":"
            ratio: 0.3
        }
    }

    content:
        Rectangle{
            anchors.fill: parent
            color: "lightskyblue"
            Column{
                id: sets
                anchors.fill: parent
                topPadding: 10
            }
        }
    footbuttons: [
        {cap: qsTr("OK"), func: function(){
            Pipeline2.run("_objectNew", {}, service_tag)
            close()
        }},
        {cap: qsTr("Cancel"), func: function(){
            close()
        }}
    ]
    Component.onCompleted: {
        Pipeline2.add(function(aInput){
            var dt = {}
            var itms = sets.children
            for (var i = 0; i < itms.length; ++i)
                dt[itms[i].key] = itms[i].input.text
            return {data: dt, out: {}}
        }, {name: "_objectNew", type: "Partial"})

        Pipeline2.add(function(aInput){
            caption = aInput["title"] || qsTr("new object")
            var cnt = aInput["content"]
            for (var j = 0; j < sets.children.length; ++j)
                sets.children[j].destroy()
            for (var i in cnt)
                entry.createObject(sets, {key: i})
            height = Object.keys(cnt).length * 30 + 100
            service_tag = aInput["tag"] || {tag: "manual"}
            show()
        }, {name: "_newObject", type: "Delegate", param: {delegate: "_objectNew"}})
    }
}