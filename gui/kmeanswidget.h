#ifndef KMEANSWIDGET_H
#define KMEANSWIDGET_H

#include "emclustermethodwidget.h"

class QDoubleSpinBox;

class KMeansWidget : public EMClusterMethodWidget
{
    Q_OBJECT

public:

    KMeansWidget(Data * data, DropletGraphWidget * graph, QWidget * parent = nullptr);

protected:

    EMClusteringContainerBase * getClusteringContainer();
    EMClusteringContainerBase * getClusteringContainer(const std::vector<Point> & centroids);

public slots:

    void useFuzzyClustering(bool b);

private:

    QDoubleSpinBox * m_fuzzinessSpinBox;
    bool m_useFuzzy {false};
};

#endif // KMEANSWIDGET_H
