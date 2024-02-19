#include "doubleslideraction.h"

DoubleSliderAction::DoubleSliderAction(double min, double max, int factor, QObject * parent) :
    QWidgetAction(parent),
    m_factor(factor)
{
    QWidget* pWidget = new QWidget;
    QHBoxLayout* pLayout = new QHBoxLayout();
    m_slider = new QSlider(nullptr);
    pLayout->addWidget (m_slider);
    pWidget->setLayout (pLayout);
    m_slider->setOrientation(Qt::Horizontal);
    m_slider->setRange(min * m_factor, max * m_factor);
    connect(m_slider, &QSlider::valueChanged, this, &DoubleSliderAction::sliderValueChanged);
    setDefaultWidget(pWidget);
}

void DoubleSliderAction::sliderValueChanged(int val)
{
    emit valueChanged((double)val / m_factor);
}

double DoubleSliderAction::value() const
{
    return (double)m_slider->value() / m_factor;
}

void DoubleSliderAction::setValue(double val)
{
    m_slider->setValue(val * m_factor);
}
