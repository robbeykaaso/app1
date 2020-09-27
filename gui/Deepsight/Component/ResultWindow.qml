import QtQuick 2.12
import QtQuick.Controls 2.5
import "../../Basic"
import "../../Component"
import "../../TreeNodeView"
import Pipeline2 1.0
import QSGBoard 1.0

TWindow{
    width: 600
    height: 700
    caption: qsTr("result")
    modality: Qt.NonModal

    content:
        Rectangle{
            anchors.fill: parent
            color: "lightskyblue"
            Column{
                anchors.fill: parent
                Matrix{
                    name: "result_abstract"
                    fontsize: 12
                    width: parent.width * 0.9
                    height: 60
                    //content: [["date", "job_type", "number_of_train_data", "number_of_val_data", "number_of_test_data", "training_time", "min_loss", "epoch", "precision", "recall", "overkill"],
                    //          ["2020-05-19 12:05:04", "training", 318, 186, 489, "0:05:22.133895", 0.101811, 30, 0.7878895, 0.9282603333333, 0.0436530000]]
                }
                LineChart{
                    width: parent.width
                    height: (parent.height - 60) * 0.5
                }
                Row{
                    width: parent.width
                    height: (parent.height - 60) * 0.5
                    Column{
                        id: histo_panel
                        width: parent.width * 0.5
                        height: parent.height * 0.9
                        spacing: 10
                        THistogram{
                            id: histo
                            width: parent.width
                            height: parent.height * 0.85
                            //oneThreshold: true
                            onThresholdChanged: function(aMax, aMin){
                                Pipeline2.run("thresholdChanged", {min: aMax, max: aMin})
                            }
                        }
                        Track{
                            property var intervals: []
                            property var histogramdata: []
                            property double value: 0.1
                            property int idx: 0
                            anchors.horizontalCenter: parent.horizontalCenter
                            height: parent.height * 0.04
                            width: parent.width * 0.8
                            interval: 100
                            caption.text: qsTr("Interval") + ":"
                            ratio: 0.2

                            function updateInterval(){
                                if (intervals.length > 1 && histogramdata.length > 1){
                                    histo.drawHistoGram(histogramdata[idx])
                                }else
                                    histo.clear()
                            }

                            onIndexChanged: function(aIndex){
                                idx = aIndex
                                updateInterval()
                            }
                            Component.onCompleted: {
                                Pipeline2.add(function(aInput){
                                    if (aInput["show"]){
                                        histo_panel.visible = true
                                        //mtx_panel.width = mtx_panel.parent.width * 0.5
                                    }else{
                                        histo_panel.visible = false
                                        //mtx_panel.width = mtx_panel.parent.width
                                        return {out: {}}
                                    }

                                    intervals = []
                                    histogramdata = []
                                    if (aInput["histogram"]){
                                        interval = Object.keys(aInput["histogram"]).length - 1
                                        for (var i in aInput["histogram"]){
                                            intervals.push(i)
                                            histogramdata.push(aInput["histogram"][i])
                                        }
                                    }
                                    if (aInput["threshold"]){
                                        histo.thresholdmax.x = aInput["threshold"][0]
                                        histo.thresholdmin.x = aInput["threshold"][1]
                                    }
                                    updateInterval()
                                    return {out: {}}
                                }, {name: "_updateTHistogramGUI"})
                            }
                        }

                        Track{
                            property double value: 0
                            anchors.horizontalCenter: parent.horizontalCenter
                            height: parent.height * 0.04
                            width: parent.width * 0.8
                            interval: 100
                            caption.text: qsTr("IOU") + ":"
                            ratio: 0.2

                            onIndexChanged: function(aIndex){
                                value = aIndex / 100
                                Pipeline2.run("thresholdChanged", {iou: value})
                            }
                        }
                    }
                    Column{
                        id: mtx_panel
                        width: histo_panel.visible ? parent.width * 0.5 : parent.width
                        height: parent.height
                        Row{
                            width: parent.width
                            height: 30
                            Combo{
                                property bool notFirst: false
                                height: parent.height
                                width: 120
                                leftPadding: parent.width * 0.05
                                caption.text: ""
                                ratio: 0
                                combo.model: []
                                combo.onCurrentIndexChanged: {
                                    if (notFirst)
                                        Pipeline2.run("updateConfuseMatrix", {for_image: !combo.currentIndex})
                                    notFirst = true

                                }
                                Component.onCompleted: {
                                    Pipeline2.find("updateConfuseMatrix").next(function(aInput){
                                        if (aInput["has_object"] !== undefined){
                                            combo.model = aInput["has_object"] ? ["for image", "for object"] : ["for image"]
                                        }
                                    })
                                }
                            }
                        }
                        Matrix{
                            name: "result_confuse"
                            fontsize: 12
                            width: parent.width
                            height: parent.height - 30
                            rowcap.text: "hello"
                            colcap.text: "world"
                            content: [[1, 2], [3, 4], [5, 6]]
                        }
                    }
                }
            }
        }
    Component.onCompleted: {
        Pipeline2.add(function(aInput){
            if (aInput["dx"] !== undefined)
                x = x + aInput["dx"]
            if (aInput["dy"] !== undefined)
                y = y + aInput["dy"]
        }, {name: "mainWindowPositionChanged"})
    }
}
