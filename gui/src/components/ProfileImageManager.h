#ifndef PROFILEIMAGEMANAGER_H
#define PROFILEIMAGEMANAGER_H

#include <QObject>
#include <QString>
#include <QPixmap>
#include <QImage>

class ProfileImageManager : public QObject
{
    Q_OBJECT

public:
    static ProfileImageManager& instance();

    bool hasCustomAvatar(int userId) const;
    QString getAvatarFilePath(int userId) const;

    bool saveProfileImage(int userId, const QString &sourcePath, QString &errorMessage);
    bool removeProfileImage(int userId);

    QPixmap getCircularAvatar(int userId, const QString &displayName, int size, const QString &fallbackBg = "#2563EB") const;

signals:
    void profileImageChanged(int userId);

private:
    explicit ProfileImageManager(QObject *parent = nullptr);
    ~ProfileImageManager() override = default;
    ProfileImageManager(const ProfileImageManager&) = delete;
    ProfileImageManager& operator=(const ProfileImageManager&) = delete;

    QString getStorageDirectory() const;
};

#endif // PROFILEIMAGEMANAGER_H

