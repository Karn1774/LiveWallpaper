import QtQuick
import QtMultimedia
import org.kde.plasma.plasmoid
import com.custom.livewallpaper

WallpaperItem {
    id: root

    readonly property var playlist: root.configuration.VideoPlaylist || []
    readonly property int currentIndex: playlist.length > 0
        ? Math.max(0, Math.min(root.configuration.CurrentIndex || 0, playlist.length - 1))
        : 0
    readonly property string currentSource: playlist.length > 0
        ? playlist[currentIndex]
        : root.configuration.VideoSource

    function advanceToNextVideo() {
        if (playlist.length > 1) {
            root.configuration.CurrentIndex = (currentIndex + 1) % playlist.length;
        }
    }

    function syncPlayback() {
        if (!videoSource.available) {
            player.stop();
        } else if (root.configuration.Paused) {
            player.pause();
        } else {
            player.play();
        }
    }

    Rectangle {
        anchors.fill: parent
        color: "black"

        VideoOutput {
            id: videoOutput
            anchors.fill: parent
            fillMode: VideoOutput.PreserveAspectCrop
        }
    }

    VideoSource {
        id: videoSource
        source: root.currentSource
    }

    MediaPlayer {
        id: player
        source: videoSource.playbackUrl
        videoOutput: videoOutput
        loops: MediaPlayer.Infinite
        audioOutput: AudioOutput {
            muted: true
            volume: 0
        }
    }

    Timer {
        interval: Math.max(60, root.configuration.IntervalSeconds || 300) * 1000
        repeat: true
        running: root.configuration.AutoAdvance
            && !root.configuration.Paused
            && root.playlist.length > 1
        onTriggered: root.advanceToNextVideo()
    }

    Connections {
        target: videoSource

        function onPlaybackUrlChanged() {
            root.syncPlayback();
        }
    }

    Connections {
        target: root.configuration

        function onValueChanged(key) {
            if (key === "Paused") {
                root.syncPlayback();
            }
        }
    }

    Component.onCompleted: {
        root.syncPlayback();
    }
}