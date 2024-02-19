#ifndef FUZZYDROPLETS_GUI_GENERIC_DOUBLESLIDERACTION_H
#define FUZZYDROPLETS_GUI_GENERIC_DOUBLESLIDERACTION_H

#include <QWidgetAction>
#include <QHBoxLayout>
#include <QSlider>

class DoubleSliderAction : public QWidgetAction {

    Q_OBJECT

public:

    DoubleSliderAction (double min, double max, int factor = 1000, QObject * parent = nullptr);

    double value() const;

public slots:

    void setValue(double);

private slots:

    void sliderValueChanged(int);

signals:

    void valueChanged(double);

private:
    QSlider * m_slider;
    int m_factor {1000};
};

#endif // FUZZYDROPLETS_GUI_GENERIC_DOUBLESLIDERACTION_H
