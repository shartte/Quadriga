
import QtQuick 2.0

Rectangle {
    id: root
    width: 100
    height: 32
    color: "#1c324e"

    property double progress : 0
    border.color: "#5c5c5c"
    border.width: 2

    Rectangle {
        id: innerRect
        color: "#1ac3ff"
        width: progress * (root.width - 4)
        anchors.top: root.top
        anchors.bottom: root.bottom
        anchors.left: root.left
        anchors.margins: 2
    }

}
