#include "themedicon.h"
#include <QStyleHints>
#include <QIcon>
#include <QGuiApplication>
#include <QPainter>
#include <QPalette>

QPixmap themedPixmap(const QString & path)
{
    if (qGuiApp->styleHints()->colorScheme() == Qt::ColorScheme::Dark) {
        QPixmap pixmap(path);
        QPainter paint (&pixmap);
        paint.setCompositionMode(QPainter::CompositionMode_SourceIn);
        paint.fillRect(pixmap.rect(), QColor(qGuiApp->palette().color(QPalette::Text)));
        paint.end();
        return pixmap;
    } else {
        return QPixmap(path);
    }
}

QIcon themedIcon(const QString & path)
{
    // return QIcon(themedPixmap(path));
    if (qGuiApp->styleHints()->colorScheme() == Qt::ColorScheme::Light) {
        return QIcon(path);
    } else {
        return QIcon(path + "Inverted");
    }
}
