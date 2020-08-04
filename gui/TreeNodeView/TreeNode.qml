import QtQuick 2.12
import QtQuick.Window 2.12
import QtQuick.Controls 2.5
import ".."

Column{
    id: root
    property int treelayer
    property string key
    property string tag: "object"
    property string caption: " "
    property alias nodechild: cld
    property bool opt_add: true
    property var scr_root
    spacing: 5
    Row{
        //width: parent.width
        //height: 40
        spacing: 5
        Text{
            text: tag == "object" ? "{" + caption + "}" : "[" + caption + "]"
            MouseArea{
                anchors.fill: parent
                onClicked: {
                    cld.visible = !cld.visible
                }
            }
        }
        Button{
            visible: opt_add
            text: "+"
            width: 12
            height: width
            onClicked: {
                typeselect.show()
            }
        }
        TWindow{
            id: typeselect
            width: Screen.desktopAvailableWidth * 0.22
            height: Screen.desktopAvailableHeight * 0.22
            caption: qsTr("Select Type")

            ButtonGroup{
                id: btngrp
            }

            content:
                Column{
                    anchors.fill: parent
                    spacing: height * 0.05
                    topPadding: spacing

                    Repeater{
                        id: sel
                        model: ListModel{
                            ListElement {cap: "double/integer"; chk: false}
                            ListElement {cap: "string"; chk: true}
                            ListElement {cap: "boolean"; chk: false}
                            ListElement {cap: "array"; chk: false}
                            ListElement {cap: "object"; chk: false}
                        }
                        Row{
                            height: 20
                            width: parent.width
                            spacing: 10
                            leftPadding: spacing
                            RadioButton{
                                text: cap
                                height: parent.height
                                checked: chk
                                ButtonGroup.group: btngrp
                            }
                            TextEdit{
                                visible: root.tag == "object"
                                text: qsTr("key")
                            }
                            TextEdit{
                                //visible: root.tag ==
                                visible: cap == "double/integer" || cap == "string"
                                text: qsTr("value")
                            }
                            CheckBox{
                                visible: cap == "boolean"
                                text: qsTr("value")
                                height: 20
                            }
                        }
                    }

                    Row{
                        anchors.right: parent.right
                        anchors.rightMargin: 5
                        spacing: 10
                        ButtonDialog{
                            text.text: qsTr("Cancel")
                            onClicked: typeselect.close()
                        }
                        ButtonDialog{
                            text.text: qsTr("OK")
                            onClicked: {
                                var tp, ky
                                for (var i = 0; i < sel.count; ++i){
                                    if (sel.itemAt(i).children[0].checked){
                                        tp = sel.itemAt(i).children[0].text
                                        ky = sel.itemAt(i).children[1].input.text
                                        break
                                    }
                                }
                                if (tag == "array"){
                                    ky = cld.children.length - Object.keys(cld.deleted).length
                                    if (sel.itemAt(i).children[3].visible)
                                        addJsonChild(tp, ky, sel.itemAt(i).children[3].checked)
                                    else if (sel.itemAt(i).children[2].visible){
                                        if (tp !== "string")
                                            tp = "double"
                                        addJsonChild(tp, ky, sel.itemAt(i).children[2].input.text);
                                    }
                                    else
                                        addJsonChild(tp, ky)
                                }
                                else{
                                    for (var j = 0; j < cld.children.length; ++j)
                                        if (ky === cld.getChildValue(j)){
                                            UIManager.setCommand({signal2: "popupMessageDialog", type: "nullptr", param: {
                                                                         title: "warning",
                                                                         text: "Duplicated key!"
                                                                     }}, null)
                                            return
                                        }

                                    if (sel.itemAt(i).children[3].visible)
                                        addJsonChild(tp, ky, sel.itemAt(i).children[3].checked)
                                    else if (sel.itemAt(i).children[2].visible){
                                        if (tp !== "string")
                                            tp = "double"
                                        addJsonChild(tp, ky, sel.itemAt(i).children[2].input.text);
                                    }
                                    else
                                        addJsonChild(tp, ky)
                                }
                            }
                        }
                    }
                }
        }
    }
    Column{
        id: cld
        property var deleted
        width: parent.width
        //height: 55
        spacing: 5
        leftPadding: 10
        visible: treelayer == 0
        /*CustomEdit1{
            caption.text: qsTr("hello2")
            editbackground.width: 100
            editbackground.height: 25
        }
        CustomEdit1{
            caption.text: qsTr("hello3")
            editbackground.width: 100
            editbackground.height: 25
        }*/

        function getChildType(){
            if (children.length > 0)
                if (children[0].children[1] instanceof CheckBox1)
                    return "boolean"
                else
                    return children[0].children[1].tag
            else
                return ""
        }

        function getChildValue(aIndex){
            var chd = children[aIndex].children[1]
            if (chd instanceof Edit1)
                return chd.input.text
            else if (chd instanceof CheckBox1)
                return chd.text
            else
                return chd.caption
        }

        Component.onCompleted: {
            deleted = {}
        }
    }

    /*ComboBox1{
        onValueChanged: {
            value
        }
    }*/

    function extractKeyChain(){
        var ret = key
        if (parent instanceof Row && parent.parent instanceof Column && parent.parent.parent instanceof TreeNode)
            ret = parent.parent.parent.extractKeyChain() + ";" + ret
        return ret
    }

    function modifyGUI(aKeys, aOpt, aType, aValue, aStyle){
        var i
        if (aKeys.length === 1){
            if (aOpt === "add")
                addJsonChild(aType, aKeys[0], aValue, aStyle)
            else{
                for (i = 0; i < cld.children.length; ++i)
                    if (!cld.deleted[cld.children[i]]){
                        var istar = false
                        if (cld.children[i].children[1] instanceof CheckBox0){
                            istar = cld.children[i].children[1].key === aKeys[0]
                            if (aOpt !== "del")
                                cld.children[i].children[1].checked = aValue
                        }else if (cld.children[i].children[1] instanceof ComboBox0){
                            istar = cld.children[i].children[1].key === aKeys[0]
                            if (aOpt !== "del")
                                cld.children[i].children[1].setCurrent(aValue)
                        }else if (cld.children[i].children[1] instanceof Edit0){
                            istar = cld.children[i].children[1].key === aKeys[0]
                            if (aOpt !== "del")
                                cld.children[i].children[1].input.text = aValue
                        }else if (cld.children[i].children[1] instanceof TreeNode)
                            istar = cld.children[i].children[1].key === aKeys[0]
                        if (istar){
                            if (aOpt === "del")
                                cld.children[i].children[0].deleteTreeNode()
                            break
                        }
                    }

            }
        }else if (aKeys.length > 1){
            for (i = 0; i < cld.children.length; ++i)
                if (!cld.deleted[cld.children[i]] && cld.children[i].children[1] instanceof TreeNode && cld.children[i].children[1].caption === aKeys[0]){
                    var kys = aKeys
                    kys.splice(0, 1)
                    cld.children[i].children[1].modifyGUI(kys, aOpt, aType, aValue, aStyle)
                    break
                }
        }
    }

    function addJsonChild(aType, aKey, aValue, aStyle, aInitialize){
        var ret
        var src = "import QtQuick 2.12; import '../Basic/DeepSight'; import UIManager 1.0; import TextFieldDoubleValidator 1.0;"
        src += "Row{"
        if (aStyle && aStyle["jsst"] && aStyle["jsst"]["invisible"])
            src += "visible: false;"
        src += "spacing: 5;"
        src += "TreeNodeDelete{"
        if (aStyle && aStyle["jsst"] && aStyle["jsst"]["opt_del"]){
            src += "enabled: false;"
            src += "text.text: '';"
            src += "color: 'transparent';"
        }
        var cap = aKey
        if (aStyle && aStyle["jsst"] && aStyle["jsst"]["caption"] && aStyle["jsst"]["caption"] !== "")
            cap = aStyle["jsst"]["caption"]
        if (aType === "boolean"){
            src += "anchors.verticalCenter: parent ? parent.children[1].verticalCenter : undefined;}"
            src += "CheckBox1{"
            src += "leftPadding: 0;"
            //if (tag !== "array")
            src += "key: '" + aKey + "';"
            src += "text: '" + cap + "';"
            src += "height: 15;"
            src += "checked: " + aValue + ";"
            if (aStyle && aStyle["jsst"] && aStyle["jsst"]["val_script"])
                src += aStyle["jsst"]["val_script"]
            else
                src += "onCheckedChanged: {UIManager.setCommand({signal2: 'treeViewGUIModified', type: 'nullptr', param: {keys: parent.parent.parent.extractKeyChain() + ';' + key, value: checked}}, null); labelColor = 'red';}"
            src += "}"
            if (aStyle && aStyle["jsst"] && aStyle["jsst"]["comment"] && aStyle["jsst"]["comment"] !== ""){
                src += "TreeNodeTag{anchors.verticalCenter: parent ? parent.children[1].verticalCenter : undefined;"
                src += "comment: '" + aStyle["jsst"]["comment"] + "';"
                src += "}"
            }
            src += "}"
            ret = Qt.createQmlObject(src, cld)
            scr_root.contentHeight += 25
        }else if (aType === "string" || aType === "double"){
            src += "anchors.verticalCenter: parent ? parent.children[1].verticalCenter : undefined;}"
            if (aStyle && aStyle["jsst"] && aStyle["jsst"]["val_type"] === "combo"){
                src += "ComboBox1{"
                //if (tag !== "array")
                src += "property int mdytick: 0;"
                src += "key: '" + aKey + "';"
                src += "caption: '" + cap + "';"
                src += "cmb.width: 60;"
                //src += "cmb.height: 35;"
                var val = ""
                var cur = 0;
                for (var i in aStyle["jsst"]["val_value"]){
                    if (val != "")
                        val += ","
                    else
                        if (typeof aStyle["jsst"]["val_value"][i] != "string")
                            src += "tag: 'double';"
                    if (aStyle["jsst"]["val_value"][i] === aValue)
                        src += "cmb.currentIndex: " + cur + ";"
                    val += "'" + aStyle["jsst"]["val_value"][i] + "'"
                    cur++
                }
                src += "model: [" + val + "];"
                if (aStyle["jsst"]["val_script"]){
                    src += aStyle["jsst"]["val_script"]
                }
                else{
                    src += "onValueChanged: {"
                    src += "if (mdytick++){"
                    src += "UIManager.setCommand({signal2: 'treeViewGUIModified', type: 'nullptr', param: {keys: parent.parent.parent.extractKeyChain() + ';' + key, value: value}}, null);"
                    src += "cap.color = 'red';"
                    //src += "console.log('hi2');"
                    src += "}}"
                }
            }else{
                src += "Edit1{"
                src += "tag: '" + aType + "';"
                //if (tag !== "array")
                src += "key: '" + aKey + "';"
                src += "caption.text: '" + cap + "';"
                src += "editbackground.width: 60;"
                src += "editbackground.height: 25;"
                src += "input.text: '" + aValue + "';"
                if (aStyle && aStyle["jsst"] && aStyle["jsst"]["val_type"] === "regExp"){
                    if (aStyle["jsst"]["val_value"] !== "")
                        src += "input.validator: RegExpValidator{regExp: " + aStyle["jsst"]["val_value"] + ";}"
                }else if (aType === "double"){
                    if (aStyle && aStyle["jsst"] && aStyle["jsst"]["val_type"] === "range"){
                        src += "input.validator: TextFieldDoubleValidator{bottom:" + aStyle["jsst"]["val_value"][0] + "; top:" + aStyle["jsst"]["val_value"][1] + ";}"
                    }else
                        src += "input.validator: DoubleValidator{}"
                }
                if (aStyle && aStyle["jsst"] && aStyle["jsst"]["val_script"])
                    src += aStyle["jsst"]["val_script"]
                else{
                    //src += "Keys.onPressed: {"
                    //src += "if (event.key === 16777220)"
                    //if (aType === "string")
                    //    src += "UIManager.setCommand({signal2: 'treeViewGUIModified', type: 'nullptr', param: {keys: parent.parent.parent.extractKeyChain() + ';' + key, value: input.text}}, null);"
                    //else
                    //    src += "UIManager.setCommand({signal2: 'treeViewGUIModified', type: 'nullptr', param: {keys: parent.parent.parent.extractKeyChain() + ';' + key, value: parseFloat(input.text)}}, null);"
                    //src += "}"

                    if (aType === "string")
                        src += "input.onTextEdited: {UIManager.setCommand({signal2: 'treeViewGUIModified', type: 'nullptr', param: {keys: parent.parent.parent.extractKeyChain() + ';' + key, value: input.text}}, null); caption.color = 'red';}"
                    else
                        src += "input.onTextEdited: {UIManager.setCommand({signal2: 'treeViewGUIModified', type: 'nullptr', param: {keys: parent.parent.parent.extractKeyChain() + ';' + key, value: parseFloat(input.text)}}, null); caption.color = 'red';}"
                }
            }
            src += "}"
            if (aStyle && aStyle["jsst"] && aStyle["jsst"]["comment"] && aStyle["jsst"]["comment"] !== ""){
                src += "TreeNodeTag{anchors.verticalCenter: parent ? parent.children[1].verticalCenter : undefined;"
                src += "comment: '" + aStyle["jsst"]["comment"] + "';"
                src += "}"
            }
            src += "}"
            ret = Qt.createQmlObject(src, cld)
            scr_root.contentHeight += 25
        }else{
            src += "}"
            src += "TreeNode{"
            src += "tag: '" + aType + "';"
            //if (tag !== "array")
            src += "key: '" + aKey + "';"
            src += "caption: '" + cap + "';"
            if (aStyle && aStyle["jsst"] && aStyle["jsst"]["opt_add"])
                src += "opt_add: false;"
            src += "}"
            if (aStyle && aStyle["jsst"] && aStyle["jsst"]["comment"] && aStyle["jsst"]["comment"] !== ""){
                src += "TreeNodeTag{"
                src += "comment: '" + aStyle["jsst"]["comment"] + "';"
                src += "}"
            }
            src += "}"
            ret = Qt.createQmlObject(src, cld)
            ret.children[1].scr_root = scr_root
            ret.children[1].treelayer = treelayer + 1
            scr_root.contentHeight += 37
        }
        scr_root.contentWidth = Math.max(scr_root.contentWidth, (treelayer + 1) * 10 + treelayer * 17 + ret.children[1].x + ret.children[1].width + 30)
        if (!aInitialize)
            UIManager.setCommand({signal2: 'treeViewGUIModified', type: 'nullptr', param: {keys: extractKeyChain(), key: aKey, value: aValue, type: aType, opt: "add"}}, null)
//        console.log(ret.children[1].x + ";" + ret.children[1].width)
//        if (cld.children.length - Object.keys(cld.deleted).length > 1)
//            scr_root.contentHeight += 5
        return ret
    }

    function recoverDefaultUI(){
        for (var i = 0; i < cld.children.length; ++i)
            if (!cld.deleted[cld.children[i]]){
                var cld0 = cld.children[i].children[1]
                if (cld0 instanceof Edit1){
                    cld0.caption.color = UIManager.fontColor
                }else if (cld0 instanceof CheckBox1){
                    cld0.labelColor = UIManager.fontColor
                }else if (cld0 instanceof ComboBox1){
                    cld0.cap.color = UIManager.fontColor
                }else{
                    cld0.recoverDefaultUI()
                }
            }
    }

    function buildArray(){
        var ret = []
        for (var i = 0; i < cld.children.length; ++i)
            if (!cld.deleted[cld.children[i]]){
                var cld0 = cld.children[i].children[1]
                if (cld0 instanceof Edit1){
                    if (cld0.tag === "double")
                        ret.push(parseFloat(cld0.input.text))
                    else
                        ret.push(cld0.input.text)
                }else if (cld0 instanceof CheckBox1){
                    ret.push(cld0.checked)
                }else if (cld0 instanceof ComboBox1){
                    if (cld0.tag === "double")
                        ret.push(parseFloat(cld0.value))
                    else
                        ret.push(cld0.value)
                }else{
                    if (cld0.tag === "object")
                        ret.push(cld0.buildObject())
                    else
                        ret.push(cld0.buildArray())
                }
            }
        return ret
    }

    function buildObject(){
        var ret = {}
        for (var i = 0; i < cld.children.length; ++i)
            if (!cld.deleted[cld.children[i]]){
                var cld0 = cld.children[i].children[1]
                if (cld0 instanceof Edit1){
                    if (cld0.tag === "double")
                        ret[cld0.key] = parseFloat(cld0.input.text)
                    else
                        ret[cld0.key] = cld0.input.text
                }else if (cld0 instanceof CheckBox1){
                    ret[cld0.key] = cld0.checked
                }else if (cld0 instanceof ComboBox1){
                    if (cld0.tag === "double")
                        ret[cld0.key] = parseFloat(cld0.value)
                    else
                        ret[cld0.key] = cld0.value
                }else{
                    //console.log(cld0.caption + ";" + cld0.tag)
                    if (cld0.tag === "object")
                        ret[cld0.key] = cld0.buildObject()
                    else
                        ret[cld0.key] = cld0.buildArray()
                }
            }
        return ret
    }

    function doBuildGUI(aKey, aValue, aStyle){
        //console.log("hi")
        if (!(aValue instanceof Object)){
            var tp = typeof aValue
            if (tp == "boolean"){
                addJsonChild("boolean", aKey, aValue, aStyle ? aStyle[aKey] : undefined, true)
            }else{
                if (tp === "string")
                    addJsonChild("string", aKey, aValue, aStyle ? aStyle[aKey] : undefined, true)
                else
                    addJsonChild("double", aKey, aValue, aStyle ? aStyle[aKey] : undefined, true)
            }
        }else{
            var nd
            if (aValue instanceof Array)
                nd = addJsonChild("array", aKey, "", aStyle ? aStyle[aKey] : undefined, true)
            else
                nd = addJsonChild("object", aKey, "", aStyle ? aStyle[aKey] : undefined, true)
            nd.children[1].buildGUI(aValue, aStyle ? aStyle[aKey] : undefined)
        }
    }

    function buildGUI(aJson, aStyle){
        if (aJson["jsst"])
            doBuildGUI("jsst", aJson["jsst"], aStyle)
        for (var i in aJson)
            if (i !== "jsst")
                doBuildGUI(i, aJson[i], aStyle)
    }

    function clearChildren(){
        cld.deleted = {}
        for (var i = 0; i < cld.children.length; ++i)
            if (!cld.deleted[cld.children[i]])
                cld.children[i].children[0].deleteTreeNode(true)
    }
}
