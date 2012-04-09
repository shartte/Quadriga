
import QtQuick 2.0

Rectangle {
    id: root
    width: 100
    height: 30

    property alias text : label.text
    signal clicked();

    border.color: "#5c5c5c"
    border.width: 2
    color: Qt.darker("#1ac3ff")

    states: [
        State {
            name: "mouseOver"
            when: mouseArea.containsMouse && !mouseArea.pressed
            PropertyChanges {
                target: root
                color: "#1ac3ff"
            }
        },
        State {
            name: "mouseDown"
            when: mouseArea.containsMouse && mouseArea.pressed
            PropertyChanges {
                target: root
                color: Qt.lighter("#1ac3ff")
            }
        }
    ]

    transitions: [
        Transition {
            from: "*"
            to: "*"
            animations: [
                ColorAnimation { duration: 100 }
            ]
        }
    ]

    Text {
        id: label
        anchors.centerIn: parent
        font.bold: true
        color: "#ffffff"
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        onClicked: root.clicked();
        hoverEnabled: true
    }
}
