#include "ProfileImageManager.h"
#include "AvatarHelper.h"
#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QSettings>
#include <QDebug>
#include <algorithm>

ProfileImageManager::ProfileImageManager(QObject *parent)
    : QObject(parent)
{
}

ProfileImageManager& ProfileImageManager::instance()
{
    static ProfileImageManager s_instance;
    return s_instance;
}

QString ProfileImageManager::getStorageDirectory() const
{
    QString basePath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    if (basePath.isEmpty()) {
        basePath = QDir::currentPath() + "/app_data";
    }
    QDir dir(basePath + "/avatars");
    if (!dir.exists()) {
        dir.mkpath(".");
    }
    return dir.absolutePath();
}

QString ProfileImageManager::getAvatarFilePath(int userId) const
{
    return getStorageDirectory() + QString("/avatar_user_%1.png").arg(userId);
}

bool ProfileImageManager::hasCustomAvatar(int userId) const
{
    if (userId <= 0) return false;
    QString path = getAvatarFilePath(userId);
    return QFile::exists(path);
}

bool ProfileImageManager::saveProfileImage(int userId, const QString &sourcePath, QString &errorMessage)
{
    if (userId <= 0) {
        errorMessage = "Invalid user account.";
        return false;
    }

    if (!QFile::exists(sourcePath)) {
        errorMessage = "The selected image file does not exist.";
        return false;
    }

    QFileInfo fi(sourcePath);
    if (fi.size() <= 0) {
        errorMessage = "The selected image file is empty.";
        return false;
    }

    if (fi.size() > 25 * 1024 * 1024) {
        errorMessage = "Image file is too large (maximum 25MB allowed).";
        return false;
    }

    QImage originalImage(sourcePath);
    if (originalImage.isNull() || originalImage.width() <= 0 || originalImage.height() <= 0) {
        errorMessage = "Unable to load this image. Please select a valid PNG, JPG, JPEG, or WEBP image.";
        return false;
    }

    // Center-square crop to preserve aspect ratio without facial distortion
    int w = originalImage.width();
    int h = originalImage.height();
    int side = std::min(w, h);
    int x = (w - side) / 2;
    int y = (h - side) / 2;

    QImage squareCropped = originalImage.copy(x, y, side, side);

    // Standardized avatar resolution: 256x256
    QImage resizedAvatar = squareCropped.scaled(256, 256, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

    QString destPath = getAvatarFilePath(userId);
    if (!resizedAvatar.save(destPath, "PNG")) {
        errorMessage = "Failed to save profile avatar to local storage.";
        return false;
    }

    // Persist in QSettings as well for redundancy
    QSettings settings("CollegeLostAndFound", "LostAndFoundGUI");
    settings.setValue(QString("avatar_user_%1").arg(userId), destPath);
    settings.sync();

    qDebug() << "[ProfileImageManager] Saved profile avatar for user" << userId << "at" << destPath;

    emit profileImageChanged(userId);
    return true;
}

bool ProfileImageManager::removeProfileImage(int userId)
{
    if (userId <= 0) return false;

    QString destPath = getAvatarFilePath(userId);
    if (QFile::exists(destPath)) {
        QFile::remove(destPath);
    }

    QSettings settings("CollegeLostAndFound", "LostAndFoundGUI");
    settings.remove(QString("avatar_user_%1").arg(userId));
    settings.sync();

    qDebug() << "[ProfileImageManager] Removed profile avatar for user" << userId;

    emit profileImageChanged(userId);
    return true;
}

QPixmap ProfileImageManager::getCircularAvatar(int userId, const QString &displayName, int size, const QString &fallbackBg) const
{
    if (userId > 0 && hasCustomAvatar(userId)) {
        QString path = getAvatarFilePath(userId);
        QPixmap customPix(path);
        if (!customPix.isNull()) {
            return AvatarHelper::createCircularAvatar(customPix, size);
        }
    }

    return AvatarHelper::createInitialsAvatar(displayName, size, fallbackBg);
}

