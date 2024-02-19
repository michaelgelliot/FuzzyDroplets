#ifndef FUZZYDROPLETS_GUI_CLUSTERMETHODWIDGET_H
#define FUZZYDROPLETS_GUI_CLUSTERMETHODWIDGET_H

#include <QWidget>

class Droplet;
class Data;

class ClusterMethodWidget : public QWidget
{
    Q_OBJECT

public:

    ClusterMethodWidget(QWidget *parent = nullptr);
    virtual ~ClusterMethodWidget();

    virtual void run() {};             // run the clustering
    virtual void cancel() {}           // cancel the clustering

    virtual bool requiresColorCorrection() const {return true;}     // reimplement to request no color correction after clustering
    virtual bool requiresLaunchWidget() const {return true;}        // reimplement to request no launch widget, continuous updates instead
    virtual bool providesProgressUpdates() const {return false;}    // reimplement to request a progress bar
    virtual bool canCancel() const {return false;}                  // reimplement to request a cancel button while running

    virtual void activate() {}
    virtual void deactivate() {}

signals:

    void beginClustering();
    void updateProgress(int percent);
    void endClustering();
    void hideLaunchWidget();
    void showLaunchWidget();
    void setDocumentModified(bool);
};

#endif // FUZZYDROPLETS_GUI_CLUSTERMETHODWIDGET_H
