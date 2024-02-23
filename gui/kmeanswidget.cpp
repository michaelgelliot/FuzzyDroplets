#include "kmeanswidget.h"
#include <QGroupBox>
#include <QFormLayout>
#include <QDoubleSpinBox>
#include "core/kmeans.h"
#include "emworker.h"
#include <QComboBox>

KMeansWidget::KMeansWidget(Data * data, DropletGraphWidget * graph, QWidget * parent)
    : EMClusterMethodWidget(data, graph, parent)
{
    QGroupBox * fuzzyBox = new QGroupBox("Fuzzy Clustering", this);
    fuzzyBox->setCheckable(true);
    fuzzyBox->setChecked(false);
    auto m_fuzzyBoxLayout = new QFormLayout;
    fuzzyBox->setLayout(m_fuzzyBoxLayout);
    connect(fuzzyBox, &QGroupBox::toggled, this, &KMeansWidget::useFuzzyClustering);

    m_fuzzinessSpinBox = new QDoubleSpinBox(this);
    m_fuzzinessSpinBox->setRange(1, 4);
    m_fuzzinessSpinBox->setSingleStep(0.1);
    m_fuzzinessSpinBox->setValue(1.5);
    m_fuzzinessSpinBox->setDecimals(1);
    m_fuzzinessSpinBox->setEnabled(false);
    m_fuzzyBoxLayout->addRow("Fuzziness", m_fuzzinessSpinBox);

    auto form = (QFormLayout*)layout();
    form->addRow(fuzzyBox);
}

void KMeansWidget::useFuzzyClustering(bool b)
{
    m_fuzzinessSpinBox->setEnabled(b);
    m_useFuzzy = b;
}

EMClusteringContainerBase * KMeansWidget::getClusteringContainer()
{
    double fuzzy = m_useFuzzy ? m_fuzzinessSpinBox->value() : 1;
    KMeans * km = new KMeans(data(), fuzzy, numClusters(), numReplicates(), maxIters(), centroidInitializationMethod(), sampleSize());
    return new EMClusteringContainer<Point>(km);
}

EMClusteringContainerBase * KMeansWidget::getClusteringContainer(const std::vector<Point> & centroids)
{
    double fuzzy = m_useFuzzy ? m_fuzzinessSpinBox->value() : 1;
    KMeans * km = new KMeans(data(), fuzzy, numClusters(), numReplicates(), maxIters(), CentroidInitialization::CustomCentroids, sampleSize(), centroids);
    return new EMClusteringContainer<Point>(km);
}
