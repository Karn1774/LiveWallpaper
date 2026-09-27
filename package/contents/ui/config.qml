import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Layouts

ColumnLayout {
    id: root

    property alias cfg_VideoSource: videoPath.text

    spacing: 8

    Label {
        text: qsTr("Video file")
        Layout.fillWidth: true
    }

    RowLayout {
        Layout.fillWidth: true

        TextField {
            id: videoPath
            Layout.fillWidth: true
            placeholderText: qsTr("Choose an MP4 or WebM file")
            selectByMouse: true
        }

        Button {
            text: qsTr("Browse...")
            icon.name: "document-open"
            onClicked: fileDialog.open()
        }
    }

    Label {
        text: qsTr("Video playback and hardware decoding depend on the installed Qt Multimedia backend and graphics drivers.")
        wrapMode: Text.WordWrap
        opacity: 0.75
        Layout.fillWidth: true
    }

    FileDialog {
        id: fileDialog
        title: qsTr("Select a wallpaper video")
        fileMode: FileDialog.OpenFile
        nameFilters: [
            qsTr("Video files (*.mp4 *.webm *.mkv *.mov)"),
            qsTr("All files (*)")
        ]

        onAccepted: videoPath.text = selectedFile.toString()
    }
}