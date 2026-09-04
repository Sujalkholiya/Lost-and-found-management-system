#ifndef IMAGELOADER_H
#define IMAGELOADER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QPixmap>
#include <QHash>
#include <QLabel>
#include <QPointer>
#include <QUrl>

class ImageLoader : public QObject {
    Q_OBJECT

public:
    static ImageLoader& instance() {
        static ImageLoader s_instance;
        return s_instance;
    }

    void loadIntoLabel(const QString& urlStr, QLabel* targetLabel, const QSize& targetSize = QSize(0, 0)) {
        if (!targetLabel || urlStr.isEmpty()) return;

        // Check in-memory cache first
        if (m_cache.contains(urlStr)) {
            QPixmap pix = m_cache.value(urlStr);
            if (!targetSize.isEmpty()) {
                targetLabel->setPixmap(pix.scaled(targetSize, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
            } else {
                targetLabel->setPixmap(pix);
            }
            return;
        }

        QPointer<QLabel> safeLabel(targetLabel);
        QUrl url(urlStr);
        QNetworkRequest request(url);
        QNetworkReply* reply = m_manager.get(request);

        connect(reply, &QNetworkReply::finished, this, [this, reply, safeLabel, urlStr, targetSize]() {
            reply->deleteLater();
            if (reply->error() == QNetworkReply::NoError) {
                QByteArray data = reply->readAll();
                QPixmap pixmap;
                if (pixmap.loadFromData(data)) {
                    m_cache.insert(urlStr, pixmap);
                    if (safeLabel) {
                        if (!targetSize.isEmpty()) {
                            safeLabel->setPixmap(pixmap.scaled(targetSize, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
                        } else {
                            safeLabel->setPixmap(pixmap);
                        }
                    }
                }
            }
        });
    }

    void clearCache() {
        m_cache.clear();
    }

private:
    ImageLoader(QObject* parent = nullptr) : QObject(parent) {}
    ~ImageLoader() override = default;
    ImageLoader(const ImageLoader&) = delete;
    ImageLoader& operator=(const ImageLoader&) = delete;

    QNetworkAccessManager m_manager;
    QHash<QString, QPixmap> m_cache;
};

#endif // IMAGELOADER_H
