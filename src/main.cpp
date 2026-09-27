#include <QApplication>
#include <QAudioOutput>
#include <QCheckBox>
#include <QComboBox>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusMessage>
#include <QFileDialog>
#include <QFileInfo>
#include <QFormLayout>
#include <QGuiApplication>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QMainWindow>
#include <QMediaPlayer>
#include <QPushButton>
#include <QSettings>
#include <QSlider>
#include <QSpinBox>
#include <QStandardPaths>
#include <QStyle>
#include <QUrl>
#include <QVariantMap>
#include <QVideoWidget>
#include <QVBoxLayout>
#include <QWidget>

class WallpaperWindow final : public QMainWindow
{
public:
    WallpaperWindow()
        : m_player(new QMediaPlayer(this))
        , m_audioOutput(new QAudioOutput(this))
        , m_videoWidget(new QVideoWidget)
        , m_seekSlider(new QSlider(Qt::Horizontal))
        , m_playButton(new QPushButton(tr("Pause preview")))
        , m_pauseWallpaperButton(new QPushButton)
        , m_nextButton(new QPushButton(tr("Next wallpaper")))
        , m_applyButton(new QPushButton(tr("Apply to all screens")))
        , m_statusLabel(new QLabel(tr("Import videos to create a wallpaper playlist.")))
        , m_playlistWidget(new QListWidget)
        , m_intervalSpin(new QSpinBox)
        , m_intervalUnit(new QComboBox)
        , m_autoAdvanceCheck(new QCheckBox(tr("Change wallpaper automatically")))
    {
        setWindowTitle(tr("Performance Live Wallpaper"));
        resize(820, 820);
        setMinimumSize(640, 620);

        auto *content = new QWidget(this);
        auto *layout = new QVBoxLayout(content);
        layout->setContentsMargins(20, 20, 20, 20);
        layout->setSpacing(12);

        auto *title = new QLabel(tr("Performance Live Wallpaper"), content);
        QFont titleFont = title->font();
        titleFont.setPointSize(titleFont.pointSize() + 3);
        titleFont.setBold(true);
        title->setFont(titleFont);

        auto *description = new QLabel(tr("Preview a video, then set it as your Plasma desktop background."), content);
        description->setWordWrap(true);

        m_videoWidget->setMinimumSize(420, 230);
        m_videoWidget->setAspectRatioMode(Qt::KeepAspectRatioByExpanding);

        m_seekSlider->setRange(0, 0);
        m_playButton->setEnabled(false);
        m_pauseWallpaperButton->setEnabled(false);
        m_nextButton->setEnabled(false);
        m_applyButton->setEnabled(false);
        m_applyButton->setDefault(true);

        m_playlistWidget->setSelectionMode(QAbstractItemView::SingleSelection);
        m_playlistWidget->setMinimumHeight(110);

        auto *libraryBox = new QGroupBox(tr("Video library"), content);
        auto *libraryLayout = new QHBoxLayout(libraryBox);
        auto *libraryButtons = new QVBoxLayout;
        auto *importButton = new QPushButton(tr("Import videos..."), libraryBox);
        importButton->setIcon(style()->standardIcon(QStyle::SP_DialogOpenButton));
        m_removeButton = new QPushButton(tr("Remove selected"), libraryBox);
        m_removeButton->setEnabled(false);
        libraryButtons->addWidget(importButton);
        libraryButtons->addWidget(m_removeButton);
        libraryButtons->addStretch(1);
        libraryLayout->addWidget(m_playlistWidget, 1);
        libraryLayout->addLayout(libraryButtons);

        auto *scheduleBox = new QGroupBox(tr("Automatic wallpaper changes"), content);
        auto *scheduleLayout = new QFormLayout(scheduleBox);
        m_intervalSpin->setRange(1, 1440);
        m_intervalSpin->setSuffix(tr(""));
        m_intervalUnit->addItem(tr("minutes"), 60);
        m_intervalUnit->addItem(tr("hours"), 3600);
        auto *intervalRow = new QWidget(scheduleBox);
        auto *intervalLayout = new QHBoxLayout(intervalRow);
        intervalLayout->setContentsMargins(0, 0, 0, 0);
        intervalLayout->addWidget(m_intervalSpin);
        intervalLayout->addWidget(m_intervalUnit);
        intervalLayout->addStretch(1);
        scheduleLayout->addRow(m_autoAdvanceCheck);
        scheduleLayout->addRow(tr("Change every:"), intervalRow);

        auto *actionRow = new QHBoxLayout;
        actionRow->addWidget(m_playButton);
        actionRow->addStretch(1);
        actionRow->addWidget(m_pauseWallpaperButton);
        actionRow->addWidget(m_nextButton);
        actionRow->addWidget(m_applyButton);

        m_statusLabel->setWordWrap(true);

        layout->addWidget(title);
        layout->addWidget(description);
        layout->addWidget(m_videoWidget, 1);
        layout->addWidget(m_seekSlider);
        layout->addWidget(libraryBox);
        layout->addWidget(scheduleBox);
        layout->addLayout(actionRow);
        layout->addWidget(m_statusLabel);
        setCentralWidget(content);

        m_player->setAudioOutput(m_audioOutput);
        m_audioOutput->setMuted(true);
        m_player->setVideoOutput(m_videoWidget);

        connect(importButton, &QPushButton::clicked, this, [this] {
            importVideos();
        });
        connect(m_removeButton, &QPushButton::clicked, this, [this] {
            removeSelectedVideo();
        });
        connect(m_playButton, &QPushButton::clicked, this, [this] {
            togglePreview();
        });
        connect(m_pauseWallpaperButton, &QPushButton::clicked, this, [this] {
            m_paused = !m_paused;
            saveSettings();
            updateButtons();
            applyWallpaper();
        });
        connect(m_nextButton, &QPushButton::clicked, this, [this] {
            goToNextVideo();
        });
        connect(m_applyButton, &QPushButton::clicked, this, [this] {
            saveSettings();
            applyWallpaper();
        });
        connect(m_playlistWidget, &QListWidget::currentRowChanged, this, [this](int row) {
            if (row >= 0 && row < m_playlist.size()) {
                m_currentIndex = row;
                saveSettings();
                loadPreview();
            }
            updateButtons();
        });
        connect(m_intervalUnit, &QComboBox::currentIndexChanged, this, [this](int index) {
            m_intervalSpin->setMaximum(index == 1 ? 24 : 1440);
        });
        connect(m_player, &QMediaPlayer::durationChanged, this, [this](qint64 duration) {
            m_seekSlider->setRange(0, static_cast<int>(duration));
        });
        connect(m_player, &QMediaPlayer::positionChanged, this, [this](qint64 position) {
            if (!m_seekSlider->isSliderDown()) {
                m_seekSlider->setValue(static_cast<int>(position));
            }
        });
        connect(m_seekSlider, &QSlider::sliderMoved, this, [this](int position) {
            m_player->setPosition(position);
        });
        connect(m_player, &QMediaPlayer::playbackStateChanged, this, [this](QMediaPlayer::PlaybackState state) {
            m_playButton->setText(state == QMediaPlayer::PlayingState ? tr("Pause preview") : tr("Play preview"));
        });
        connect(m_player, &QMediaPlayer::errorOccurred, this, [this](QMediaPlayer::Error, const QString &message) {
            m_statusLabel->setText(tr("Playback error: %1").arg(message));
        });

        restoreSettings();
    }

private:
    QString currentVideoPath() const
    {
        if (m_currentIndex < 0 || m_currentIndex >= m_playlist.size()) {
            return {};
        }
        return m_playlist.at(m_currentIndex);
    }

    void importVideos()
    {
        const QStringList paths = QFileDialog::getOpenFileNames(
            this,
            tr("Import videos into the wallpaper library"),
            QStandardPaths::writableLocation(QStandardPaths::MoviesLocation),
            tr("Video files (*.mp4 *.webm *.mkv *.mov);;All files (*)"));
        if (paths.isEmpty()) {
            return;
        }

        const int oldSize = m_playlist.size();
        for (const QString &path : paths) {
            const QFileInfo fileInfo(path);
            if (!fileInfo.isFile() || !fileInfo.isReadable()) {
                continue;
            }
            const QString canonicalPath = fileInfo.canonicalFilePath();
            const QString storedPath = canonicalPath.isEmpty() ? fileInfo.absoluteFilePath() : canonicalPath;
            if (!m_playlist.contains(storedPath)) {
                m_playlist.append(storedPath);
            }
        }

        if (m_playlist.isEmpty()) {
            m_statusLabel->setText(tr("None of the selected files could be imported."));
            return;
        }

        populatePlaylist();
        if (oldSize == 0) {
            m_currentIndex = 0;
            m_playlistWidget->setCurrentRow(m_currentIndex);
            loadPreview();
        }
        saveSettings();
        updateButtons();
        m_statusLabel->setText(tr("Imported %1 video(s). Apply the playlist to start using it as wallpaper.")
                                   .arg(m_playlist.size() - oldSize));
    }

    void removeSelectedVideo()
    {
        const int row = m_playlistWidget->currentRow();
        if (row < 0 || row >= m_playlist.size()) {
            return;
        }

        m_playlist.removeAt(row);
        if (m_playlist.isEmpty()) {
            m_currentIndex = 0;
            m_player->stop();
            m_player->setSource(QUrl());
            m_seekSlider->setRange(0, 0);
            m_statusLabel->setText(tr("The video library is empty."));
        } else {
            m_currentIndex = qMin(row, m_playlist.size() - 1);
        }
        populatePlaylist();
        if (!m_playlist.isEmpty()) {
            m_playlistWidget->setCurrentRow(m_currentIndex);
            loadPreview();
        }
        saveSettings();
        updateButtons();
    }

    void populatePlaylist()
    {
        m_playlistWidget->blockSignals(true);
        m_playlistWidget->clear();
        for (const QString &path : std::as_const(m_playlist)) {
            const QFileInfo fileInfo(path);
            auto *item = new QListWidgetItem(fileInfo.fileName(), m_playlistWidget);
            item->setToolTip(path);
            if (!fileInfo.isReadable()) {
                item->setForeground(palette().color(QPalette::Disabled, QPalette::Text));
                item->setText(item->text() + tr(" (unavailable)"));
            }
        }
        if (!m_playlist.isEmpty()) {
            m_currentIndex = qBound(0, m_currentIndex, m_playlist.size() - 1);
            m_playlistWidget->setCurrentRow(m_currentIndex);
        }
        m_playlistWidget->blockSignals(false);
    }

    void restoreSettings()
    {
        QSettings settings;
        m_playlist = settings.value(QStringLiteral("playlist")).toStringList();
        m_currentIndex = settings.value(QStringLiteral("currentIndex"), 0).toInt();
        m_paused = settings.value(QStringLiteral("paused"), false).toBool();
        m_autoAdvanceCheck->setChecked(settings.value(QStringLiteral("autoAdvance"), false).toBool());
        m_intervalUnit->setCurrentIndex(settings.value(QStringLiteral("intervalUnit"), 0).toInt());
        m_intervalSpin->setMaximum(m_intervalUnit->currentIndex() == 1 ? 24 : 1440);
        m_intervalSpin->setValue(settings.value(QStringLiteral("intervalAmount"), 5).toInt());
        populatePlaylist();
        if (!m_playlist.isEmpty()) {
            loadPreview();
        }
        updateButtons();
    }

    void saveSettings()
    {
        QSettings settings;
        settings.setValue(QStringLiteral("playlist"), m_playlist);
        settings.setValue(QStringLiteral("currentIndex"), m_currentIndex);
        settings.setValue(QStringLiteral("paused"), m_paused);
        settings.setValue(QStringLiteral("autoAdvance"), m_autoAdvanceCheck->isChecked());
        settings.setValue(QStringLiteral("intervalUnit"), m_intervalUnit->currentIndex());
        settings.setValue(QStringLiteral("intervalAmount"), m_intervalSpin->value());
    }

    void loadPreview()
    {
        const QString path = currentVideoPath();
        if (path.isEmpty()) {
            return;
        }
        const QFileInfo fileInfo(path);
        if (!fileInfo.isReadable()) {
            m_statusLabel->setText(tr("Video is missing or unreadable: %1").arg(path));
            return;
        }
        m_player->setSource(QUrl::fromLocalFile(path));
        m_player->setLoops(QMediaPlayer::Infinite);
        m_player->play();
        m_statusLabel->setText(tr("Previewing %1").arg(fileInfo.fileName()));
    }

    void togglePreview()
    {
        if (m_player->playbackState() == QMediaPlayer::PlayingState) {
            m_player->pause();
        } else {
            m_player->play();
        }
    }

    void updateButtons()
    {
        const bool hasVideos = !m_playlist.isEmpty();
        const bool hasSelection = m_playlistWidget->currentRow() >= 0;
        m_playButton->setEnabled(hasSelection);
        m_pauseWallpaperButton->setEnabled(hasVideos);
        m_pauseWallpaperButton->setText(m_paused ? tr("Resume wallpaper") : tr("Pause wallpaper"));
        m_nextButton->setEnabled(m_playlist.size() > 1);
        m_applyButton->setEnabled(hasVideos);
        m_removeButton->setEnabled(hasSelection);
    }

    int intervalSeconds() const
    {
        return m_intervalSpin->value() * m_intervalUnit->currentData().toInt();
    }

    QStringList playlistUrls() const
    {
        QStringList urls;
        urls.reserve(m_playlist.size());
        for (const QString &path : m_playlist) {
            urls.append(QUrl::fromLocalFile(path).toString(QUrl::FullyEncoded));
        }
        return urls;
    }

    void goToNextVideo()
    {
        if (m_playlist.size() < 2) {
            return;
        }
        m_currentIndex = (m_currentIndex + 1) % m_playlist.size();
        m_playlistWidget->setCurrentRow(m_currentIndex);
        loadPreview();
        saveSettings();
        applyWallpaper();
    }

    void applyWallpaper()
    {
        if (m_playlist.isEmpty() || currentVideoPath().isEmpty()) {
            m_statusLabel->setText(tr("Import at least one video first."));
            return;
        }
        for (const QString &path : m_playlist) {
            if (!QFileInfo(path).isReadable()) {
                m_statusLabel->setText(tr("A playlist video is missing or unreadable: %1").arg(path));
                return;
            }
        }

        QDBusConnection bus = QDBusConnection::sessionBus();
        if (!bus.isConnected()) {
            m_statusLabel->setText(tr("Could not connect to the session D-Bus."));
            return;
        }

        QDBusInterface shell(QStringLiteral("org.kde.plasmashell"),
                             QStringLiteral("/PlasmaShell"),
                             QStringLiteral("org.kde.PlasmaShell"),
                             bus);
        if (!shell.isValid()) {
            m_statusLabel->setText(tr("Could not connect to Plasma Shell. Is this a Plasma session?"));
            return;
        }

        bool countIsValid = false;
        uint screenCount = shell.property("numScreens").toUInt(&countIsValid);
        if (!countIsValid || screenCount == 0) {
            const auto screens = QGuiApplication::screens();
            screenCount = static_cast<uint>(screens.isEmpty() ? 1 : screens.size());
        }

        const QString currentUrl = QUrl::fromLocalFile(currentVideoPath()).toString(QUrl::FullyEncoded);
        const QVariantMap parameters{
            {QStringLiteral("VideoSource"), currentUrl},
            {QStringLiteral("VideoPlaylist"), playlistUrls()},
            {QStringLiteral("CurrentIndex"), m_currentIndex},
            {QStringLiteral("AutoAdvance"), m_autoAdvanceCheck->isChecked()},
            {QStringLiteral("IntervalSeconds"), intervalSeconds()},
            {QStringLiteral("Paused"), m_paused},
        };

        for (uint screen = 0; screen < screenCount; ++screen) {
            const QDBusMessage reply = shell.call(QStringLiteral("setWallpaper"),
                                                  QStringLiteral("com.custom.livewallpaper"),
                                                  parameters,
                                                  screen);
            if (reply.type() == QDBusMessage::ErrorMessage) {
                m_statusLabel->setText(tr("Could not set the wallpaper on screen %1: %2")
                                           .arg(screen + 1)
                                           .arg(reply.errorMessage()));
                return;
            }
        }

        m_player->pause();
        m_statusLabel->setText(tr("Playlist applied to all screens. Automatic changes run in Plasma after this window closes."));
    }

    QMediaPlayer *m_player;
    QAudioOutput *m_audioOutput;
    QVideoWidget *m_videoWidget;
    QSlider *m_seekSlider;
    QPushButton *m_playButton;
    QPushButton *m_pauseWallpaperButton;
    QPushButton *m_nextButton;
    QPushButton *m_applyButton;
    QLabel *m_statusLabel;
    QListWidget *m_playlistWidget;
    QPushButton *m_removeButton = nullptr;
    QSpinBox *m_intervalSpin;
    QComboBox *m_intervalUnit;
    QCheckBox *m_autoAdvanceCheck;
    QStringList m_playlist;
    int m_currentIndex = 0;
    bool m_paused = false;
};

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QApplication::setApplicationName(QStringLiteral("livewallpaper"));
    QApplication::setApplicationDisplayName(QObject::tr("Performance Live Wallpaper"));
    QApplication::setOrganizationName(QStringLiteral("com.custom"));

    WallpaperWindow window;
    window.show();
    return app.exec();
}