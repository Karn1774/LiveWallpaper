#pragma once

#include <QObject>
#include <QQmlExtensionPlugin>
#include <QUrl>

class VideoSource : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString source READ source WRITE setSource NOTIFY sourceChanged)
    Q_PROPERTY(QUrl playbackUrl READ playbackUrl NOTIFY playbackUrlChanged)
    Q_PROPERTY(bool available READ available NOTIFY availableChanged)

public:
    explicit VideoSource(QObject *parent = nullptr);

    QString source() const;
    void setSource(const QString &source);

    QUrl playbackUrl() const;
    bool available() const;

signals:
    void sourceChanged();
    void playbackUrlChanged();
    void availableChanged();

private:
    QString m_source;
    QUrl m_playbackUrl;
    bool m_available = false;
};

class WallpaperPlugin final : public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QQmlExtensionInterface")

public:
    void registerTypes(const char *uri) override;
};