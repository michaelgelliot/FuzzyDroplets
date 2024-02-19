#ifndef EMCLUSTERMETHODWIDGET_H
#define EMCLUSTERMETHODWIDGET_H

#include "gui/clustermethodwidget.h"
#include "core/geometry.h"
#include "core/centroids.h"

class Data;
class DropletGraphWidget;
class QSpinBox;
class QComboBox;
class QPushButton;
class QFormLayout;
struct EMClusteringContainerBase;

namespace Plot
{
class RingMarker;
}

class EMClusterMethodWidget : public ClusterMethodWidget
{
    Q_OBJECT

public:

    explicit EMClusterMethodWidget(Data * data, DropletGraphWidget * graph, QWidget *parent = nullptr);
    virtual ~EMClusterMethodWidget();

    virtual bool providesProgressUpdates() const override {return true;}
    virtual bool canCancel() const override {return true;}

    virtual void run() override;
    virtual void cancel() override;

    virtual void activate() override;
    virtual void deactivate() override;

    Data * data() {return m_data;}
    const Data * data() const {return m_data;}

    DropletGraphWidget * graph() {return m_graph;}
    const DropletGraphWidget * graph() const {return m_graph;}

    int numClusters() const;
    int numReplicates() const;
    int maxIters() const;
    int sampleSize() const;
    CentroidInitialization centroidInitializationMethod() const;

protected:

    bool eventFilter(QObject *obj, QEvent *event) override;

    virtual EMClusteringContainerBase * getClusteringContainer() = 0;
    virtual EMClusteringContainerBase * getClusteringContainer(const std::vector<Point> & centroids) = 0;

public slots:

    void dataColorCountChanged();
    void numClustersChanged();
    void initMethodChanged();
    void initializeCentroidMarkers();
    void threadFinished();

signals:

    void startThread();
    void cancelThread();

protected:

    void stopThread();

    Data * m_data;
    DropletGraphWidget * m_graph;

    QSpinBox * m_numClusters;
    QSpinBox * m_replicates;
    QSpinBox * m_maxIters;
    QComboBox * m_centroidInit;
    QSpinBox * m_sampleSize;
    QPushButton * m_reinitialize;
    QFormLayout * m_initBoxLayout;

    int m_backupNumReplicates {20};
    int m_backupSampleSize {1000};

    std::vector<Plot::RingMarker *> m_markers;
    int m_selectedCentroid {-1};
    bool m_dragging {false};
    Point m_dragOffset;

    QThread * workerThread {nullptr};
};

#endif // EMCLUSTERMETHODWIDGET_H
