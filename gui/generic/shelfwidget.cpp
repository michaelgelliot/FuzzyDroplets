#include "shelfwidget.h"
#include <QGuiApplication>
#include <QStyleHints>
#include <QScrollArea>
#include <QBoxLayout>
#include <QLabel>
#include <QTimer>
#include <QScrollBar>

ShelfWidget::ShelfWidget(QWidget * parent)
    : QWidget(parent)
{
    m_container = new QWidget;
    QVBoxLayout * vLayout = new QVBoxLayout;
    vLayout->setContentsMargins(0,0,0,0);
    vLayout->setSpacing(6);
    vLayout->addStretch(1);
    m_container->setLayout(vLayout);

    sa = new QScrollArea(this);
    sa->setFrameStyle(QFrame::NoFrame);
    sa->setWidget(m_container);
    sa->setWidgetResizable(true);
    sa->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    QHBoxLayout * layout = new QHBoxLayout;
    layout->setContentsMargins(0,0,0,0);
    layout->addWidget(sa);
    setLayout(layout);

    connect(qGuiApp->styleHints(), &QStyleHints::colorSchemeChanged, this, &ShelfWidget::guiColorSchemeChanged, Qt::QueuedConnection);

}

void ShelfWidget::addWidget(QWidget * w, const QString & title)
{
    CollapsibleContainer * c = new CollapsibleContainer(title, w);
    QVBoxLayout * L = (QVBoxLayout*)m_container->layout();
    L->insertWidget(L->count()-1, c);
    //setMinimumWidth(qMax(minimumWidth(), w->minimumWidth()));
    L->invalidate();
}

CollapsibleContainer * ShelfWidget::collapsibleContainer(int index)
{
    QVBoxLayout * L = (QVBoxLayout*)m_container->layout();
    return reinterpret_cast<CollapsibleContainer*>(L->itemAt(index)->widget());
}

CollapsibleContainerTitle::CollapsibleContainerTitle(const QString & title, QWidget * parent)
    : QLabel(parent),
    m_title(title)
{
    auto f = font();
    f.setBold(true);
    setFont(f);

    setContentsMargins(6,6,6,6);

    setAutoFillBackground(true);
    setBackgroundRole(QPalette::Midlight);

    setCursor(Qt::PointingHandCursor);

    showCollapsed();
}

void CollapsibleContainerTitle::showExpanded()
{
    setText(QChar(11206) + QString(" ") + m_title);
    m_collapsed = false;
    emit setCollapsed(false);
}

void CollapsibleContainerTitle::showCollapsed()
{
    setText(QChar(11208) + QString(" ") + m_title);
    m_collapsed = true;
    emit setCollapsed(true);
}

void CollapsibleContainerTitle::mousePressEvent(QMouseEvent * e)
{
    if (m_collapsed) showExpanded();
    else showCollapsed();
}

CollapsibleContainer::CollapsibleContainer(const QString & title, QWidget * widget, QWidget * parent)
    : QWidget(parent),
    m_widget(widget)
{
    QVBoxLayout * layout = new QVBoxLayout;
    titleLabel = new CollapsibleContainerTitle(title);

    layout->addWidget(titleLabel);
    layout->addWidget(widget);
    layout->setSpacing(0);
    layout->setContentsMargins(0,0,0,0);
    setLayout(layout);

    setMinimumWidth(qMax(1,widget->sizeHint().width()));

    connect(titleLabel, &CollapsibleContainerTitle::setCollapsed, this, &CollapsibleContainer::setCollapsed);
    connect(qGuiApp->styleHints(), &QStyleHints::colorSchemeChanged, this, &CollapsibleContainer::guiColorSchemeChanged, Qt::QueuedConnection);

    setCollapsed(true);
}

void CollapsibleContainer::setCollapsed(bool b)
{
    m_widget->setHidden(b);
}

void CollapsibleContainer::showCollapsed()
{
    titleLabel->showCollapsed();
}

void CollapsibleContainer::showExpanded()
{
    titleLabel->showExpanded();
}


void ShelfWidget::guiColorSchemeChanged()
{
    m_container->setPalette(qGuiApp->palette());
    sa->setPalette(qGuiApp->palette());
    sa->horizontalScrollBar()->setPalette(qGuiApp->palette());
    sa->verticalScrollBar()->setPalette(qGuiApp->palette());
    setPalette(qGuiApp->palette());
}


void CollapsibleContainer::guiColorSchemeChanged()
{
    m_widget->setPalette(qGuiApp->palette());
    titleLabel->setPalette(qGuiApp->palette());
    setPalette(qGuiApp->palette());
}
