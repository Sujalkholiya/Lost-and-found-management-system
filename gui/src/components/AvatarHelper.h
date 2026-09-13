#ifndef AVATARHELPER_H
#define AVATARHELPER_H

#include <QString>
#include <QStringList>
#include <QPixmap>
#include <QPainter>
#include <QFont>
#include <QColor>
#include <algorithm>

namespace AvatarHelper {

inline QString getInitials(const QString &name)
{
    QString trimmed = name.trimmed();
    if (trimmed.isEmpty()) return "U";
    QStringList parts = trimmed.split(' ', Qt::SkipEmptyParts);
    if (parts.size() >= 2) {
        return (parts[0].left(1) + parts[1].left(1)).toUpper();
    }
    return parts[0].left(1).toUpper();
}

inline QPixmap createInitialsAvatar(const QString &name, int size = 36, const QString &bgHex = "#4F46E5")
{
    QPixmap pixmap(size, size);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing, true);

    // Draw circular background
    painter.setBrush(QColor(bgHex));
    painter.setPen(Qt::NoPen);
    painter.drawEllipse(0, 0, size, size);

    // Draw initials text
    painter.setPen(QColor("#FFFFFF"));
    int fontSize = static_cast<int>(size * 0.38);
    if (fontSize < 9) fontSize = 9;
    QFont font("Segoe UI", fontSize, QFont::Bold);
    painter.setFont(font);
    painter.drawText(pixmap.rect(), Qt::AlignCenter, getInitials(name));

    return pixmap;
}

} // namespace AvatarHelper

#endif // AVATARHELPER_H

