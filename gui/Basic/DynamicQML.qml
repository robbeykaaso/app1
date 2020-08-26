import QtQuick 2.12
import Pipeline2 1.0

Item{
    property string name
    signal loaded(var aItem)

    Component{
        id: loader
        Loader{
            Component.onDestruction: {
                console.log("hi2")
            }
        }
    }

    Component.onCompleted: {
        Pipeline2.find("loadDynamicQMLs").next(function(aInput){
            for (var i in aInput){
                var ld = loader.createObject(parent)
                ld.loaded.connect(
                    function(aLoader){
                        return function(){
                            loaded(aLoader.item)
                        }
                    }(ld)
                )
                ld.source = aInput[i]
            }
        }, {tag: name}, {vtype: []})
        Pipeline2.run("loadDynamicQMLs", name, {tag: name})
    }
}
