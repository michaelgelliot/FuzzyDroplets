#include "notification.h"
#include <QGraphicsEffect>
#include <QLabel>
#include <QPropertyAnimation>
#include <QEvent>
#include <QBoxLayout>
#include <QPainterPath>

Notification::Notification(QWidget *parent)
    : QWidget{parent}
{
    setAttribute( Qt::WA_TransparentForMouseEvents);
    label = new QLabel(this);
    auto font = label->font();
    font.setBold(true);
    label->setFont(font);
    QHBoxLayout * layout = new QHBoxLayout(this);
    setLayout(layout);
    layout->addWidget(label);
    setStyleSheet("border-radius: 10px; border: 0px; background-color:rgba(0,0,0,125); color:rgb(245,245,245)"); // todo deal with qstylehints
    label->setMargin(12);
    label->setAlignment(Qt::AlignCenter);
    label->setTextFormat(Qt::TextFormat::RichText);

    setGraphicsEffect(new QGraphicsOpacityEffect(this));

    m_fadeInAnimation = new QPropertyAnimation(graphicsEffect(), "opacity", this);
    m_fadeInAnimation->setEasingCurve(QEasingCurve::InQuad);
    m_fadeInAnimation->setDuration(m_fadeDuration);
    connect(m_fadeInAnimation,SIGNAL(finished()),this,SLOT(fadedIn()));

    m_fadeOutAnimation = new QPropertyAnimation(graphicsEffect(), "opacity", this);
    m_fadeOutAnimation->setEasingCurve(QEasingCurve::OutQuad);
    m_fadeOutAnimation->setDuration(m_fadeDuration);
    connect(m_fadeOutAnimation,SIGNAL(finished()),this,SLOT(fadedOut()));

    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &Notification::timeout);

    hide();
}

void Notification::setFadeDuration(int duration)
{
    m_fadeDuration = duration;
    m_fadeInAnimation->setDuration(m_fadeDuration);
    m_fadeOutAnimation->setDuration(m_fadeDuration);
}

void Notification::fadeIn(const QString & msg)
{
    label->setText(msg);
    adjustSize();
    setLabelPosition();

    if (m_fadeInAnimation->state() != QPropertyAnimation::Running) {
        if (m_fadeOutAnimation->state() == QPropertyAnimation::Running) {
            m_fadeOutAnimation->stop();
            m_fadeInAnimation->setStartValue(m_fadeOutAnimation->currentValue());
            m_fadeInAnimation->setEndValue(1);
            m_fadeInAnimation->start();
        } else if (m_timer->isActive()) {
            m_timer->start(2000);
        } else {
            m_fadeInAnimation->setStartValue(0);
            m_fadeInAnimation->setEndValue(1);
            m_fadeInAnimation->start();
            show();
        }
    }
}

void Notification::fadeOut()
{
    if (isVisible() && m_fadeOutAnimation->state() != QPropertyAnimation::Running) {
        m_fadeInAnimation->stop();
        timeout();
    }
}

void Notification::timeout()
{
    m_timer->stop();
    m_fadeOutAnimation->setStartValue(1);
    m_fadeOutAnimation->setEndValue(0);
    m_fadeOutAnimation->start();
}

void Notification::fadedIn()
{
    if (m_autoFadeout)
        m_timer->start(m_autoFadeoutPauseTime);
}

void Notification::fadedOut()
{
    hide();
}

void Notification::setLabelPosition()
{
    double offset = 20;
    QRect R = parentWidget()->rect();
    R = QRect(QPoint(R.left() + m_margins.left(), R.top() + m_margins.top()), QPoint(R.right() - m_margins.right(), R.bottom() - m_margins.bottom()));
    QPoint p = R.center() - rect().center();
    if (m_alignment == Qt::AlignTop)
        p.setY(R.top() + offset);
    else if (m_alignment == Qt::AlignBottom)
        p.setY(R.bottom() - height() - offset);
    move(p);
    raise();
}
