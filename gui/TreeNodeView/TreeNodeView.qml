import QtQuick 2.12
import QtQuick.Controls 2.5
import Pipeline2 1.0

ScrollView{
    id: scrrange
    width: parent.width
    height: parent.height - 35
    contentHeight: 0
    contentWidth: parent.width
    clip: true

    TreeNode{
        id: root
        property var modifiedCache
        caption: qsTr("root")
        width: parent.width
        height: 500
        spacing: 5
        topPadding: spacing
        leftPadding: 10
        scr_root: scrrange
        treelayer: 0

        Component.onCompleted: {
            var sample = {
                            "hello": "world",
                            "he": [true, false],
                            "hi": {
                                "hi1": "hi2",
                                "ge": {
                                    "too": 3,
                                    "heww": {
                                        "ll": [3, 3, 4],
                                        "dd": "dddd",
                                        "ff": false
                                    }
                                }
                            },
                            "hi20": true,
                            "hello2": "world",
                            "he2": [0, {"kao": "gege"}, 1],
                            "hi2": [
                                {"hello": "world"},
                                {"no": [true, false]}
                            ],
                            "hi22": 3
                        }
            //buildGUI(mytree, mytree.sample)
            Pipeline2.add(function(aInput){
                modifiedCache = {}
                clearChildren()
                contentHeight = 0
                buildGUI(aInput["data"], aInput["style"])
                return {data: aInput, out: {}}
            }, {name: "loadTreeView", type: "Partial"})

            Pipeline2.add(function(aInput){
                var kys = aInput["keys"]
                if (kys[0] === caption){
                    kys.splice(0, 1)
                    modifyGUI(kys, aInput["opt"], aInput["type"], aInput["value"], aInput["style"])
                }
            }, {name: "treeViewModelModified"})

            Pipeline2.add(function(aInput){
                if (!modifiedCache)
                    modifiedCache = {}
                modifiedCache[aInput["keys"]] = aInput["value"]
            }, {name: "treeViewGUIModified"})

            Pipeline2.add(function(aInput){
                return {data: root.buildObject(), out: {}}
            }, {name: "saveTreeView", type: "Partial"})
        }
    }
}
