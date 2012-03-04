
import QtQuick 2.0

Rectangle {
    width: 100
    height: 69
    color: "#ffffff"
    opacity: 0.5

    Text {
        height: 500
        font.pointSize: 50

        horizontalAlignment:Text.AlignHCenter
        text: "Hello Ogre World. This is QML calling!"
        anchors.right: parent.right
        anchors.left: parent.left
    }

    TextEdit {
        id: text_edit1
        font.pixelSize: 12
        anchors.fill: parent
    }

    Component.onCompleted: {
        myObj.callMethod("Test text");
    }
}
