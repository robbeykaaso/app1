import QtQuick 2.12
import Pipeline2 1.0

Grid{
    property int viewcount: 1
    columns: 1
    rows: 1
    id: root

    Rectangle{
        width: parent.width / parent.columns
        height: parent.height / parent.rows
        color: "transparent"
        border.color: "black"
    }

    function updateRowsAndColumns(){
        var cnt = Math.ceil((Math.sqrt(viewcount)))
        columns = cnt
        rows = Math.ceil(viewcount / cnt)
        // console.log(viewcount + ";" + columns + ";" + rows)
    }

    function addView(aIndex){
        var src = "import QtQuick 2.12;"
        src += "AttachViewBoard{"
        src += "idx: " + aIndex + ";"
        src += "}"
        Qt.createQmlObject(src, root)
        //UIManager.setCommand({signal2: "showSiblings", type: "nullptr", param: {board: brd}}, null)
        return "panel" + aIndex;
    }

    function deleteView(aIndex){
        children[aIndex].destroy()
    }

    Component.onCompleted: {
        UIManager.registerPipe("setDisplayMode", "mdyGUI", function(aInput){
            var ret = []
            var sum = aInput["sum"]
            if (sum > 0){
                var del = sum - viewcount
                var i
                if (del > 0){
                    viewcount += del
                    updateRowsAndColumns()
                    for (i = 0; i < del; ++i){
                        ret.push(addView(viewcount - del + i))
                    }
                }else if (del < 0){
                    del = - del
                    viewcount -= del
                    for (i = 0; i < del; ++i)
                        deleteView(viewcount + del - i - 1)
                    updateRowsAndColumns()
                }
            }
            return {newViews: ret}
        })

        UIManager.registerPipe("addView", "mdyGUI", function(aInput){
            var sum = aInput["sum"]
            var del = sum - viewcount
            var ret = []
            if (del > 0){
                viewcount += del
                updateRowsAndColumns()
                for (var i = 0; i < del; ++i)
                    ret.push(addView(viewcount - del + i))
            }
            return {newViews: ret}
        })
        UIManager.registerPipe("deleteView", "mdyGUI", function(aInput){
            var del = aInput["del"]
            if (viewcount - del <= 1)
                del = viewcount - 1
            viewcount -= del
            for (var i = 0; i < del; ++i)
                deleteView(viewcount + del - i - 1)
            updateRowsAndColumns()
        })
    }
}
