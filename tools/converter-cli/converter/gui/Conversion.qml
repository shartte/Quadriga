
import QtQuick 2.0

Item {
    width: 640
    height: 480

    Text {
        id: headline
        color: "#ffffff"
        text: "Asset Conversion"
        font.capitalization: Font.SmallCaps
        font.pointSize: 14
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.margins: 20
    }

    Text {
        id: status
        color: "#ffffff"
        anchors.left: parent.left
        anchors.top: headline.bottom
        anchors.leftMargin: 20
        text: ""
    }

    ProgressBar {
        id: progressBar
        progress: 0.5
        anchors.left: parent.left
        anchors.right: parent.right
        height: 32
        anchors.margins: 20
        anchors.top: status.bottom
    }

    Rectangle {
        id: separator
        anchors.top: progressBar.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.margins: 20
        color: "#333333"
        height: 1
    }

    Text {
        id: logWindow
        anchors.top: separator.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.margins: 20
        color: "#dddddd"
        text: "Status Information"
    }

    Component.onCompleted: {
        if (typeof eventbus !== "undefined") {
            eventbus.subscribe("log", function (channel, message) {
                logWindow.text += message.message;
                console.log(message);
            });
            eventbus.subscribe("progress", function (channel, message) {
                progressBar.progress = message.progress;
            });
        }
    }

}