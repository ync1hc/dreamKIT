import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: settings_page
    width: Screen.width
    height: Screen.height
    color: "#0F0F0F"

    Loader {
        anchors.fill: parent
        source: "../uss/uss.qml"
    }
}
