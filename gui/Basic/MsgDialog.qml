import QtQuick 2.12
import QtQuick.Dialogs 1.2
import Pipeline2 1.0

MessageDialog {
    title: "Hello"
    text: "World"

    Component.onCompleted: {
        Pipeline2.add(function(aInput){
            title = aInput["title"]
            text = aInput["text"]
            open()
            return {out: {}}
        }, {name: "popMessage"})
    }
}
