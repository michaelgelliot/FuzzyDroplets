#ifndef FUZZY_DROPLETS_MAINWINDOW_H
#define FUZZY_DROPLETS_MAINWINDOW_H

#include <QMainWindow>
#include "plot/axis.h"

class Data;
class CommandStack;
class SampleListWidget;
class DropletGraphWidget;
class ShelfWidget;
class ClusteringWidget;
class QSplitter;
class ExperimentalDesignWidget;
class PaintingWidget;
class AssignmentWidget;
class QPushButton;


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:

    enum Theme
    {
        Dark,
        Light,
        User
    };

    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    Data * data() {return m_data;}
    const Data * data() const  {return m_data;}

protected:

    void closeEvent(QCloseEvent * e);

public slots:

    void showExperimentalDesignDialog();
    void hideExperimentalDesignDialog();
    void addDataFiles();
    void addFolder();
    bool exportAll();
    void exportJPG();
    void guiColorSchemeChanged();
    void exportReport();

private slots:

    void setLeftAxisComponentVisibility(bool);
    void setRightAxisComponentVisibility(bool);
    void setTopAxisComponentVisibility(bool);
    void setBottomAxisComponentVisibility(bool);
    void zoomMarkersCompletely();
    void zoomMarkersPartially();
    void doNotZoomMarkers();
    void parallelWorkStarted();
    void parallelWorkFinished();
    void designChanged();
    void selectedSamplesChanged();
    void citation();
    void openTutorial();
    void setUserTheme();
    void setDarkTheme();
    void setLightTheme();

private:

    void processFileLoadErrorMessage(const std::string & error);
    void setAxisComponentVisibility(QString name, Plot::Axis * axis, Plot::Axis::Component component, bool visible);
    void exportTIFF();

    Data * m_data;
    CommandStack * m_commandStack {nullptr};
    SampleListWidget * m_sampleListWidget {nullptr};
    DropletGraphWidget * m_graphWidget {nullptr};
    QSplitter * m_splitter;
    QWidget * m_shelfWidgetContainer;
    ShelfWidget * m_shelfWidget;
    ClusteringWidget * m_clusteringWidget;
    AssignmentWidget * m_assignmentWidget;
    ExperimentalDesignWidget * m_designWidget;
    PaintingWidget * m_paintingWidget;

    QAction * m_addDataFilesAction;
    QAction * m_addFolderAction;
    QAction * m_expDesignAction;
    QAction * m_saveAction;
    QAction * m_exportReportAction;
    QAction * m_exitAction;
    QAction * m_undoAction;
    QAction * m_redoAction;
    QAction * m_convexHullAction;
    QAction * m_zoomInAction;
    QAction * m_zoomOutAction;
    QAction * m_zoomResetAction;
    QMenu * m_exportImageMenu;
    QMenu * m_topAxisMenu;
    QMenu * m_bottomAxisMenu;
    QMenu * m_leftAxisMenu;
    QMenu * m_rightAxisMenu;
    QMenu * m_selectionMenu;
    QPushButton * m_expDesignPushButton;
    Theme m_theme {User};
};

#endif // FUZZY_DROPLETS_MAINWINDOW_H
