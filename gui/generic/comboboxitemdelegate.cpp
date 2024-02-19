#include "comboboxitemdelegate.h"

ComboBoxItemDelegate::ComboBoxItemDelegate(const QStringList & items, bool showFocusBox, QObject *parent)
    : QStyledItemDelegate(parent),
      m_items(items),
      m_showFocusBox(showFocusBox)
{
    QComboBox *cb = new QComboBox(nullptr);
    cb->addItems(m_items);
    m_minWidth = cb->minimumSizeHint().width();
    delete cb;
}

void ComboBoxItemDelegate::paint(QPainter* painter, const QStyleOptionViewItem & option, const QModelIndex &index) const
{
    QStyleOptionViewItem itemOption(option);
    if (!m_showFocusBox && (itemOption.state & QStyle::State_HasFocus))
        itemOption.state = itemOption.state ^ QStyle::State_HasFocus;
    QStyledItemDelegate::paint(painter, itemOption, index);
};

QWidget * ComboBoxItemDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QComboBox *cb = new QComboBox(parent);
    cb->addItems(m_items);
    for (int i = 0; i < cb->count(); ++i)
        cb->setItemData(i, Qt::AlignCenter, Qt::TextAlignmentRole); // todo: doesn't seem to  work on fusion style
    return cb;
}

void ComboBoxItemDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    QComboBox *cb = qobject_cast<QComboBox *>(editor);
    const QString currentText = index.data(Qt::EditRole).toString();
    const int cbIndex = cb->findText(currentText);
    if (cbIndex >= 0) cb->setCurrentIndex(cbIndex);
}

void ComboBoxItemDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    QComboBox *cb = qobject_cast<QComboBox *>(editor);
    model->setData(index, cb->currentText(), Qt::EditRole);
}

