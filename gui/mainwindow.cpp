#include "mainwindow.h"
#include "core/colorscheme.h"
#include "core/data.h"
#include "gui/generic/themedicon.h"
#include "qdialogbuttonbox.h"
#include "qevent.h"
#include "qpainter.h"
#include "samplelistwidget.h"
#include "dropletgraphwidget.h"
#include "clusteringwidget.h"
#include "assignmentwidget.h"
#include "paintingwidget.h"
#include "experimentaldesignwidget.h"
#include "generic/commandstack.h"
#include "generic/shelfwidget.h"
#include "generic/themedicon.h"
#include "gui/generic/commandstack.h"
#include "pointcloud.h"
#include "plot/axis.h"
#include "gui/generic/slideraction.h"

#include <QPlainTextEdit>
#include <QFileDialog>
#include <QSettings>
#include <QGuiApplication>
#include <QScreen>
#include <QSplitter>
#include <QMenuBar>
#include <QMessageBox>
#include <QSvgRenderer>
#include <QImageWriter>
#include <QTemporaryFile>
#include <QBoxLayout>
#include <QPushButton>
#include <QActionGroup>
#include <QStyleHints>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
    m_data(new Data(this)),
    m_commandStack(new CommandStack(this))
{
    setMouseTracking(true);
    resize(qGuiApp->primaryScreen()->size().width() * 0.75, qGuiApp->primaryScreen()->size().height() * 0.6);

    m_sampleListWidget = new SampleListWidget(m_data, m_commandStack, this);
    m_graphWidget = new DropletGraphWidget(m_data, m_commandStack, this);
    m_paintingWidget = new PaintingWidget(m_data, m_commandStack, m_graphWidget, this);
    m_clusteringWidget = new ClusteringWidget(m_data, m_paintingWidget, m_graphWidget, m_sampleListWidget, m_commandStack, this);
    m_assignmentWidget = new AssignmentWidget(m_data, m_paintingWidget, m_graphWidget, m_sampleListWidget, m_commandStack, this);

    m_shelfWidget = new ShelfWidget;
    m_shelfWidget->addWidget(m_paintingWidget, "Painting Tools");
    m_shelfWidget->addWidget(m_clusteringWidget, "Clustering");
    m_shelfWidget->addWidget(m_assignmentWidget, "Assignment");

    m_paintingWidget->setEnabled(false);
    m_clusteringWidget->setEnabled(false);
    m_assignmentWidget->setEnabled(false);
    m_graphWidget->setEnabled(false);

    m_shelfWidgetContainer = new QWidget;
    QVBoxLayout * layout = new QVBoxLayout;
    m_expDesignPushButton = new QPushButton(themedIcon(":/science"), "Experimental Design...");
    m_expDesignPushButton->setEnabled(false);
    connect(m_expDesignPushButton, &QPushButton::pressed, this, &MainWindow::showExperimentalDesignDialog);
    layout->addWidget(m_expDesignPushButton);
    layout->addWidget(m_shelfWidget);
    m_shelfWidgetContainer->setLayout(layout);

    m_splitter = new QSplitter;
    setCentralWidget(m_splitter);
    m_splitter->setStyleSheet("QSplitter::handle {width: 1}");
    m_splitter->addWidget(m_sampleListWidget);
    m_splitter->addWidget(m_graphWidget);
    m_splitter->addWidget(m_shelfWidgetContainer);
    m_splitter->setStretchFactor(0,0);
    m_splitter->setStretchFactor(1,1);
    m_splitter->setStretchFactor(2,0);

    auto fileMenu = menuBar()->addMenu("&File");
    m_addDataFilesAction = fileMenu->addAction(themedIcon(":/file"), "Add Data Files...", this, &MainWindow::addDataFiles);
    m_addFolderAction = fileMenu->addAction(themedIcon(":/folder"), "Add Folder...", this, &MainWindow::addFolder);
    fileMenu->addSeparator();
    m_saveAction = fileMenu->addAction(themedIcon(":/save"), "Save As...", this, &MainWindow::exportAll);
    m_saveAction->setEnabled(false);
    fileMenu->addSeparator();
    m_svgMenu = fileMenu->addMenu("Export SVG");
    m_svgSquareAction = m_svgMenu->addAction(themedIcon(":/square"), "Square Markers...", this, &MainWindow::exportSVGSquare);
    m_svgRoundAction = m_svgMenu->addAction(themedIcon(":/circle"), "Round Markers...", this, &MainWindow::exportSVGRound);
    m_svgMenu->setEnabled(false);
    m_jpgMenu = fileMenu->addMenu("Export JPEG");
    m_jpgSquareAction = m_jpgMenu->addAction(themedIcon(":/square"),"Square Markers...", this, &MainWindow::exportJPGSquare);
    m_jpgRoundAction = m_jpgMenu->addAction(themedIcon(":/circle"), "Round Markers...", this, &MainWindow::exportJPGRound);
    m_jpgMenu->setEnabled(false);
    fileMenu->addSeparator();
    m_exportReportAction = fileMenu->addAction(themedIcon(":/exportReport"), "Export Assignment Report...", this, &MainWindow::exportReport);
    m_exportReportAction->setEnabled(false);
    fileMenu->addSeparator();
    m_exitAction = fileMenu->addAction(themedIcon(":/exit"), "Exit", this, &MainWindow::close);

    auto editMenu = menuBar()->addMenu("&Edit");
    m_undoAction = editMenu->addAction(themedIcon(":/undo"), "Undo", QKeySequence::Undo, m_commandStack, &CommandStack::undo);
    m_redoAction = editMenu->addAction(themedIcon(":/redo"), "Redo", QKeySequence::Redo, m_commandStack, &CommandStack::redo);
    connect(m_commandStack, &CommandStack::undoAvailable, m_undoAction, &QAction::setEnabled);
    connect(m_commandStack, &CommandStack::redoAvailable, m_redoAction, &QAction::setEnabled);
    m_undoAction->setEnabled(false);
    m_redoAction->setEnabled(false);
    editMenu->addSeparator();
    m_expDesignAction = editMenu->addAction(themedIcon(":/science"), "Experimental Design...", this, &MainWindow::showExperimentalDesignDialog);
    m_expDesignAction->setEnabled(false);
    editMenu->addSeparator();
    m_selectionMenu = editMenu->addMenu("Selection");
    m_selectionMenu->addAction("Select All", m_sampleListWidget, &SampleListWidget::selectAll);
    m_selectionMenu->addAction("Select Experimental Samples", m_sampleListWidget, &SampleListWidget::selectExperimentalSamples);
    m_selectionMenu->addAction("Select Positive Controls", m_sampleListWidget, &SampleListWidget::selectPositiveControls);
    m_selectionMenu->addAction("Select Negative Controls", m_sampleListWidget, &SampleListWidget::selectNegativeControls);
    m_selectionMenu->addAction("Select Non-Template Controls", m_sampleListWidget, &SampleListWidget::selectNonTemplateControls);
    m_selectionMenu->addSeparator();
    m_selectionMenu->addAction("Invert Selection", m_sampleListWidget, &SampleListWidget::invertSelection);
    m_selectionMenu->setEnabled(false);
    editMenu->addSeparator();

    auto viewMenu = menuBar()->addMenu("&View");
    m_zoomInAction = viewMenu->addAction(themedIcon(":/zoomIn"), "Zoom In", QKeySequence::ZoomIn, m_graphWidget, &DropletGraphWidget::zoomIn);
    m_zoomOutAction = viewMenu->addAction(themedIcon(":/zoomOut"), "Zoom Out", QKeySequence::ZoomOut, m_graphWidget, &DropletGraphWidget::zoomOut);
    viewMenu->addSeparator();
    m_zoomResetAction = viewMenu->addAction("Reset Zoom", QKeyCombination(Qt::ControlModifier, Qt::Key_0), m_graphWidget, &DropletGraphWidget::resetZoom);
    m_zoomInAction->setAutoRepeat(false);
    m_zoomOutAction->setAutoRepeat(false);
    m_zoomResetAction->setAutoRepeat(false);
    m_zoomInAction->setEnabled(false);
    m_zoomOutAction->setEnabled(false);
    m_zoomResetAction->setEnabled(false);
    viewMenu->addSeparator();
    m_convexHullAction = viewMenu->addAction("Convex Hulls", QKeyCombination(Qt::ControlModifier, Qt::Key_H), m_graphWidget, &DropletGraphWidget::setConvexHullsVisible);
    m_convexHullAction->setCheckable(true);
    m_convexHullAction->setEnabled(false);
    viewMenu->addSeparator();
    auto markersMenu = viewMenu->addMenu("Markers");
    auto markerSizeMenu = markersMenu->addMenu("Size");
    SliderAction * sliderAction = new SliderAction(tr(""));
    sliderAction->slider()->setRange(1, 20);
    QSettings settings;
    sliderAction->slider()->setValue(settings.value("graphWidgetMarkerSize", 3).toDouble());
    connect(sliderAction ->slider(), &QSlider::valueChanged, m_graphWidget, &DropletGraphWidget::setMarkerSize);
    markerSizeMenu->addAction(sliderAction);

    auto zoomMarkers = markersMenu->addMenu("Zoom Style");
    auto z1 = zoomMarkers->addAction("Zoom Completely", this, &MainWindow::zoomMarkersCompletely); z1->setCheckable(true);
    auto z2 = zoomMarkers->addAction("Zoom Partially", this, &MainWindow::zoomMarkersPartially); z2->setCheckable(true);
    auto z3 = zoomMarkers->addAction("Do Not Zoom", this, &MainWindow::doNotZoomMarkers); z3->setCheckable(true);
    QActionGroup * zoomGroup = new QActionGroup(this);
    zoomGroup->addAction(z1);
    zoomGroup->addAction(z2);
    zoomGroup->addAction(z3);
    double zoomType = settings.value("graphWidgetScaleFactor", 0.15).toDouble();
    if (zoomType > 0.999) {
        m_graphWidget->setScaleFactor(1);
        z1->setChecked(true);
    }
    else if (zoomType < 0.001) {
        m_graphWidget->setScaleFactor(0);
        z3->setChecked(true);
    } else {
        m_graphWidget->setScaleFactor(0.15);
        z2->setChecked(true);
    }

    auto helpMenu = menuBar()->addMenu("Help");
    helpMenu->addAction("How to Cite...", this, &MainWindow::citation);

    m_leftAxisMenu = viewMenu->addMenu(themedIcon(":/borderLeft"), "Left Axis");
    auto leftAxisLabel = m_leftAxisMenu->addAction("Label"); leftAxisLabel->setCheckable(true); leftAxisLabel->setData(Plot::Axis::Label);
    auto leftAxisNumbers = m_leftAxisMenu->addAction("Numbers"); leftAxisNumbers->setCheckable(true); leftAxisNumbers->setData(Plot::Axis::MajorTickLabel);
    auto leftAxisMajorTick = m_leftAxisMenu->addAction("Major Ticks"); leftAxisMajorTick->setCheckable(true); leftAxisMajorTick->setData(Plot::Axis::MajorTick);
    auto leftAxisMediumTick = m_leftAxisMenu->addAction("Medium Ticks"); leftAxisMediumTick->setCheckable(true); leftAxisMediumTick->setData(Plot::Axis::MediumTick);
    auto leftAxisMinorTick = m_leftAxisMenu->addAction("Minor Ticks"); leftAxisMinorTick->setCheckable(true); leftAxisMinorTick->setData(Plot::Axis::MinorTick);
    leftAxisLabel->setChecked(((const Plot::BoxGraphBase*)m_graphWidget)->leftAxis()->isVisible(Plot::Axis::Label));
    leftAxisNumbers->setChecked(((const Plot::BoxGraphBase*)m_graphWidget)->leftAxis()->isVisible(Plot::Axis::MajorTickLabel));
    leftAxisMajorTick->setChecked(((const Plot::BoxGraphBase*)m_graphWidget)->leftAxis()->isVisible(Plot::Axis::MajorTick));
    leftAxisMediumTick->setChecked(((const Plot::BoxGraphBase*)m_graphWidget)->leftAxis()->isVisible(Plot::Axis::MediumTick));
    leftAxisMinorTick->setChecked(((const Plot::BoxGraphBase*)m_graphWidget)->leftAxis()->isVisible(Plot::Axis::MinorTick));
    connect(leftAxisLabel, &QAction::toggled, this, &MainWindow::setLeftAxisComponentVisibility);
    connect(leftAxisNumbers, &QAction::toggled, this, &MainWindow::setLeftAxisComponentVisibility);
    connect(leftAxisMajorTick, &QAction::toggled, this, &MainWindow::setLeftAxisComponentVisibility);
    connect(leftAxisMediumTick, &QAction::toggled, this, &MainWindow::setLeftAxisComponentVisibility);
    connect(leftAxisMinorTick, &QAction::toggled, this, &MainWindow::setLeftAxisComponentVisibility);

    m_rightAxisMenu = viewMenu->addMenu(themedIcon(":/borderRight"), "Right Axis");
    auto rightAxisLabel = m_rightAxisMenu->addAction("Label"); rightAxisLabel->setCheckable(true); rightAxisLabel->setData(Plot::Axis::Label);
    auto rightAxisNumbers = m_rightAxisMenu->addAction("Numbers"); rightAxisNumbers->setCheckable(true); rightAxisNumbers->setData(Plot::Axis::MajorTickLabel);
    auto rightAxisMajorTick = m_rightAxisMenu->addAction("Major Ticks"); rightAxisMajorTick->setCheckable(true); rightAxisMajorTick->setData(Plot::Axis::MajorTick);
    auto rightAxisMediumTick = m_rightAxisMenu->addAction("Medium Ticks"); rightAxisMediumTick->setCheckable(true); rightAxisMediumTick->setData(Plot::Axis::MediumTick);
    auto rightAxisMinorTick = m_rightAxisMenu->addAction("Minor Ticks"); rightAxisMinorTick->setCheckable(true); rightAxisMinorTick->setData(Plot::Axis::MinorTick);
    rightAxisLabel->setChecked(((const Plot::BoxGraphBase*)m_graphWidget)->rightAxis()->isVisible(Plot::Axis::Label));
    rightAxisNumbers->setChecked(((const Plot::BoxGraphBase*)m_graphWidget)->rightAxis()->isVisible(Plot::Axis::MajorTickLabel));
    rightAxisMajorTick->setChecked(((const Plot::BoxGraphBase*)m_graphWidget)->rightAxis()->isVisible(Plot::Axis::MajorTick));
    rightAxisMediumTick->setChecked(((const Plot::BoxGraphBase*)m_graphWidget)->rightAxis()->isVisible(Plot::Axis::MediumTick));
    rightAxisMinorTick->setChecked(((const Plot::BoxGraphBase*)m_graphWidget)->rightAxis()->isVisible(Plot::Axis::MediumTick));
    connect(rightAxisLabel, &QAction::toggled, this, &MainWindow::setRightAxisComponentVisibility);
    connect(rightAxisNumbers, &QAction::toggled, this, &MainWindow::setRightAxisComponentVisibility);
    connect(rightAxisMajorTick, &QAction::toggled, this, &MainWindow::setRightAxisComponentVisibility);
    connect(rightAxisMediumTick, &QAction::toggled, this, &MainWindow::setRightAxisComponentVisibility);
    connect(rightAxisMinorTick, &QAction::toggled, this, &MainWindow::setRightAxisComponentVisibility);

    m_bottomAxisMenu = viewMenu->addMenu(themedIcon(":/borderBottom"), "Bottom Axis");
    auto bottomAxisLabel = m_bottomAxisMenu->addAction("Label"); bottomAxisLabel->setCheckable(true); bottomAxisLabel->setData(Plot::Axis::Label);
    auto bottomAxisNumbers = m_bottomAxisMenu->addAction("Numbers"); bottomAxisNumbers->setCheckable(true); bottomAxisNumbers->setData(Plot::Axis::MajorTickLabel);
    auto bottomAxisMajorTick = m_bottomAxisMenu->addAction("Major Ticks"); bottomAxisMajorTick->setCheckable(true); bottomAxisMajorTick->setData(Plot::Axis::MajorTick);
    auto bottomAxisMediumTick = m_bottomAxisMenu->addAction("Medium Ticks"); bottomAxisMediumTick->setCheckable(true); bottomAxisMediumTick->setData(Plot::Axis::MediumTick);
    auto bottomAxisMinorTick = m_bottomAxisMenu->addAction("Minor Ticks"); bottomAxisMinorTick->setCheckable(true); bottomAxisMinorTick->setData(Plot::Axis::MinorTick);
    bottomAxisLabel->setChecked(((const Plot::BoxGraphBase*)m_graphWidget)->bottomAxis()->isVisible(Plot::Axis::Label));
    bottomAxisNumbers->setChecked(((const Plot::BoxGraphBase*)m_graphWidget)->bottomAxis()->isVisible(Plot::Axis::MajorTickLabel));
    bottomAxisMajorTick->setChecked(((const Plot::BoxGraphBase*)m_graphWidget)->bottomAxis()->isVisible(Plot::Axis::MajorTick));
    bottomAxisMediumTick->setChecked(((const Plot::BoxGraphBase*)m_graphWidget)->bottomAxis()->isVisible(Plot::Axis::MediumTick));
    bottomAxisMinorTick->setChecked(((const Plot::BoxGraphBase*)m_graphWidget)->bottomAxis()->isVisible(Plot::Axis::MediumTick));
    connect(bottomAxisLabel, &QAction::toggled, this, &MainWindow::setBottomAxisComponentVisibility);
    connect(bottomAxisNumbers, &QAction::toggled, this, &MainWindow::setBottomAxisComponentVisibility);
    connect(bottomAxisMajorTick, &QAction::toggled, this, &MainWindow::setBottomAxisComponentVisibility);
    connect(bottomAxisMediumTick, &QAction::toggled, this, &MainWindow::setBottomAxisComponentVisibility);
    connect(bottomAxisMinorTick, &QAction::toggled, this, &MainWindow::setBottomAxisComponentVisibility);

    m_topAxisMenu = viewMenu->addMenu(themedIcon(":/borderTop"), "Top Axis");
    auto topAxisLabel = m_topAxisMenu->addAction("Label"); topAxisLabel->setCheckable(true); topAxisLabel->setData(Plot::Axis::Label);
    auto topAxisNumbers = m_topAxisMenu->addAction("Numbers"); topAxisNumbers->setCheckable(true); topAxisNumbers->setData(Plot::Axis::MajorTickLabel);
    auto topAxisMajorTick = m_topAxisMenu->addAction("Major Ticks"); topAxisMajorTick->setCheckable(true); topAxisMajorTick->setData(Plot::Axis::MajorTick);
    auto topAxisMediumTick = m_topAxisMenu->addAction("Medium Ticks"); topAxisMediumTick->setCheckable(true); topAxisMediumTick->setData(Plot::Axis::MediumTick);
    auto topAxisMinorTick = m_topAxisMenu->addAction("Minor Ticks"); topAxisMinorTick->setCheckable(true); topAxisMinorTick->setData(Plot::Axis::MinorTick);
    topAxisLabel->setChecked(((const Plot::BoxGraphBase*)m_graphWidget)->topAxis()->isVisible(Plot::Axis::Label));
    topAxisNumbers->setChecked(((const Plot::BoxGraphBase*)m_graphWidget)->topAxis()->isVisible(Plot::Axis::MajorTickLabel));
    topAxisMajorTick->setChecked(((const Plot::BoxGraphBase*)m_graphWidget)->topAxis()->isVisible(Plot::Axis::MajorTick));
    topAxisMediumTick->setChecked(((const Plot::BoxGraphBase*)m_graphWidget)->topAxis()->isVisible(Plot::Axis::MediumTick));
    topAxisMinorTick->setChecked(((const Plot::BoxGraphBase*)m_graphWidget)->topAxis()->isVisible(Plot::Axis::MediumTick));
    connect(topAxisLabel, &QAction::toggled, this, &MainWindow::setTopAxisComponentVisibility);
    connect(topAxisNumbers, &QAction::toggled, this, &MainWindow::setTopAxisComponentVisibility);
    connect(topAxisMajorTick, &QAction::toggled, this, &MainWindow::setTopAxisComponentVisibility);
    connect(topAxisMediumTick, &QAction::toggled, this, &MainWindow::setTopAxisComponentVisibility);
    connect(topAxisMinorTick, &QAction::toggled, this, &MainWindow::setTopAxisComponentVisibility);

    connect(m_paintingWidget, &PaintingWidget::startParallelWork, m_graphWidget, &DropletGraphWidget::parallelWorkStarted);
    connect(m_paintingWidget, &PaintingWidget::finishParallelWork, m_graphWidget, &DropletGraphWidget::parallelWorkFinished);
    connect(m_paintingWidget, &PaintingWidget::startParallelWork, m_sampleListWidget, &SampleListWidget::parallelWorkStarted);
    connect(m_paintingWidget, &PaintingWidget::finishParallelWork, m_sampleListWidget, &SampleListWidget::parallelWorkFinished);
    connect(m_paintingWidget, &PaintingWidget::startParallelWork, m_clusteringWidget, &ClusteringWidget::parallelWorkStarted);
    connect(m_paintingWidget, &PaintingWidget::finishParallelWork, m_clusteringWidget, &ClusteringWidget::parallelWorkFinished);
    connect(m_paintingWidget, &PaintingWidget::startParallelWork, m_assignmentWidget, &AssignmentWidget::parallelWorkStarted);
    connect(m_paintingWidget, &PaintingWidget::finishParallelWork, m_assignmentWidget, &AssignmentWidget::parallelWorkFinished);
    connect(m_paintingWidget, &PaintingWidget::startParallelWork, this, &MainWindow::parallelWorkStarted);
    connect(m_paintingWidget, &PaintingWidget::finishParallelWork, this, &MainWindow::parallelWorkFinished);

    connect(m_clusteringWidget, &ClusteringWidget::startParallelWork, m_graphWidget, &DropletGraphWidget::parallelWorkStarted);
    connect(m_clusteringWidget, &ClusteringWidget::finishParallelWork, m_graphWidget, &DropletGraphWidget::parallelWorkFinished);
    connect(m_clusteringWidget, &ClusteringWidget::startParallelWork, m_sampleListWidget, &SampleListWidget::parallelWorkStarted);
    connect(m_clusteringWidget, &ClusteringWidget::finishParallelWork, m_sampleListWidget, &SampleListWidget::parallelWorkFinished);
    connect(m_clusteringWidget, &ClusteringWidget::startParallelWork, m_paintingWidget, &PaintingWidget::parallelWorkStarted);
    connect(m_clusteringWidget, &ClusteringWidget::finishParallelWork, m_paintingWidget, &PaintingWidget::parallelWorkFinished);
    connect(m_clusteringWidget, &ClusteringWidget::startParallelWork, m_assignmentWidget, &AssignmentWidget::parallelWorkStarted);
    connect(m_clusteringWidget, &ClusteringWidget::finishParallelWork, m_assignmentWidget, &AssignmentWidget::parallelWorkFinished);
    connect(m_clusteringWidget, &ClusteringWidget::startParallelWork, this, &MainWindow::parallelWorkStarted);
    connect(m_clusteringWidget, &ClusteringWidget::finishParallelWork, this, &MainWindow::parallelWorkFinished);

    connect(m_assignmentWidget, &AssignmentWidget::startParallelWork, m_graphWidget, &DropletGraphWidget::parallelWorkStarted);
    connect(m_assignmentWidget, &AssignmentWidget::finishParallelWork, m_graphWidget, &DropletGraphWidget::parallelWorkFinished);
    connect(m_assignmentWidget, &AssignmentWidget::startParallelWork, m_sampleListWidget, &SampleListWidget::parallelWorkStarted);
    connect(m_assignmentWidget, &AssignmentWidget::finishParallelWork, m_sampleListWidget, &SampleListWidget::parallelWorkFinished);
    connect(m_assignmentWidget, &AssignmentWidget::startParallelWork, m_paintingWidget, &PaintingWidget::parallelWorkStarted);
    connect(m_assignmentWidget, &AssignmentWidget::finishParallelWork, m_paintingWidget, &PaintingWidget::parallelWorkFinished);
    connect(m_assignmentWidget, &AssignmentWidget::startParallelWork, m_clusteringWidget, &ClusteringWidget::parallelWorkStarted);
    connect(m_assignmentWidget, &AssignmentWidget::finishParallelWork, m_clusteringWidget, &ClusteringWidget::parallelWorkFinished);
    connect(m_assignmentWidget, &AssignmentWidget::startParallelWork, this, &MainWindow::parallelWorkStarted);
    connect(m_assignmentWidget, &AssignmentWidget::finishParallelWork, this, &MainWindow::parallelWorkFinished);

    connect(m_data, &Data::selectedSamplesChanged, this, &MainWindow::selectedSamplesChanged);
    connect(m_data, &Data::designChanged, this, &MainWindow::designChanged);

    connect(qGuiApp->styleHints(), &QStyleHints::colorSchemeChanged, this, &MainWindow::guiColorSchemeChanged, Qt::QueuedConnection);

    setWindowTitle("Fuzzy Droplets");
}

MainWindow::~MainWindow()
{
    delete m_data;
    delete m_commandStack;
}

void MainWindow::closeEvent(QCloseEvent * e)
{
    if (m_commandStack->canUndo()) {
        if (QMessageBox::question(this, "Save Changes?", "Export changes before closing?", QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes) == QMessageBox::Yes) {
            if (!exportAll()) {
                e->ignore();
            }
        }
    }
}

void MainWindow::designChanged()
{
    m_exportReportAction->setEnabled(m_data->design()->clusterCount() > 0);
}

void MainWindow::parallelWorkStarted()
{
    m_expDesignPushButton->setEnabled(false);
    menuBar()->setEnabled(false);
}

void MainWindow::parallelWorkFinished()
{
    m_expDesignPushButton->setEnabled(true);
    menuBar()->setEnabled(true);
}

void MainWindow::guiColorSchemeChanged()
{
    setPalette(qGuiApp->palette());
    for (auto * obj : findChildren<QWidget*>(Qt::FindChildrenRecursively)) {
        obj->setPalette(qGuiApp->palette());
    }
    m_addDataFilesAction->setIcon(themedIcon(":/file"));
    m_addFolderAction->setIcon(themedIcon(":/folder"));
    m_saveAction->setIcon(themedIcon(":/save"));
    m_exitAction->setIcon(themedIcon(":/exit"));
    m_expDesignAction->setIcon(themedIcon(":/science"));
    m_expDesignPushButton->setIcon(themedIcon(":/science"));
    m_undoAction->setIcon(themedIcon(":/undo"));
    m_redoAction->setIcon(themedIcon(":/redo"));
    m_exportReportAction->setIcon(themedIcon(":/exportReport"));
    m_topAxisMenu->setIcon(themedIcon(":/borderTop"));
    m_bottomAxisMenu->setIcon(themedIcon(":/borderBottom"));
    m_leftAxisMenu->setIcon(themedIcon(":/borderLeft"));
    m_rightAxisMenu->setIcon(themedIcon(":/borderRight"));
    m_svgSquareAction->setIcon(themedIcon(":/square"));
    m_svgRoundAction->setIcon(themedIcon(":/circle"));
    m_jpgSquareAction->setIcon(themedIcon(":/square"));
    m_jpgRoundAction->setIcon(themedIcon(":/circle"));
    m_zoomInAction->setIcon(themedIcon(":/zoomIn"));
    m_zoomOutAction->setIcon(themedIcon(":/zoomOut"));
    if (qGuiApp->styleHints()->colorScheme() == Qt::ColorScheme::Dark)
        m_data->colorScheme()->setColor(0, QColor(Qt::white).rgb());
    else
        m_data->colorScheme()->setColor(0, QColor(Qt::black).rgb());
    m_data->updateRgba();
    m_paintingWidget->updatePaletteWidgets();
    m_graphWidget->updateStaticPrimitives();
    m_graphWidget->update();
}

void MainWindow::zoomMarkersCompletely()
{
    QSettings settings;
    settings.setValue("graphWidgetScaleFactor", 1);
    m_graphWidget->setScaleFactor(1);
}

void MainWindow::zoomMarkersPartially()
{
    QSettings settings;
    settings.setValue("graphWidgetScaleFactor", 0.15);
    m_graphWidget->setScaleFactor(0.15);
}

void MainWindow::doNotZoomMarkers()
{
    QSettings settings;
    settings.setValue("graphWidgetScaleFactor", 0);
    m_graphWidget->setScaleFactor(0);
}

void MainWindow::setLeftAxisComponentVisibility(bool b)
{
    setAxisComponentVisibility("left", m_graphWidget->leftAxis(), static_cast<QAction*>(sender())->data().value<Plot::Axis::Component>(), b);
}

void MainWindow::setRightAxisComponentVisibility(bool b)
{
    setAxisComponentVisibility("right", m_graphWidget->rightAxis(), static_cast<QAction*>(sender())->data().value<Plot::Axis::Component>(), b);
}

void MainWindow::setTopAxisComponentVisibility(bool b)
{
    setAxisComponentVisibility("top", m_graphWidget->topAxis(), static_cast<QAction*>(sender())->data().value<Plot::Axis::Component>(), b);
}

void MainWindow::setBottomAxisComponentVisibility(bool b)
{
    setAxisComponentVisibility("bottom", m_graphWidget->bottomAxis(), static_cast<QAction*>(sender())->data().value<Plot::Axis::Component>(), b);
}

void MainWindow::setAxisComponentVisibility(QString name, Plot::Axis * axis, Plot::Axis::Component component, bool visible)
{
    axis->setVisibility(component, visible);
    QSettings settings;
    QString componentName;
    switch (component) {
        case Plot::Axis::Label : componentName = "Label"; break;
        case Plot::Axis::MajorTickLabel : componentName = "MajorTickLabel"; break;
        case Plot::Axis::MajorTick : componentName = "MajorTick"; break;
        case Plot::Axis::MediumTick : componentName = "MediumTick"; break;
        case Plot::Axis::MinorTick : componentName = "MinorTick"; break;
        default: break;
    }
    settings.setValue(name + "Axis" + componentName, visible);
    m_graphWidget->recalculateLayout();
    m_graphWidget->update();
}

void MainWindow::addDataFiles()
{
    QSettings settings;
    auto dir = settings.value("openFileDir", QDir::homePath()).toString();
    auto list = QFileDialog::getOpenFileNames(this, QString(), dir, "Comma Separated Values (*.csv)");
    if (list.size() > 0) {
        dir = QFileInfo(list[0]).absolutePath();
        settings.setValue("openFileDir", dir);
        std::vector<std::string> paths;
        for (auto & path : list) {
            paths.push_back(path.toStdString());
        }
        auto samplePaths = m_data->samplePaths();
        QStringList skippedFiles;
        for (int i = paths.size()-1; i >= 0; --i) {
            if (std::find(samplePaths.begin(), samplePaths.end(), paths[i]) != samplePaths.end()) {
                skippedFiles.append(QString::fromStdString(paths[i]));
                paths.erase(paths.begin() + i);
            }
        }
        if (skippedFiles.size() > 0) {
            skippedFiles.sort();
            QMessageBox::information(this, "Skipped Files", "The following files were skipped because they have already been loaded:\n    " + skippedFiles.join("\n    "));
        }
        if (paths.size() > 0) {
            auto prev = m_data->sampleCount();
            std::string error;
            m_data->addSamples(paths, error);
            if (error.size() > 0)
                processFileLoadErrorMessage(error);
            std::vector<size_t> newSamples(m_data->sampleCount() - prev, 0);
            std::iota(newSamples.begin(), newSamples.end(), prev);
            m_data->setSelectedSamples(newSamples);
        }
    }

    m_convexHullAction->setEnabled(m_data->pointCount() > 0);
    m_selectionMenu->setEnabled(m_data->pointCount() > 0);
    m_saveAction->setEnabled(m_data->pointCount() > 0);
    m_svgMenu->setEnabled(m_data->pointCount() > 0);
    m_jpgMenu->setEnabled(m_data->pointCount() > 0);
}

void MainWindow::addFolder()
{
    QSettings settings;
    auto dir = settings.value("openFileDir", QDir::homePath()).toString();
    auto folder = QFileDialog::getExistingDirectory(this, QString(), dir);
    if (folder.size() > 0) {
        QDir dir(folder);
        auto list = dir.entryInfoList(QStringList() << "*.csv", QDir::Files);
        if (list.size() > 0) {
            std::vector<std::string> paths;
            for (auto & path : list) {
                paths.push_back(path.absoluteFilePath().toStdString());
            }
            auto samplePaths = m_data->samplePaths();
            QStringList skippedFiles;
            for (int i = paths.size()-1; i >= 0; --i) {
                if (std::find(samplePaths.begin(), samplePaths.end(), paths[i]) != samplePaths.end()) {
                    skippedFiles.append(QString::fromStdString(paths[i]));
                    paths.erase(paths.begin() + i);
                }
            }
            if (skippedFiles.size() > 0) {
                skippedFiles.sort();
                QMessageBox::information(this, "Skipped Files", "The following files were skipped because they have already been loaded:\n    " + skippedFiles.join("\n    "));
            }
            if (paths.size() > 0) {
                auto prev = m_data->sampleCount();
                std::string error;
                m_data->addSamples(paths, error);
                if (error.size() > 0)
                    processFileLoadErrorMessage(error);
                settings.setValue("openFileDir", dir.absolutePath());
                std::vector<size_t> newSamples(m_data->sampleCount() - prev, 0);
                std::iota(newSamples.begin(), newSamples.end(), prev);
                m_data->setSelectedSamples(newSamples);
            }
        }
    }

    m_convexHullAction->setEnabled(m_data->pointCount() > 0);
    m_selectionMenu->setEnabled(m_data->pointCount() > 0);
    m_saveAction->setEnabled(m_data->pointCount() > 0);
    m_svgMenu->setEnabled(m_data->pointCount() > 0);
    m_jpgMenu->setEnabled(m_data->pointCount() > 0);
}

void MainWindow::processFileLoadErrorMessage(const std::string & error)
{
    QDialog d;
    d.setWindowTitle("Error Loading Data");
    QVBoxLayout * layout = new QVBoxLayout;
    QPlainTextEdit *e = new QPlainTextEdit;
    e->setPlainText(QString::fromStdString(error));
    layout->addWidget(e);
    auto buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok);
    connect(buttonBox, &QDialogButtonBox::accepted, &d, &QDialog::accept);
    layout->addWidget(buttonBox);
    d.setLayout(layout);
    d.exec();
}

void MainWindow::showExperimentalDesignDialog()
{
    if (m_splitter->indexOf(m_graphWidget) != -1) {
        m_designWidget = new ExperimentalDesignWidget(m_data, m_paintingWidget, m_commandStack, m_sampleListWidget, m_graphWidget, this);
        connect(m_designWidget, &ExperimentalDesignWidget::closed, this, &MainWindow::hideExperimentalDesignDialog);
        auto sizes = m_splitter->sizes();
        m_splitter->replaceWidget(1, m_designWidget);
        m_splitter->setSizes(sizes);
        m_sampleListWidget->setEnabled(false);
        m_shelfWidget->setEnabled(false);
        m_expDesignPushButton->setEnabled(false);
        menuBar()->setEnabled(false);
        m_designWidget->run();
    }
}

void MainWindow::hideExperimentalDesignDialog()
{
    if (m_splitter->indexOf(m_designWidget) != -1) {
        auto sizes = m_splitter->sizes();
        m_splitter->replaceWidget(1, m_graphWidget);
        m_splitter->setSizes(sizes);
        m_sampleListWidget->setEnabled(true);
        m_shelfWidget->setEnabled(true);
        m_expDesignPushButton->setEnabled(true);
        menuBar()->setEnabled(true);
        m_designWidget->deleteLater();
        m_designWidget = nullptr;
    }
}

bool MainWindow::exportAll()
{
    QSettings settings;
    auto folder = QFileDialog::getExistingDirectory(this, QString(), settings.value("openFileDir", QDir::homePath()).toString());
    if (folder.size() > 0) {
        QDir dir(folder);
        auto existing = dir.entryList(QDir::Files);
        int overwriteCount = 0;
        for (int i = 0; i < m_data->sampleCount(); ++i) {
            QFileInfo fi(QString::fromStdString(m_data->samplePath(i)));
            if (existing.contains(fi.fileName())) {
                ++overwriteCount;
            }
        }
        if (overwriteCount > 0) {
            if (QMessageBox::question(this, "", "Overwrite " + QString::number(overwriteCount) + " existing files?") == QMessageBox::No)
                return false;
        }
        for (int i = 0; i < m_data->sampleCount(); ++i) {
            QFileInfo fi(QString::fromStdString(m_data->samplePath(i)));
            QFile file(dir.absoluteFilePath(fi.fileName()));
            file.open(QFile::WriteOnly | QFile::Truncate);
            QTextStream ts(&file);
            auto indices = m_data->sampleIndices(i);
            for (size_t pos = indices[0]; pos < indices[1]; ++pos) {
                ts.setRealNumberPrecision(m_data->precision(pos).second);
                ts << m_data->point(pos).y() << ",";
                ts.setRealNumberPrecision(m_data->precision(pos).first);
                ts << m_data->point(pos).x() << "," ;
                if (m_data->colorComponentCount() < 2) {
                    ts << "0" << Qt::endl;
                } else {
                    ts.setRealNumberPrecision(6);
                    for (size_t col = 1; col < m_data->colorComponentCount(); ++col) {
                        ts << m_data->fuzzyColor(pos).weight(col);
                        if (col < m_data->colorComponentCount() - 1)
                            ts << ",";
                    }
                    ts << Qt::endl;
                }
            }
            file.close();
        }
        return true;
    }
    return false;
}

void MainWindow::exportSVGSquare()
{
    m_graphWidget->pointCloud()->setRoundSvgMarkers(false);
    exportSVG();
}

void MainWindow::exportSVGRound()
{
    m_graphWidget->pointCloud()->setRoundSvgMarkers(true);
    exportSVG();
}

void MainWindow::exportJPGSquare()
{
    m_graphWidget->pointCloud()->setRoundSvgMarkers(false);
    exportJPG();
}

void MainWindow::exportJPGRound()
{
    m_graphWidget->pointCloud()->setRoundSvgMarkers(true);
    exportJPG();
}

void MainWindow::exportSVG()
{
    QSettings settings;
    auto file = QFileDialog::getSaveFileName(this, "", settings.value("exportDir", QDir::homePath()).toString(), "SVG Files (*.svg)");
    if (!file.isEmpty()) {
        QFileInfo fi(file);
        settings.setValue("exportDir", fi.absolutePath());
        m_graphWidget->exportSvg(file.toStdString());
    }
}

void MainWindow::exportJPG()
{
    QSettings settings;
    auto output = QFileDialog::getSaveFileName(this, "", settings.value("exportDir", QDir::homePath()).toString(), "JPEG Files (*.jpg *.jpeg)");
    if (!output.isEmpty()) {
        QFileInfo fi(output);
        settings.setValue("exportDir", fi.absolutePath());
        QTemporaryFile file;
        file.open();
        file.close();
        m_graphWidget->exportSvg(file.fileName().toStdString());
        QSvgRenderer renderer(file.fileName());
        QImage img(m_graphWidget->bufferSize() * 10, QImage::Format_ARGB32);
        QPainter painter(&img);
        renderer.render(&painter);
        QImageWriter writer(output);
        writer.setQuality(100);
        writer.write(img);
    }
}

void MainWindow::exportReport()
{
    if (m_data->design()->clusterCount() == 0) {
        QMessageBox::information(this, "Cannot Generate Report", "Please set up the experimental design before generating the report.");
        return;
    }
    QSettings settings;
    auto output = QFileDialog::getSaveFileName(this, "", settings.value("exportDir", QDir::homePath()).toString(), "Comma Separated Values (*.csv)");
    if (!output.isEmpty()) {
        QFileInfo fi(output);
        settings.setValue("exportDir", fi.absolutePath());
        QFile file(output);
        file.open(QFile::WriteOnly | QFile::Truncate);
        QTextStream ts(&file);
        ts << "File,Type,Unassigned Droplets,";
        for (size_t i = 0; i < m_data->design()->clusterCount(); ++i) {
            ts << QString::fromStdString(m_data->design()->clusterLabel(i)) << ",";
        }
        ts << "Total" << Qt::endl;
        for (size_t i = 0; i < m_data->sampleCount(); ++i) {
            ts << QString::fromStdString(m_data->samplePath(i)) << ",";
            if (m_data->sampleType(i) == Data::Experimental)
                ts << "Experimental Sample,";
            else if (m_data->sampleType(i) == Data::PositiveControl)
                ts << "Positive Control,";
            else if (m_data->sampleType(i) == Data::NegativeControl)
                ts << "Negative Control,";
            else if (m_data->sampleType(i) == Data::NonTemplateControl)
                ts << "Non-Template Control,";
            std::vector<double> counts(m_data->colorComponentCount(), 0);
            for (size_t pt = m_data->sampleIndices(i)[0]; pt < m_data->sampleIndices(i)[1]; ++pt) {
                for (size_t col = 0; col < m_data->colorComponentCount(); ++col) {
                    counts[col] += m_data->fuzzyColor(pt).weight(col);
                }
            }
            for (size_t j = 0; j < counts.size(); ++j) {
                ts << counts[j] << ",";
            }
            ts << m_data->sampleSize(i) << Qt::endl;
        }
        file.close();
    }
}

void MainWindow::selectedSamplesChanged()
{
    m_paintingWidget->setEnabled(m_data->selectedSamples().size() > 0);
    m_clusteringWidget->setEnabled(m_data->selectedSamples().size() > 0);
    m_graphWidget->setEnabled(m_data->selectedSamples().size() > 0);
    m_expDesignPushButton->setEnabled(m_data->selectedSamples().size() > 0);
    m_expDesignAction->setEnabled(m_data->selectedSamples().size() > 0);
    m_zoomInAction->setEnabled(m_data->selectedSamples().size() > 0);
    m_zoomOutAction->setEnabled(m_data->selectedSamples().size() > 0);
    m_zoomResetAction->setEnabled(m_data->selectedSamples().size() > 0);
}

void MainWindow::citation()
{
    QDialog d;
    d.setWindowTitle("How to Cite");
    QVBoxLayout * layout = new QVBoxLayout;
    QTextEdit *e = new QTextEdit;

    QString citation = "<p>If you use Random Forest, please additionally cite:</p>";
    citation += "<p>Wright, M. N. & Ziegler, A. (2017). ranger: A fast implementation of random forests for high dimensional data in C++ and R. <i>Journal of Statistical Software</i> 77:1-17. https://doi.org/10.18637/jss.v077.i01.</p>";

    e->setHtml(citation);
    e->setReadOnly(true);
    layout->addWidget(e);
    auto buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok);
    connect(buttonBox, &QDialogButtonBox::accepted, &d, &QDialog::accept);
    layout->addWidget(buttonBox);
    d.setLayout(layout);
    d.exec();
}
