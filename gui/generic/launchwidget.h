#ifndef FUZZYDROPLETS_GUI_GENERIC_LAUNCHWIDGET_H
#define FUZZYDROPLETS_GUI_GENERIC_LAUNCHWIDGET_H

#include <QWidget>

class QProgressBar;
class QPushButton;

class LaunchWidget : public QWidget
{
    Q_OBJECT

public:

    LaunchWidget(QWidget *parent = nullptr);

    void setProperties(bool cancelEnabled, bool progressBarEnabled);

    QProgressBar * progressBar() {return m_progressBar;}
    QPushButton * runButton() {return m_runButton;}
    QPushButton * cancelButton() {return m_cancelButton;}

public slots:

    void setReadyMode();
    void setRunningMode();
    void guiColorSchemeChanged();

private:

    bool m_cancelEnabled {true};
    bool m_progressBarEnabled {true};
    bool m_readyMode {true};

    QProgressBar * m_progressBar;
    QPushButton * m_runButton;
    QPushButton * m_cancelButton;
};

#endif // FUZZYDROPLETS_GUI_GENERIC_LAUNCHWIDGET_H
