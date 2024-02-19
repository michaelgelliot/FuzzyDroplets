#ifndef FUZZYDROPLETS_GUI_GENERIC_NOTIFICATION_H
#define FUZZYDROPLETS_GUI_GENERIC_NOTIFICATION_H

#include <QLabel>
#include <QTimer>

class QGraphicsOpacityEffect;
class QPropertyAnimation;

class Notification : public QWidget
{
    Q_OBJECT

public:

    explicit Notification(QWidget *parent = nullptr);

    void fadeIn(const QString & msg);
    void fadeOut();
    void setLabelPosition();

    bool autoFadeout() const {return m_autoFadeout;}
    void setAutoFadeout(bool b) {m_autoFadeout = b;}

    int autoFadeoutPauseTime() const {return m_autoFadeoutPauseTime;}
    void setAutoFadeoutPauseTime(int i) {m_autoFadeoutPauseTime = i;}

    int fadeDuration() const {return m_fadeDuration;}
    void setFadeDuration(int duration);

    void setContentsMargins(const QMargins & margins) {m_margins = margins;}
    QMargins contentsMargins() const {return m_margins;}

    Qt::Alignment alignment() const {return m_alignment;}
    void setAlignment(Qt::Alignment alignment) {m_alignment = alignment;}

public slots:

    void timeout();
    void fadedIn();
    void fadedOut();

signals:

private:

    QLabel *label;
    QTimer * m_timer;
    QPropertyAnimation * m_fadeInAnimation;
    QPropertyAnimation * m_fadeOutAnimation;
    bool m_autoFadeout {true};
    int m_autoFadeoutPauseTime {2000};
    int m_fadeDuration {300};
    QMargins m_margins;
    Qt::Alignment m_alignment;

};

#endif // FUZZYDROPLETS_GUI_GENERIC_NOTIFICATION_H
