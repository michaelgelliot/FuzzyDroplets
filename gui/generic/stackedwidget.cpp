#include "stackedwidget.h"
#include <QVBoxLayout>
#include <QComboBox>
#include <QPushButton>
#include <QProgressBar>

StackedWidget::StackedWidget(QWidget *parent)
    : QWidget{parent},
      m_layout(new QVBoxLayout),
      m_activeWidget(nullptr)
{
    m_layout->setContentsMargins(0,0,0,0);
    m_comboBox = new QComboBox;

    QVBoxLayout * outer = new QVBoxLayout;
    outer->setContentsMargins(0,0,0,0);
    outer->addWidget(m_comboBox);
    outer->addLayout(m_layout);
    setLayout(outer);
    connect(m_comboBox, &QComboBox::activated, this, &StackedWidget::setCurrentIndex);
}

void StackedWidget::addWidget(QWidget * w, QString label)
{
    m_widgets.append(w);
    m_comboBox->addItem(label);
    if (!m_activeWidget) {
        m_activeWidget = w;
        m_layout->addWidget(w);
        w->show();
    } else {
        w->hide();
    }
}

void StackedWidget::setCurrentIndex(int i)
{
    if (m_activeWidget) {
        auto old = m_activeWidget;
        m_activeWidget->hide();
        m_layout->removeWidget(m_activeWidget);
        m_layout->addWidget(m_widgets[i]);
        m_activeWidget = m_widgets[i];
        m_activeWidget->show();
        emit widgetChanged(old, m_activeWidget);
    }
}

void StackedWidget::enableComboBox()
{
    m_comboBox->setEnabled(true);
}

void StackedWidget::disableComboBox()
{
    m_comboBox->setEnabled(false);
}
