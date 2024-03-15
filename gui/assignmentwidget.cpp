#include "assignmentwidget.h"

#include "randomforestwidget.h"
#include "nearestneighbourswidget.h"
#include "gmmassignmentwidget.h"
#include "paintingwidget.h"
#include "generic/stackedwidget.h"
#include "generic/launchwidget.h"
#include "generic/commandstack.h"
#include "../core/data.h"
#include "dropletgraphwidget.h"
#include <QBoxLayout>
#include <QProgressBar>
#include <QPushButton>
#include <QFormLayout>
#include <QComboBox>
#include <QCheckBox>
#include <QGroupBox>


AssignmentWidget::AssignmentWidget(Data * data, PaintingWidget * painting, DropletGraphWidget * graph, SampleListWidget * sampleList, CommandStack * commandStack, QWidget *parent)
    : QWidget{parent},
    m_data(data),
    m_painting(painting),
    m_graph(graph),
    m_commandStack(commandStack)
{
    m_launchWidget = new LaunchWidget;
    m_stackedWidget = new StackedWidget;

    auto layout = new QVBoxLayout;

    layout->addWidget(m_stackedWidget);

    m_sourceTargetGroup = new QGroupBox;
    QFormLayout * form = new QFormLayout;
    m_targetComboBox = new QComboBox;
    m_targetComboBox->addItems({"Experimental Samples", "Selection"});
    form->addRow("Assign to", m_targetComboBox);
    m_sourceTargetGroup->setLayout(form);
    layout->addWidget(m_sourceTargetGroup);

    m_unambCB = new QCheckBox("Unambiguous Samples");
    m_posCB = new QCheckBox("Positive Controls");
    m_negCB = new QCheckBox("Negative Controls");
    m_ntcCB = new QCheckBox("Non-Template Controls");
    m_unambCB->setChecked(true);
    m_posCB->setChecked(true);
    m_negCB->setChecked(true);
    m_ntcCB->setChecked(true);
    form->addRow("Assign from", m_unambCB);
    form->addRow("", m_posCB);
    form->addRow("", m_negCB);
    form->addRow("", m_ntcCB);

    layout->addWidget(m_launchWidget);

    m_launchWidget->hide();

    connect(m_targetComboBox, &QComboBox::currentIndexChanged, this, &AssignmentWidget::updateLaunchStatus);
    connect(m_posCB, &QCheckBox::stateChanged, this, &AssignmentWidget::updateLaunchStatus);
    connect(m_negCB, &QCheckBox::stateChanged, this, &AssignmentWidget::updateLaunchStatus);
    connect(m_ntcCB, &QCheckBox::stateChanged, this, &AssignmentWidget::updateLaunchStatus);
    connect(m_data, &Data::selectedSamplesChanged, this, &AssignmentWidget::updateLaunchStatus);

    connect(m_stackedWidget, &StackedWidget::widgetChanged, this, &AssignmentWidget::widgetChanged);
    connect(m_launchWidget->runButton(), &QPushButton::pressed, this, &AssignmentWidget::run);
    connect(m_launchWidget->cancelButton(), &QPushButton::pressed, this, &AssignmentWidget::cancel);

    connect(m_data, &Data::selectedSamplesChanged, this, &AssignmentWidget::selectedSamplesChanged);
    connect(m_data, &Data::sampleTypesChanged, this, &AssignmentWidget::sampleTypesChanged);

    m_stackedWidget->addWidget(new RandomForestWidget(m_data, this), "Random Forest");
    m_stackedWidget->addWidget(new NearestNeighboursWidget(m_data, this), "Nearest Neighbours");
    m_stackedWidget->addWidget(new GmmAssignmentWidget(m_data, this), "Gaussian Mixture Model");

    setLayout(layout);

    widgetChanged(nullptr, m_stackedWidget->currentWidget());
}

void AssignmentWidget::startAssignmentActions()
{
    m_sourceTargetGroup->setEnabled(false);
    m_launchWidget->progressBar()->setValue(0);
    m_launchWidget->setRunningMode();
    m_stackedWidget->disableComboBox();
    emit startParallelWork();
}

void AssignmentWidget::endAssignmentActions()
{
    m_sourceTargetGroup->setEnabled(true);
    m_launchWidget->setReadyMode();
    m_stackedWidget->enableComboBox();
    m_painting->colorsSetProgramatically();
    emit finishParallelWork();
}

void AssignmentWidget::widgetChanged(QWidget * oldWidget, QWidget * newWidget)
{
    if (oldWidget) {
        AssignmentMethodWidget * c = qobject_cast<AssignmentMethodWidget*>(newWidget);
        disconnect(c, &AssignmentMethodWidget::hideLaunchWidget, this, &AssignmentWidget::hideLaunchWidget);
        disconnect(c, &AssignmentMethodWidget::showLaunchWidget, this, &AssignmentWidget::showLaunchWidget);
        disconnect(c, &AssignmentMethodWidget::beginAssignment, this, &AssignmentWidget::startAssignmentActions);
        disconnect(c, &AssignmentMethodWidget::endAssignment, this, &AssignmentWidget::endAssignmentActions);
        disconnect(c, &AssignmentMethodWidget::updateProgress, m_launchWidget->progressBar(), &QProgressBar::setValue);
        c->deactivate();
    }

    AssignmentMethodWidget * c = qobject_cast<AssignmentMethodWidget*>(newWidget);
    m_launchWidget->setVisible(false);
    ((QVBoxLayout*)layout())->removeWidget(m_launchWidget);

    if (c) {
        if (c->requiresLaunchWidget()) {
            m_launchWidget->setVisible(true);
            ((QVBoxLayout*)layout())->addWidget(m_launchWidget);
        }
        c->activate();
        connect(c, &AssignmentMethodWidget::hideLaunchWidget, this, &AssignmentWidget::hideLaunchWidget);
        connect(c, &AssignmentMethodWidget::showLaunchWidget, this, &AssignmentWidget::showLaunchWidget);
        connect(c, &AssignmentMethodWidget::beginAssignment, this, &AssignmentWidget::startAssignmentActions);
        connect(c, &AssignmentMethodWidget::endAssignment, this, &AssignmentWidget::endAssignmentActions);
        connect(c, &AssignmentMethodWidget::updateProgress, m_launchWidget->progressBar(), &QProgressBar::setValue);
    }

    updateLaunchStatus();
}

void AssignmentWidget::hideLaunchWidget()
{
    m_launchWidget->hide();
}

void AssignmentWidget::showLaunchWidget()
{
    m_launchWidget->show();
}

void AssignmentWidget::run()
{
    AssignmentMethodWidget * c = static_cast<AssignmentMethodWidget*>(m_stackedWidget->currentWidget());
    m_launchWidget->setProperties(c->canCancel(), c->providesProgressUpdates());

    auto source = sourceIndices();
    auto target = targetIndices();
    if (source.size() > 0 && target.size() > 0)
    c->run(source, target);
}

QList<size_t> AssignmentWidget::targetIndices() const
{
    QList<size_t> target;
    if (m_targetComboBox->currentIndex() == 0) { // target sample data
        for (int i = 0; i < m_data->sampleCount(); ++i) {
            if (m_data->sampleType(i) == Data::SampleType::Experimental) {
                auto bounds = m_data->sampleIndices(i);
                for (auto i = bounds[0]; i < bounds[1]; ++i) {
                    target.push_back(i);
                }
            }
        }
    } else { // target selection
        for (auto sampleIdx : m_data->selectedSamples()) {
            auto bounds = m_data->sampleIndices(sampleIdx);
            for (auto i = bounds[0]; i < bounds[1]; ++i) {
                target.push_back(i);
            }
        }
    }
    return target;
}

QList<size_t> AssignmentWidget::sourceIndices() const
{
    QList<size_t> source;

    if (m_posCB->isChecked()) {
        for (int i = 0; i < m_data->sampleCount(); ++i) {
            if (m_data->sampleType(i) == Data::SampleType::PositiveControl && !(m_targetComboBox->currentIndex() == 0 && m_data->sampleType(i) == Data::SampleType::Experimental)) {
                auto bounds = m_data->sampleIndices(i);
                for (auto j = bounds[0]; j < bounds[1]; ++j) {
                    if (!(m_targetComboBox->currentIndex() == 1 && m_data->isSelected(j)))
                        source.push_back(j);
                }
            }
        }
    }

    if (m_negCB->isChecked()) {
        for (int i = 0; i < m_data->sampleCount(); ++i) {
            if (m_data->sampleType(i) == Data::SampleType::NegativeControl && !(m_targetComboBox->currentIndex() == 0 && m_data->sampleType(i) == Data::SampleType::Experimental)) {
                auto bounds = m_data->sampleIndices(i);
                for (auto j = bounds[0]; j < bounds[1]; ++j) {
                    if (!(m_targetComboBox->currentIndex() == 1 && m_data->isSelected(j)))
                        source.push_back(j);
                }
            }
        }
    }

    if (m_ntcCB->isChecked()) {
        for (int i = 0; i < m_data->sampleCount(); ++i) {
            if (m_data->sampleType(i) == Data::SampleType::NonTemplateControl && !(m_targetComboBox->currentIndex() == 0 && m_data->sampleType(i) == Data::SampleType::Experimental)) {
                auto bounds = m_data->sampleIndices(i);
                for (auto j = bounds[0]; j < bounds[1]; ++j) {
                    if (!(m_targetComboBox->currentIndex() == 1 && m_data->isSelected(j)))
                        source.push_back(j);
                }
            }
        }
    }

    if (m_unambCB->isChecked()) {
        for (int i = 0; i < m_data->sampleCount(); ++i) {
            if (m_data->sampleType(i) == Data::SampleType::UnambiguousSample && !(m_targetComboBox->currentIndex() == 0 && m_data->sampleType(i) == Data::SampleType::Experimental)) {
                auto bounds = m_data->sampleIndices(i);
                for (auto j = bounds[0]; j < bounds[1]; ++j) {
                    if (!(m_targetComboBox->currentIndex() == 1 && m_data->isSelected(j)))
                        source.push_back(j);
                }
            }
        }
    }

    return source;
}


void AssignmentWidget::parallelWorkStarted()
{
    setEnabled(false);
}

void AssignmentWidget::parallelWorkFinished()
{
    setEnabled(true);
}

void AssignmentWidget::cancel()
{
    AssignmentMethodWidget * c = static_cast<AssignmentMethodWidget*>(m_stackedWidget->currentWidget());
    c->cancel();

//    // restore original colours
//    size_t pos = 0;
//    for (size_t i = 0; i < m_data->points().size(); ++i) {
//        if (m_data->isSelected(i)) {
//            m_data->setColor(i, m_originalColors[pos]);
//            ++pos;
//        }
//    }
}


void AssignmentWidget::updateLaunchStatus()
{
    bool targetIsValid = false;
    if (m_targetComboBox->currentIndex() == 0) { // target sample data
        for (int i = 0; i < m_data->sampleCount(); ++i) {
            if (m_data->sampleType(i) == Data::Experimental) {
                targetIsValid = true;
                break;
            }
        }
    } else { // target selection
        targetIsValid = (m_data->selectedSamples().size() > 0);
    }

    bool sourceIsValid = false;
    for (size_t i = 0; i < m_data->sampleCount(); ++i) {
        if (m_posCB->isChecked() && m_data->sampleType(i) == Data::PositiveControl && (m_targetComboBox->currentIndex() == 0 || std::find(m_data->selectedSamples().begin(), m_data->selectedSamples().end(), i) == m_data->selectedSamples().end())) {
            sourceIsValid = true;
            break;
        }
        if (m_negCB->isChecked() && m_data->sampleType(i) == Data::NegativeControl && (m_targetComboBox->currentIndex() == 0 || std::find(m_data->selectedSamples().begin(), m_data->selectedSamples().end(), i) == m_data->selectedSamples().end())) {
            sourceIsValid = true;
            break;
        }
        if (m_ntcCB->isChecked() && m_data->sampleType(i) == Data::NonTemplateControl && (m_targetComboBox->currentIndex() == 0 || std::find(m_data->selectedSamples().begin(), m_data->selectedSamples().end(), i) == m_data->selectedSamples().end())) {
            sourceIsValid = true;
            break;
        }
    }

    m_launchWidget->setEnabled(sourceIsValid && targetIsValid);
}

void AssignmentWidget::sampleTypesChanged(std::vector<size_t> samples)
{
    setEnabled(m_data->selectedSamples().size() > 0 && hasMixedTypes());
    updateLaunchStatus();
}

void AssignmentWidget::selectedSamplesChanged()
{
    setEnabled(m_data->selectedSamples().size() > 0 && hasMixedTypes());
    updateLaunchStatus();
}

bool AssignmentWidget::hasMixedTypes()
{
    if (m_data->sampleCount() > 0) {
        auto type = m_data->sampleType(0);
        for (size_t i = 1; i < m_data->sampleCount(); ++i)
            if (m_data->sampleType(i) != type) return true;
    }
    return false;
}
