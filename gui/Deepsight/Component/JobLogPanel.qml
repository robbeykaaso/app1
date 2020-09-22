import QtQuick 2.12
import QtQuick.Controls 2.5
import Pipeline2 1.0

Rectangle{
    width: parent.width
    height: parent.height - 60
    color: "black"
    ListView{
        leftMargin: 5
        anchors.fill: parent
        clip: true
        model: ListModel{
           /* ListElement{
                cap: "..."
            }
            ListElement{
                cap: "..."
            }
            ListElement{
                cap: "..."
            }*/
        }
        delegate: Label{
            text: cap
            color: "white"
        }

        ScrollBar.vertical: ScrollBar{

        }

        Component.onCompleted: {
            Pipeline2.add(function(aInput){
                if (aInput["log"]){
                    model.clear()
                    for (var i in aInput["log"])
                        model.append({cap: aInput["log"][i]})
                    positionViewAtEnd()  //https://forum.qt.io/topic/45016/solved-set-the-currentindex-of-listview-immediately-without-animation/2
                    //currentIndex = count - 1
                }else if (aInput["log_new"]){
                    model.append({cap: aInput["log_new"]})
                    positionViewAtEnd()
                    //currentIndex = count - 1
                }
            }, {name: "updateTaskJobLog"})
        }
    }
}
