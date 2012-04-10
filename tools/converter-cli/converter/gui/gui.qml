
import QtQuick 2.0

Rectangle {
    id: guiRoot
    color: "black"
    width: 640
    height: 480

    property variant pathChooser : pathChooser

    PathChooser {
        id: pathChooser
        anchors.fill: parent
        opacity: 0
    }

    Conversion {
        id: conversion
        anchors.fill: parent
        opacity: 0
    }

    states: [
        State {
            name: "choosePath"
            PropertyChanges {
                target: pathChooser
                opacity: 1
            }
        },
        State {
            name: "conversion"
            PropertyChanges {
                target: conversion
                opacity: 1
            }
        }
    ]

    transitions: [
        Transition {
            from: "*"
            to: "*"
            animations: [
                NumberAnimation { targets: [pathChooser, conversion]; properties: "opacity"; duration: 200 }
            ]
        }
    ]

    function setState(state) {
        guiRoot.state = state;
    }


    /*
    function addGuiItem(qmlUrl, creationCallback, properties) {

        if (!properties)
            properties = {};

        var component = Qt.createComponent(qmlUrl);

        var finishCreation = function() {
            var item = component.createObject(guiRoot, properties);
            if (!item) {
                console.log("FAILED: " + component.errorString());
            } else {
                console.log("Creation from " + qmlUrl + " succeeded.");
            }
            creationCallback(item);
        };

        switch (component.status) {
        case Component.Ready:
            finishCreation();
            break;
        case Component.Error:
            console.log("Loading component from " + qmlUrl + " failed: " + component.errorString());
            break;
        case Component.Loading:
            component.onCompleted = function() {
                if (component.status === Component.Error) {
                    console.log("Loading component from " + qmlUrl + " failed: " + component.errorString());
                } else {
                    finishCreation();
                }
            };
            break;
        }
    }
    */
}
