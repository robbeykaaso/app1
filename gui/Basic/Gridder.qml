import QtQuick 2.12
import Pipeline2 1.0

Grid{
    property string name
    property var size: 1
    property Component com: Component{
            Rectangle{
                width: parent.width / parent.columns
                height: parent.height / parent.rows
                color: "transparent"
                border.color: "black"
            }
        }

    property int viewcount: 0
    columns: 1
    rows: 1
    spacing: - 1
    id: root

    function updateRowsAndColumns(){
        if (typeof size === "object"){
            rows = size[0]
            columns = size[1]
        }else{
            var cnt = Math.ceil((Math.sqrt(viewcount)))
            columns = cnt
            rows = Math.ceil(viewcount / cnt)
        }
        // console.log(viewcount + ";" + columns + ";" + rows)
    }

    function addView(aIndex){
        com.createObject(root, {name: name + "_gridder" + aIndex})
    }

    function deleteView(aIndex){
        children[aIndex].destroy()
    }

    function updateViewCount(aSum){
        size = aSum
        var sum
        if (typeof aSum === "object")
            sum = aSum[0] * aSum[1]
        else
            sum = aSum
        if (sum >= 0){
            var del = sum - viewcount
            var i
            if (del > 0){
                viewcount += del
                updateRowsAndColumns()
                for (i = 0; i < del; ++i){
                    addView(viewcount - del + i)
                }
            }else if (del < 0){
                del = - del
                viewcount -= del
                for (i = 0; i < del; ++i)
                    deleteView(viewcount + del - i - 1)
                updateRowsAndColumns()
            }
        }
    }

    Component.onCompleted: {
        Pipeline2.add(function(aInput){
            updateViewCount(aInput["size"])
            return {out: {}}
        }, {name: name + "_updateViewCount"})
        updateViewCount(size)

        /*UIManager.registerPipe("addView", "mdyGUI", function(aInput){
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
        })*/
    }
}
