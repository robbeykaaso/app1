import QtQuick 2.12
import QtQuick.Controls 2.5
import "../Basic"
import Pipe2 1.0

TWindow{
    id: root
    width: 300
    height: 100
    caption: qsTr("progress")
    content: Column{
        leftPadding: parent.width * 0.1
        width: parent.width * 0.8
        height: parent.height
        spacing: height * 0.1
        Text{
            id: title
            text: ""
            font.pixelSize: 12
            height: parent.height * 0.3
            width: parent.width
        }

        ProgressBar{
            property int sum
            property int cur
            property string tit
            id: bar
            height: parent.height * 0.5
            width: parent.width
            from: 0
            to: 1
            value: 0

            function updateProgress(aInfo){
                if (aInfo["title"]){
                    sum = aInfo["sum"]
                    cur = 0
                    tit = aInfo["title"]
                }else{
                    if (aInfo["step"] !== undefined)
                        cur += aInfo["step"]
                    else
                        ++cur
                }
                value = cur / sum
                title.text = tit + " (" + Math.round(value * 100) + "%)"

                if (value == 1){
                    root.close()
                }
                else if (!root.visible){
                    root.show()
                }
                return {out: [{out: value}]}
            }

            Pipe2{
                param: {
                    "name": "updateProgress",
                    "type": "Partial"
                }
                func: bar.updateProgress
            }
        }
    }
}
