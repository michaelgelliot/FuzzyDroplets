#ifndef FUZZYDROPLETS_GUI_GENERIC_THEMEDICON_H
#define FUZZYDROPLETS_GUI_GENERIC_THEMEDICON_H

#include <QIcon>

QPixmap themedPixmap(const QString & path);

// return a dark or light icon depending on the current theme
QIcon themedIcon(const QString & path);

#endif // FUZZYDROPLETS_GUI_GENERIC_THEMEDICON_H
