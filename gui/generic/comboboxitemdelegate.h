#ifndef FUZZYDROPLETS_GUI_GENERIC_COMBOBOXITEMDELEGATE_H
#define FUZZYDROPLETS_GUI_GENERIC_COMBOBOXITEMDELEGATE_H

#include <QStyledItemDelegate>
#include <QComboBox>

// This provides the drop-down combobox used to edit data type in column 1
class ComboBoxItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:

    ComboBoxItemDelegate(const QStringList & items, bool showFocusBox, QObject *parent = nullptr);

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;

    int minWidth() const {return m_minWidth;}

protected:

    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;

private:

    int m_minWidth;
    QStringList m_items;
    bool m_showFocusBox;
};

#endif // FUZZYDROPLETS_GUI_GENERIC_COMBOBOXITEMDELEGATE_H
