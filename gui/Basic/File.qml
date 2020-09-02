import QtQuick 2.12
import QtQuick.Dialogs 1.2
import Pipeline2 1.0

FileDialog {
    property string name
    property var service_tag

    title: qsTr("Please choose files")
    selectMultiple: true
    selectFolder: false
   // nameFilters: ["Image files (*.jpg *.png *.jpeg *.bmp)"] //"All files(*)"
    onAccepted: {
        Pipeline2.run(name + "_fileSelected", [], service_tag)
    }
    Component.onCompleted: {
        Pipeline2.add(function(aInput){
            var pths = []
            for (var i in fileUrls){
                //https://stackoverflow.com/questions/24927850/get-the-path-from-a-qml-url
                pths.push(decodeURIComponent(fileUrls[i].replace(/^(file:\/{3})|(qrc:\/{2})|(http:\/{2})/,"")))
                // unescape html codes like '%23' for '#'
                //pths += fileUrls[i].substring(8, fileUrls[i].length) + ";"
            }
            return {data: pths, out: {}}
        }, {name: name + "_fileSelected", type: "Partial", vtype: []})

        Pipeline2.add(function(aInput){
            selectFolder = aInput["folder"]
            if (selectFolder){
                title = aInput["title"] || qsTr("Please choose folder")
                nameFilters = ""
            }else{
                title = aInput["title"] || qsTr("Please choose files")
                nameFilters = aInput["filter"]
                selectMultiple = true
            }
            service_tag = aInput["tag"] || {tag: "manual"}
            open()
        }, {name: name + "_selectFile", type: "Delegate", param: {delegate: name + "_fileSelected"}})
    }
}
