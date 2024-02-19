#include "launchwidget.h"
#include "themedicon.h"
#include <QHBoxLayout>
#include <QProgressBar>
#include <QPushButton>
#include <QGuiApplication>
#include <QStyleHints>

LaunchWidget::LaunchWidget(QWidget *parent)
    : QWidget{parent}
{
    m_progressBar = new QProgressBar;
    m_progressBar->setRange(0, 100);
    m_cancelButton = new QPushButton(themedIcon(":/exit"), "");
    m_runButton = new QPushButton(themedIcon(":/play"), "");
//    m_progressBar->setTextVisible(true);
//    m_progressBar->setFormat("%p\%");
//    m_progressBar->setAlignment(Qt::AlignCenter);

    m_runButton->setToolTip("Run");
    m_cancelButton->setToolTip("Cancel");

    QHBoxLayout * layout = new QHBoxLayout;
    layout->setContentsMargins(0,0,0,0);

    layout->addWidget(m_progressBar);

    QHBoxLayout * buttonLayout = new QHBoxLayout;
    buttonLayout->setContentsMargins(0,0,0,0);
    buttonLayout->setSpacing(0);
    buttonLayout->addWidget(m_cancelButton);
    buttonLayout->addWidget(m_runButton);
    layout->addLayout(buttonLayout);

    layout->setStretch(0, 1);
    layout->setStretch(1, 0);

    setLayout(layout);

    QSizePolicy sp = m_progressBar->sizePolicy();
    sp.setRetainSizeWhenHidden(true);
    m_progressBar->setSizePolicy(sp);

    setReadyMode();

    connect(qGuiApp->styleHints(), &QStyleHints::colorSchemeChanged, this, &LaunchWidget::guiColorSchemeChanged, Qt::QueuedConnection);
}

void LaunchWidget::setProperties(bool cancelEnabled, bool progressBarEnabled)
{
    m_cancelEnabled = cancelEnabled;
    m_progressBarEnabled = progressBarEnabled;
    if (m_readyMode)
        setReadyMode();
    else
        setRunningMode();
}

void LaunchWidget::setReadyMode()
{
    m_readyMode = true;
    m_runButton->setEnabled(true);
    m_progressBar->hide();
    m_cancelButton->hide();
    m_runButton->setIcon(themedIcon(":/play"));
    m_runButton->show();

}

void LaunchWidget::setRunningMode()
{
    m_readyMode = false;
    m_runButton->setEnabled(false);
    m_runButton->setIcon(themedIcon(":/hourglass"));
    m_runButton->setHidden(m_cancelEnabled || m_progressBarEnabled);
    m_cancelButton->setVisible(m_cancelEnabled);
    m_progressBar->setVisible(m_progressBarEnabled);
}

void LaunchWidget::guiColorSchemeChanged()
{
    m_cancelButton->setIcon(themedIcon(":/exit"));
    m_runButton->setIcon(themedIcon(":/play"));
}
