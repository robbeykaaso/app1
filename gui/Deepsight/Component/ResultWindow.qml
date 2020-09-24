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
                    width: parent.width * 0.5
                    height: parent.height * 0.9
                    spacing: 10
                    THistogram{
                        id: histo
                        width: parent.width
                        height: parent.height * 0.85
                        //oneThreshold: true
                        onThresholdChanged: function(aMax, aMin){
                            Pipeline2.run("thresholdChanged", [aMax, aMin])
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
                            //thresholdChanged(maxthreshold.x, minthreshold.x)
                        }
                    }
                }
                Matrix{
                    name: "result_confuse"
                    fontsize: 12
                    width: parent.width * 0.5
                    height: parent.height
                    rowcap.text: "hello"
                    colcap.text: "world"
                    content: [[1, 2], [3, 4], [5, 6]]
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
