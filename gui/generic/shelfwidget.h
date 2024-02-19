#ifndef FUZZYDROPLETS_GUI_GENERIC_SHELFWIDGET_H
#define FUZZYDROPLETS_GUI_GENERIC_SHELFWIDGET_H

#include <QLabel>

class QScrollArea;
class CollapsibleContainer;

class ShelfWidget : public QWidget
{
    Q_OBJECT

public:

    ShelfWidget(QWidget * parent = nullptr);

    void addWidget(QWidget * w, const QString & title);
    CollapsibleContainer * collapsibleContainer(int index);

public slots:

    void guiColorSchemeChanged();

private:

    QWidget * m_container;
    QScrollArea * sa;
};

class CollapsibleContainerTitle : public QLabel
{
    Q_OBJECT

public:

    CollapsibleContainerTitle(const QString & title, QWidget * parent = nullptr);

    void showExpanded();
    void showCollapsed();

    void mousePressEvent(QMouseEvent * e);

signals:

    void setCollapsed(bool);

private:

    QString m_title;
    bool m_collapsed;

};

class CollapsibleContainer : public QWidget
{
    Q_OBJECT

public:

    CollapsibleContainer(const QString & title, QWidget * widget, QWidget * parent = nullptr);

    void showCollapsed();
    void showExpanded();

protected slots:

    void setCollapsed(bool);
    void guiColorSchemeChanged();

private:

    QWidget * m_widget;
    CollapsibleContainerTitle * titleLabel;
};

#endif // FUZZYDROPLETS_GUI_GENERIC_SHELFWIDGET_H
