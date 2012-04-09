
import QtQuick 2.0

Item
{
    width: 640
    height: 480

    property alias path : path.text

    signal ok

    Text {
        id: headline
        color: "#ffffff"
        text: "Choose Installation Path"

        font.capitalization: Font.SmallCaps
        font.pointSize: 14

        anchors.margins: 20
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
    }

    Text {
        id: description
        anchors.margins: 20
        anchors.topMargin: 5
        anchors.top: headline.bottom
        anchors.left: parent.left
        anchors.right: parent.right

        color: "#ffffff"

        text: "Please select the directory where your current Temple of Elemental Evil installation resides."
    }

    Rectangle {
        id: pathBox
        border.color: "#5c5c5c"
        border.width: 2

        color: "#111111"

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.margins: 20
        anchors.top: description.bottom
        height: 20

        TextInput {
            x: 100
            id: path
            color: "#ffffff"
            width: 200
            anchors.fill: parent
            anchors.margins: 2
        }
    }

    Button {
        anchors.left: parent.left
        anchors.top: pathBox.bottom
        anchors.margins: 20
        text: "Ok"
        onClicked: ok()
    }

}
