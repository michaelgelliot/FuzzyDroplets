#ifndef FUZZYDROPLETS_GUI_GENERIC_STACKEDWIDGET_H
#define FUZZYDROPLETS_GUI_GENERIC_STACKEDWIDGET_H

#include <QWidget>
class QVBoxLayout;
class QComboBox;

class StackedWidget : public QWidget
{
    Q_OBJECT

public:

    explicit StackedWidget(QWidget *parent = nullptr);

    void addWidget(QWidget * w, QString label);
    QWidget * currentWidget() const {return m_activeWidget;}

signals:

    void widgetChanged(QWidget * oldWidget, QWidget * newWidget);

public slots:

    void setCurrentIndex(int);
    void enableComboBox();
    void disableComboBox();

private:

    QList<QWidget*> m_widgets;
    QVBoxLayout * m_layout;
    QWidget * m_activeWidget;
    QComboBox * m_comboBox;
};

#endif // FUZZYDROPLETS_GUI_GENERIC_STACKEDWIDGET_H
