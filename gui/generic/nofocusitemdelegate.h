#ifndef FUZZYDROPLETS_GUI_GENERIC_NOFOCUSITEMDELEGATE_H
#define FUZZYDROPLETS_GUI_GENERIC_NOFOCUSITEMDELEGATE_H

#include <QStyledItemDelegate>

// This provides a no-focus-box delegate for item widgets, for example tables with row selection or column selection where single cells should not be shown to have focus
class NoFocusDelegate : public QStyledItemDelegate
{
protected:

    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
};

#endif // FUZZYDROPLETS_GUI_GENERIC_NOFOCUSITEMDELEGATE_H
