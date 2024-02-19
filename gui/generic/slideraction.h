#ifndef SLIDERACTION_H
#define SLIDERACTION_H

#include <QSlider>
#include <QWidgetAction>
#include <QBoxLayout>
#include <QLabel>

class SliderAction : public QWidgetAction {
public:
    SliderAction (const QString& title) :
        QWidgetAction (NULL) {
        QWidget* pWidget = new QWidget (NULL);
        QHBoxLayout* pLayout = new QHBoxLayout();
        //QLabel* pLabel = new QLabel (title);  //bug fixed here, pointer was missing
        //pLayout->addWidget (pLabel);
        pSpinBox = new QSlider(NULL);
        pSpinBox->setOrientation(Qt::Horizontal);
        pLayout->addWidget (pSpinBox);
        pWidget->setLayout (pLayout);
        setDefaultWidget(pWidget);
    }

    QSlider * slider () {
        return pSpinBox;
    }

private:
    QSlider * pSpinBox;
};

#endif // SLIDERACTION_H
