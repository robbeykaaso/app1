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
    caption: qsTr("label statistics")
    //modality: Qt.NonModal

    content:
        List{
            id: lst
            name: "label_filter"
            anchors.fill: parent
        }
    footbuttons: [
        {cap: qsTr("OK"), func: function(){
            Pipeline2.run("_labelStatisticsShowed", {}, service_tag)
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
                ret = ret.concat(entries[sels[i]]["entry"][3])
            return {data: {type: "label", value: ret}, out: {}}
        }, {name: "_labelStatisticsShowed", type: "Partial"})

        Pipeline2.add(function(aInput){
            service_tag = {tag: aInput["tag"]}
            entries = aInput["data"]
            lst.updateList(aInput)
            show()
        }, {name: "showLabelStatistics", type: "Delegate", param: {delegate: "_labelStatisticsShowed"}})
    }
}
