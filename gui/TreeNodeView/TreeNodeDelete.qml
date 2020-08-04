import QtQuick 2.12
import QtQuick.Controls 2.5

Button{
    text: "x"
    width: 12
    height: width
    font.pixelSize: 12

    function enumChildHeight(aNode){
        var ret = 0
        if (aNode instanceof TreeNode){
            ret += 12
            for (var i = 0; i < aNode.nodechild.children.length; ++i)
                if (!aNode.nodechild.deleted[aNode.nodechild.children[i]])
                    ret += enumChildHeight(aNode.nodechild.children[i].children[1])
        }else
            ret += 25
        return ret
    }

    function deleteTreeNode(aNotNotify){
        parent.parent.deleted[parent] = true
        if (!aNotNotify)
            UIManager.setCommand({signal2: 'treeViewGUIModified', type: 'nullptr', param: {keys: parent.parent.parent.extractKeyChain(), key: parent.children[1].key, opt: "del"}}, null)
        parent.parent.parent.scr_root.contentHeight -= enumChildHeight(parent.children[1])
        parent.destroy()
    }

    onClicked: {
        deleteTreeNode()
    }
}
