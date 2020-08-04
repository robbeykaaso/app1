import QtQuick 2.12
import QtQuick.Controls 2.5
import "../Basic"
import UIManager 1.0

Button0{
    property string comment
    text.text: "?"
    width: 12
    height: width
    text.color: UIManager.fontColor
    text.font.pixelSize: UIManager.fontDespSize
    radius: 6

    onClicked: {
        detail.open()
    }
    Popup{
        id: detail
        Text{
            text: comment
            color: UIManager.fontColor
            font.pixelSize: UIManager.fontTitleSize
        }
    }
}
