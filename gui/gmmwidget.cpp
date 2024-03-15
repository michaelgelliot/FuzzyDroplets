#include "gmmwidget.h"
#include <QGroupBox>
#include <QRadioButton>
#include <QFormLayout>
#include <QButtonGroup>
#include <QComboBox>
#include <QSpinBox>
#include <QCheckBox>
#include "../core/gmm.h"
#include "emworker.h"

GMMWidget::GMMWidget(Data * data, DropletGraphWidget * graph, QWidget * parent)
    : EMClusterMethodWidget(data, graph, parent)
{
    auto form = (QFormLayout*)layout();

    QGroupBox * constraintsBox= new QGroupBox("Constraints");
    constraintsLayout = new QFormLayout;

    m_clusterOutliersCheckbox = new QCheckBox(this);
    constraintsLayout->addRow("Cluster Outliers", m_clusterOutliersCheckbox);
    m_clusterOutliersCheckbox->setToolTip("Model outliers as uniformly distributed across the graph");
    constraintsLayout->labelForField(m_clusterOutliersCheckbox)->setToolTip("Model outliers as uniformly distributed across the graph");

    m_fixedMeanCheckbox = new QCheckBox(this);
    constraintsLayout->addRow( "Fixed Means", m_fixedMeanCheckbox);
    m_sharedScaleCheckbox = new QCheckBox(this);
    constraintsLayout->addRow( "Shared Scale", m_sharedScaleCheckbox);
    m_sharedRhoCheckbox = new QCheckBox(this);
    constraintsLayout->addRow( "Shared Correlation Coefficient", m_sharedRhoCheckbox);

    constraintsBox->setLayout(constraintsLayout);
    form->addRow(constraintsBox);

    QGroupBox * fuzzyBox = new QGroupBox("Output");
    QRadioButton * fuzzy = new QRadioButton;
    fuzzy->setChecked(true);
    QRadioButton * randomDiscrete = new QRadioButton;
    QRadioButton * deterministicDiscrete = new  QRadioButton;
    m_fuzzyButtonGroup = new QButtonGroup;
    m_fuzzyButtonGroup->addButton(fuzzy, 0);
    m_fuzzyButtonGroup->addButton(randomDiscrete, 1);
    m_fuzzyButtonGroup->addButton(deterministicDiscrete, 2);
    QFormLayout * fuzzyLayout = new QFormLayout;
    fuzzyLayout->addRow("Fuzzy", fuzzy);
    fuzzyLayout->addRow("Randomly Discretized", randomDiscrete);
    fuzzyLayout->addRow("Deterministically Discretized", deterministicDiscrete);
    fuzzyBox->setLayout(fuzzyLayout);
    form->addRow(fuzzyBox);
}

EMClustering<BinormalDistribution>::DefuzzificationPolicy GMMWidget::getDefuzzificationPolicy(int id)
{
    return (id == 0) ? EMClustering<BinormalDistribution>::Fuzzy : ((id == 1) ? EMClustering<BinormalDistribution>::RandomlyDefuzzify : EMClustering<BinormalDistribution>::DeterministicallyDefuzzify);
}

EMClusteringContainerBase * GMMWidget::getClusteringContainer()
{
    Gmm * gmm = new Gmm(data(), m_clusterOutliersCheckbox->isChecked(), m_fixedMeanCheckbox->isChecked(), m_sharedScaleCheckbox->isChecked(), m_sharedRhoCheckbox->isChecked(), getDefuzzificationPolicy(m_fuzzyButtonGroup->checkedId()), numClusters(), numReplicates(), maxIters(), centroidInitializationMethod(), sampleSize());
    return new EMClusteringContainer<BinormalDistribution>(gmm);
}

EMClusteringContainerBase * GMMWidget::getClusteringContainer(const std::vector<Point> & centroids)
{
    Gmm * gmm = new Gmm(data(), m_clusterOutliersCheckbox->isChecked(), m_fixedMeanCheckbox->isChecked(), m_sharedScaleCheckbox->isChecked(), m_sharedRhoCheckbox->isChecked(), getDefuzzificationPolicy(m_fuzzyButtonGroup->checkedId()), numClusters(), numReplicates(), maxIters(), CentroidInitialization::CustomCentroids, sampleSize(), centroids);
    return new EMClusteringContainer<BinormalDistribution>(gmm);
}
