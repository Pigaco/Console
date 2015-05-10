import QtQuick 2.0
import QtQuick.Layouts 1.0

Rectangle {
    id: root
    width: 800
    height: 900

    Text {
        id: header
        text: qsTr("Settings")
        anchors.left: parent.left
        anchors.leftMargin: 20
        anchors.top: parent.top
        anchors.topMargin: 20
        font.bold: true
        font.pixelSize: 50
    }

    Text {
        id: backButton
        x: 358
        y: -4
        width: 122
        height: 58
        text: qsTr("Back")
        anchors.right: parent.right
        anchors.rightMargin: 20
        anchors.top: parent.top
        font.bold: false
        anchors.topMargin: 20
        font.pixelSize: 50

        focus: true

        styleColor: "#779bd6"
        style: activeFocus ? Text.Outline : Text.Normal
        Keys.onReturnPressed: root.parent.source = ""

        KeyNavigation.down: playerSettings
    }

    ListModel {
        id: settingsModel
        ListElement {
            settingsElement: Players {
                id: playerSettings
                height: 324
                width: parent.width
            }
        }
    }

    ListView {
        anchors.left: parent.left
        anchors.leftMargin: 0
        anchors.right: parent.right
        anchors.rightMargin: 0
        anchors.top: header.bottom
        anchors.topMargin: 10
        anchors.bottom: parent.bottom

        KeyNavigation.up: backButton
        model: settingsModel

        delegate: Component {

        }
    }

}

