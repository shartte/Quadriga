
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

    ListModel {
        id: logEntries
    }

    ListView {
        id: logWindow
        clip: true

        anchors.top: separator.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.margins: 20

        model: logEntries
        delegate: Text {
            text: getLevel(level) + message
            color: "#ffffff"
        }
    }

    function getLevel(level) {
        var result = "<b>[";
        switch (level) {
        case "info":
            result += "<font color=\"#ffffff\">INFO</font>";
            break;
        case "error":
            result += "<font color=\"#ff0000\">ERROR</font>";
            break;
        case "warn":
            result += "<font color=\"#cccc00\">WARN</font>";
            break;
        default:
            result += message.level.toUpperCase();
            break;
        }
        result += "]</b> ";
        return result;
    }

    Component.onCompleted: {
        console.log("Subscribing to event bus.");
        eventbus.subscribe("log", function (channel, message) {
            logEntries.append(message);
            logWindow.positionViewAtEnd();
        });
        eventbus.subscribe("progress", function (channel, message) {
            progressBar.progress = message.progress;
        });
        console.log("Subscribed to event bus.");
    }

}
