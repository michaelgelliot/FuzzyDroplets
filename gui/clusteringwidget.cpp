#include "clusteringwidget.h"
#include "clustermethodwidget.h"
#include "generic/stackedwidget.h"
#include "generic/launchwidget.h"
#include "generic/commandstack.h"
#include "core/data.h"
#include "dropletgraphwidget.h"
#include "kmeanswidget.h"
#include "gmmwidget.h"
#include "dbscanwidget.h"
#include "paintingwidget.h"
#include <set>
#include <QBoxLayout>
#include <QProgressBar>
#include <QPushButton>

ClusteringWidget::ClusteringWidget(Data * data, PaintingWidget * painting,  DropletGraphWidget * graph, SampleListWidget * sampleList, CommandStack * commandStack, QWidget *parent)
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
    layout->addWidget(m_launchWidget);

    setLayout(layout);

    m_launchWidget->hide();

    connect(m_stackedWidget, &StackedWidget::widgetChanged, this, &ClusteringWidget::widgetChanged);
    connect(m_launchWidget->runButton(), &QPushButton::pressed, this, &ClusteringWidget::run);
    connect(m_launchWidget->cancelButton(), &QPushButton::pressed, this, &ClusteringWidget::cancel);

    m_stackedWidget->addWidget(new KMeansWidget(m_data, graph, this), "k-Means Clustering");
    m_stackedWidget->addWidget(new GMMWidget(m_data, graph, this), "Gaussian Mixture Model");
    m_stackedWidget->addWidget(new DBScanWidget(m_data, graph, this), "DBSCAN");

    widgetChanged(nullptr, m_stackedWidget->currentWidget());
}

void ClusteringWidget::widgetChanged(QWidget * oldWidget, QWidget * newWidget)
{
    if (oldWidget) {
        ClusterMethodWidget * c = dynamic_cast<ClusterMethodWidget*>(oldWidget);
        disconnect(c, &ClusterMethodWidget::hideLaunchWidget, this, &ClusteringWidget::hideLaunchWidget);
        disconnect(c, &ClusterMethodWidget::showLaunchWidget, this, &ClusteringWidget::showLaunchWidget);
        disconnect(c, &ClusterMethodWidget::endClustering, this, &ClusteringWidget::cleanUpAfterClustering);
        disconnect(c, &ClusterMethodWidget::beginClustering, m_launchWidget, &LaunchWidget::setRunningMode);
        disconnect(c, &ClusterMethodWidget::endClustering, m_launchWidget, &LaunchWidget::setReadyMode);
        disconnect(c, &ClusterMethodWidget::beginClustering, this, &ClusteringWidget::startParallelWork);
        disconnect(c, &ClusterMethodWidget::endClustering, this, &ClusteringWidget::finishParallelWork);
        disconnect(c, &ClusterMethodWidget::beginClustering, m_stackedWidget, &StackedWidget::disableComboBox);
        disconnect(c, &ClusterMethodWidget::endClustering, m_stackedWidget, &StackedWidget::enableComboBox);
        disconnect(c, &ClusterMethodWidget::updateProgress, m_launchWidget->progressBar(), &QProgressBar::setValue);
        c->deactivate();
    }

    ClusterMethodWidget * c = qobject_cast<ClusterMethodWidget*>(newWidget);
    m_launchWidget->setVisible(false);
    ((QVBoxLayout*)layout())->removeWidget(m_launchWidget);

    if (c) {
        if (c->requiresLaunchWidget()) {
            m_launchWidget->setVisible(true);
            ((QVBoxLayout*)layout())->addWidget(m_launchWidget);
        }
        c->activate();
        connect(c, &ClusterMethodWidget::hideLaunchWidget, this, &ClusteringWidget::hideLaunchWidget);
        connect(c, &ClusterMethodWidget::showLaunchWidget, this, &ClusteringWidget::showLaunchWidget);
        connect(c, &ClusterMethodWidget::endClustering, this, &ClusteringWidget::cleanUpAfterClustering);
        connect(c, &ClusterMethodWidget::beginClustering, m_launchWidget, &LaunchWidget::setRunningMode);
        connect(c, &ClusterMethodWidget::endClustering, m_launchWidget, &LaunchWidget::setReadyMode);
        connect(c, &ClusterMethodWidget::beginClustering, this, &ClusteringWidget::startParallelWork);
        connect(c, &ClusterMethodWidget::endClustering, this, &ClusteringWidget::finishParallelWork);
        connect(c, &ClusterMethodWidget::beginClustering, m_stackedWidget, &StackedWidget::disableComboBox);
        connect(c, &ClusterMethodWidget::endClustering, m_stackedWidget, &StackedWidget::enableComboBox);
        connect(c, &ClusterMethodWidget::updateProgress, m_launchWidget->progressBar(), &QProgressBar::setValue);
    }
}

void ClusteringWidget::parallelWorkStarted()
{
    setEnabled(false);
}

void ClusteringWidget::parallelWorkFinished()
{
    setEnabled(true);
}

void ClusteringWidget::hideLaunchWidget()
{
    m_launchWidget->hide();
}

void ClusteringWidget::showLaunchWidget()
{
    m_launchWidget->show();
}

void ClusteringWidget::run()
{
    // back up the original colours so we can restore them if the run is cancelled or if the user clicks undo
    m_originalColors.clear();
    m_originalColors.reserve(m_data->selectedPointCount());
    for (size_t i = 0; i < m_data->points().size(); ++i)
        if (m_data->isSelected(i)) m_originalColors.push_back(m_data->fuzzyColor(i));
    ClusterMethodWidget * c = static_cast<ClusterMethodWidget*>(m_stackedWidget->currentWidget());
    m_launchWidget->setProperties(c->canCancel(), c->providesProgressUpdates());
    c->run();
}

void ClusteringWidget::cancel()
{
    ClusterMethodWidget * c = static_cast<ClusterMethodWidget*>(m_stackedWidget->currentWidget());
    c->cancel();

    // restore original colours
    size_t pos = 0;
    for (size_t i = 0; i < m_data->points().size(); ++i) {
        if (m_data->isSelected(i)) {
            m_data->setColor(i, m_originalColors[pos]);
            ++pos;
        }
    }
}

void ClusteringWidget::cleanUpAfterClustering()
{
    ClusterMethodWidget * c = static_cast<ClusterMethodWidget*>(m_stackedWidget->currentWidget());

    if (c->requiresColorCorrection()) {
        if (m_data->design()->isValid()) {
            m_data->matchColorsToDesign(Data::Selected);
        } else if (m_data->pointCount() != m_data->selectedPointCount()) {
            m_data->matchSelectedColorToUnselectedColors();
        }
    }

    m_painting->colorsSetProgramatically();
}
