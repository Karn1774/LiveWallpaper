#include "wallpaperplugin.h"

#include <QFileInfo>
#include <QUrl>
#include <qqml.h>

VideoSource::VideoSource(QObject *parent)
    : QObject(parent)
{
}

QString VideoSource::source() const
{
    return m_source;
}

void VideoSource::setSource(const QString &source)
{
    if (m_source == source) {
        return;
    }

    m_source = source;
    m_playbackUrl.clear();

    if (!source.isEmpty()) {
        QUrl candidate(source);
        if (candidate.scheme().isEmpty()) {
            candidate = QUrl::fromLocalFile(QFileInfo(source).absoluteFilePath());
        }

        if (candidate.isLocalFile() && QFileInfo(candidate.toLocalFile()).isFile()) {
            m_playbackUrl = candidate;
        }
    }

    const bool isAvailable = !m_playbackUrl.isEmpty();
    if (m_available != isAvailable) {
        m_available = isAvailable;
        emit availableChanged();
    }

    emit sourceChanged();
    emit playbackUrlChanged();
}

QUrl VideoSource::playbackUrl() const
{
    return m_playbackUrl;
}

bool VideoSource::available() const
{
    return m_available;
}

void WallpaperPlugin::registerTypes(const char *uri)
{
    qmlRegisterType<VideoSource>(uri, 1, 0, "VideoSource");
}