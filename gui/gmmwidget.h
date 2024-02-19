#ifndef GMMWIDGET_H
#define GMMWIDGET_H

#include "emclustermethodwidget.h"
#include "core/emclustering.h"
#include "core/binormal.h"

class QCheckBox;
class QButtonGroup;

class GMMWidget : public EMClusterMethodWidget
{
    Q_OBJECT

public:

    GMMWidget(Data * data, DropletGraphWidget * graph, QWidget * parent = nullptr);

protected:

    EMClusteringContainerBase * getClusteringContainer();
    EMClusteringContainerBase * getClusteringContainer(const std::vector<Point> & centroids);
    EMClustering<BinormalDistribution>::DefuzzificationPolicy getDefuzzificationPolicy(int id);

private:

    bool m_clusterOutliers {true};
    QButtonGroup * m_fuzzyButtonGroup;
    QFormLayout * constraintsLayout;
    QCheckBox * m_clusterOutliersCheckbox;
    QCheckBox * m_fixedMeanCheckbox;
    QCheckBox * m_sharedScaleCheckbox;
    QCheckBox * m_sharedRhoCheckbox;
};

#endif // GMMWIDGET_H
