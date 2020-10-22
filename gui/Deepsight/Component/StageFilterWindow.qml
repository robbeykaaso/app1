import QtQuick 2.12
import QtQuick.Controls 2.5
import "../../Basic"
import "../..//Component"
import Pipeline2 1.0

TWindow{
    property var service_tag
    property var entries
    width: 200
    height: 400
    caption: qsTr("stage statistics")
    //modality: Qt.NonModal

    content:
        List{
            id: lst
            name: "stage_filter"
            anchors.fill: parent
        }
    footbuttons: [
        {cap: qsTr("OK"), func: function(){
            Pipeline2.run("_stageStatisticsShowed", {}, service_tag)
            close()
        }},
        {cap: qsTr("Cancel"), func: function(){
            close()
        }}
    ]
    Component.onCompleted: {
        Pipeline2.add(function(aInput){
            var ret = []
            var sels = lst.selects
            for (var i in sels)
                ret.push(entries[sels[i]]["entry"][0])
            return {data: {type: "stage", value: ret}, out: {}}
        }, {name: "_stageStatisticsShowed", type: "Partial"})

        Pipeline2.add(function(aInput){
            service_tag = {tag: aInput["tag"]}
            entries = aInput["data"]
            lst.updateList(aInput)
            show()
        }, {name: "showStageStatistics", type: "Delegate", param: {delegate: "_stageStatisticsShowed"}})
    }
}
