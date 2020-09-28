import QtQuick 2.12
import "../../Basic"
import Pipeline2 1.0

Rectangle{
    visible: false
    width: 120
    height: 240
    color: "lightskyblue"
    anchors.left: parent.left
    anchors.top: parent.top

    function updateROIGeometry(){
        Pipeline2.run("updateROIGeometry", [parseFloat(roi_x.input.text),
                                            parseFloat(roi_y.input.text),
                                            parseFloat(roi_x.input.text) + parseFloat(roi_w.input.text),
                                            parseFloat(roi_y.input.text) + parseFloat(roi_h.input.text)])
    }

    Column{
        anchors.fill: parent
        topPadding: 15
        Spin{
            width: 80
            caption.text: qsTr("count") + ":"
            ratio: 0.4
            anchors.horizontalCenter: parent.horizontalCenter
            spin.onValueChanged: {
                if (spin.value < 1)
                    spin.value = 1
                else{
                    Pipeline2.run("updateROICount", parseInt(spin.value))
                }
            }
        }
        Edit{
            id: roi_x
            width: 80
            caption.text: qsTr("x") + ":"
            ratio: 0.4
            anchors.horizontalCenter: parent.horizontalCenter
            input.onAccepted: updateROIGeometry()
        }
        Edit{
            id: roi_y
            width: 80
            caption.text: qsTr("y") + ":"
            ratio: 0.4
            anchors.horizontalCenter: parent.horizontalCenter
            input.onAccepted: updateROIGeometry()
        }
        Edit{
            id: roi_w
            width: 80
            caption.text: qsTr("width") + ":"
            ratio: 0.4
            anchors.horizontalCenter: parent.horizontalCenter
            input.onAccepted: updateROIGeometry()
        }
        Edit{
            id: roi_h
            width: 80
            caption.text: qsTr("height") + ":"
            ratio: 0.4
            anchors.horizontalCenter: parent.horizontalCenter
            input.onAccepted: updateROIGeometry()
        }
        Check{
            id: roi_r
            width: 80
            caption.text: qsTr("ratio") + ":"
            ratio: 0.4
            check.checked: true
            anchors.horizontalCenter: parent.horizontalCenter
            check.onCheckedChanged: Pipeline2.run("updateROIRatioMode", check.checked)
        }
        Check{
            id: roi_l
            width: 80
            caption.text: qsTr("local") + ":"
            ratio: 0.4
            anchors.horizontalCenter: parent.horizontalCenter
        }
        AutoSize{

        }
        Component.onCompleted: {
            Pipeline2.add(function(aInput){
                if (aInput["visible"]){
                    parent.visible = true
                    if (aInput["count"])
                        children[0].spin.value = aInput["count"]
                    if (aInput["x"] !== undefined)
                        roi_x.input.text = Math.round(aInput["x"] * 100) / 100
                    if (aInput["y"] !== undefined)
                        roi_y.input.text = Math.round(aInput["y"] * 100) / 100
                    if (aInput["w"] !== undefined)
                        roi_w.input.text = Math.round(aInput["w"] * 100) / 100
                    if (aInput["h"] !== undefined)
                        roi_h.input.text = Math.round(aInput["h"] * 100) / 100
                    if (aInput["r"] !== undefined)
                        roi_r.check.checked = aInput["r"]
                    if (aInput["l"] !== undefined)
                        roi_l.check.checked = aInput["l"]
                }else
                    parent.visible = false
            }, {name: "updateROIGUI"})
        }
    }
}
