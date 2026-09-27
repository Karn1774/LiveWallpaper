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
    property int pendingIndex: -1
    property bool transitionPending: false

    function advanceToNextVideo() {
        if (playlist.length < 2 || transitionPending) {
            return;
        }

        let nextIndex = (currentIndex + 1) % playlist.length;
        if (root.configuration.PlaybackOrder === 1) {
            const offset = 1 + Math.floor(Math.random() * (playlist.length - 1));
            nextIndex = (currentIndex + offset) % playlist.length;
        }

        pendingIndex = nextIndex;
        transitionPending = true;
        fadeOut.restart();
    }

    function commitPendingVideo() {
        if (pendingIndex >= 0) {
            root.configuration.CurrentIndex = pendingIndex;
            pendingIndex = -1;
        }
    }

    function finishVideoTransition() {
        if (!transitionPending) {
            return;
        }
        transitionPending = false;
        pendingIndex = -1;
        fadeIn.restart();
        refreshRotationTimer();
    }

    function refreshRotationTimer() {
        rotationTimer.stop();
        const intervalSeconds = Number(root.configuration.IntervalSeconds) || 300;
        if (root.configuration.AutoAdvance
                && !root.configuration.Paused
                && playlist.length > 1) {
            rotationTimer.interval = Math.max(60, intervalSeconds) * 1000;
            rotationTimer.start();
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
            opacity: 1
        }
    }

    NumberAnimation {
        id: fadeOut
        target: videoOutput
        property: "opacity"
        to: 0
        duration: 350
        easing.type: Easing.InOutQuad
        onFinished: root.commitPendingVideo()
    }

    NumberAnimation {
        id: fadeIn
        target: videoOutput
        property: "opacity"
        to: 1
        duration: 450
        easing.type: Easing.InOutQuad
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
        id: rotationTimer
        repeat: true
        onTriggered: root.advanceToNextVideo()
    }

    Timer {
        id: transitionTimeout
        interval: 8000
        running: root.transitionPending
        onTriggered: root.finishVideoTransition()
    }

    Connections {
        target: videoSource

        function onPlaybackUrlChanged() {
            root.syncPlayback();
        }
    }

    Connections {
        target: root.configuration

        function onValueChanged(key, value) {
            if (key === "Paused") {
                root.syncPlayback();
            }
            if (key === "Paused"
                    || key === "AutoAdvance"
                    || key === "IntervalSeconds"
                    || key === "VideoPlaylist"
                    || key === "PlaybackOrder") {
                root.refreshRotationTimer();
            }
        }
    }

    Connections {
        target: player

        function onMediaStatusChanged(status) {
            if (root.transitionPending && status === MediaPlayer.LoadedMedia) {
                root.finishVideoTransition();
            }
        }

        function onErrorOccurred(error, errorString) {
            if (root.transitionPending) {
                root.finishVideoTransition();
            }
        }
    }

    Component.onCompleted: {
        root.syncPlayback();
        root.refreshRotationTimer();
    }
}