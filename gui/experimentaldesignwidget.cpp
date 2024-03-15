#include "experimentaldesignwidget.h"
#include "samplelistwidget.h"
#include "pointcloud.h"
#include "generic/commandstack.h"
#include "plot/continuousaxis.h"
#include "plot/marker2d.h"
#include "../core/data.h"
#include "../core/colorscheme.h"
#include "paintingwidget.h"
#include "dropletgraphwidget.h"

#include <QBoxLayout>
#include <QSpinBox>
#include <QLabel>
#include <QLineEdit>
#include <QGroupBox>
#include <QFormLayout>
#include <QRadioButton>
#include <QButtonGroup>
#include <QTimer>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QToolButton>
#include <QTableWidget>
#include <QHeaderView>
#include <QColorDialog>
#include <QMouseEvent>
#include <QFile>
#include <QFileDialog>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QSettings>
#include <QMessageBox>

#include <random>

const QString ExperimentalDesignWizard::jsonPath = "design.json.path";

ExperimentalDesignWizard::ExperimentalDesignWizard(Data * data, PaintingWidget * painting, CommandStack * cmdStack, Design * design, QWidget * parent)
    : m_data(data),
    m_painting(painting),
    m_commandStack(cmdStack),
    m_design(design)
{
    setWindowTitle("Experimental Design");
    setWindowFlag(Qt::WindowStaysOnTopHint);

    ////////////
    // PAGE 1 //
    ////////////

    page1 = new QWidget;
    auto page1Layout = new QVBoxLayout;

    QLabel * page1Label = new QLabel("Set up the ddPCR targets. The main window will show an idealized representation of the layout that you define.");
    page1Label->setWordWrap(true);
    page1Layout->addWidget(page1Label);

    QHBoxLayout * openFileLayout = new QHBoxLayout;
    openFileLayout->addStretch(1);
    QPushButton * openFileButton = new QPushButton("Load from File...");
    openFileLayout->addWidget(openFileButton);
    connect(openFileButton, &QPushButton::clicked, this, &ExperimentalDesignWizard::openFromFile);
    openFileLayout->setContentsMargins(0,0,0,0);
    page1Layout->addLayout(openFileLayout);

    QGroupBox * ch1Group = new QGroupBox("Channel 1");
    m_ch1Layout = new QFormLayout;
    m_ch1SpinBox = new QSpinBox;
    m_ch1SpinBox->setRange(0, 2);
    m_ch1SpinBox->setValue(design->ch1TargetCount());
    connect(m_ch1SpinBox, &QSpinBox::valueChanged, this, &ExperimentalDesignWizard::setCh1TargetCount);
    m_ch1Layout->addRow("Number of targets:", m_ch1SpinBox);
    ch1Group->setLayout(m_ch1Layout);
    page1Layout->addWidget(ch1Group);
    setCh1TargetCount(m_design->ch1TargetCount());

    QGroupBox * ch2Group = new QGroupBox("Channel 2");
    m_ch2Layout = new QFormLayout;
    m_ch2SpinBox = new QSpinBox;
    m_ch2SpinBox->setRange(0, 2);
    m_ch2SpinBox->setValue(design->ch2TargetCount());
    connect(m_ch2SpinBox, &QSpinBox::valueChanged, this, &ExperimentalDesignWizard::setCh2TargetCount);
    m_ch2Layout->addRow("Number of targets:", m_ch2SpinBox);
    ch2Group->setLayout(m_ch2Layout);
    page1Layout->addWidget(ch2Group);
    setCh2TargetCount(m_design->ch2TargetCount());

    auto unstaggered = new QRadioButton;
    auto staggeredCh1 = new QRadioButton;
    auto staggeredCh2 = new QRadioButton;
    QButtonGroup * staggeredBtnGroup = new QButtonGroup(this);
    staggeredBtnGroup->addButton(unstaggered, 0);
    staggeredBtnGroup->addButton(staggeredCh1, 1);
    staggeredBtnGroup->addButton(staggeredCh2, 2);
    connect(staggeredBtnGroup, &QButtonGroup::idClicked, this, &ExperimentalDesignWizard::staggeredButtonClicked);
    if (m_design->staggering() == Design::Unstaggered)
        unstaggered->setChecked(true);
    else if (m_design->staggering() == Design::StaggeredInChannel1)
        staggeredCh1->setChecked(true);
    else
        staggeredCh2->setChecked(true);
    QGroupBox * staggeredGroup = new QGroupBox("Staggering");
    QFormLayout * staggeredLayout = new QFormLayout;
    staggeredLayout->addRow("Unstaggered", unstaggered);
    staggeredLayout->addRow("Staggered in Channel 1", staggeredCh1);
    staggeredLayout->addRow("Staggered in Channel 2", staggeredCh2);
    staggeredGroup->setLayout(staggeredLayout);
    page1Layout->addWidget(staggeredGroup);

    page1Layout->setSizeConstraint(QLayout::SetFixedSize);
    page1Layout->setContentsMargins(0,0,0,0);

    page1->setLayout(page1Layout);

    ////////////
    // PAGE 2 //
    ////////////

    page2 = new QWidget;
    auto page2Layout = new QVBoxLayout;
    page2Layout->addWidget(new QLabel("Set the colors of each cluster."));
    colorListTable = new QTableWidget;
    colorListTable->setColumnCount(2);
    colorListTable->horizontalHeader()->hide();
    colorListTable->verticalHeader()->hide();
    colorListTable->horizontalHeader()->setStretchLastSection(true);
    colorListTable->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    colorListTable->setStyleSheet("QTableWidget { gridline-color: transparent; background-color: transparent }");
    colorListTable->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    page2Layout->addWidget(colorListTable);
    page2Layout->setStretch(1, 1);
    page2Layout->setContentsMargins(0,0,0,0);
    page2->setLayout(page2Layout);
    page2->hide();


    ////////////
    // PAGE 3 //
    ////////////

    page3 = new QWidget;
    auto page3Layout = new QVBoxLayout;

    auto label3 = new QLabel("Drag the markers to indicate the approximate centroids of each cluster. You can view different samples using the sample list on the left of the main window. Match by relative position, not by colour!");
    label3->setWordWrap(true);
    page3Layout->addWidget(label3);

    QHBoxLayout * saveFileLayout = new QHBoxLayout;
    saveFileLayout->addStretch(1);
    QPushButton * saveFileButton = new QPushButton("Save to File...");
    connect(saveFileButton, &QPushButton::clicked, this, &ExperimentalDesignWizard::saveToFile);
    saveFileLayout->addWidget(saveFileButton);
    saveFileLayout->setContentsMargins(0,0,0,0);
    page3Layout->addLayout(saveFileLayout);

    page3Layout->addStretch(1);

    page3->setLayout(page3Layout);
    page3->hide();





    ///////////////

    mainLayout = new QVBoxLayout;
    mainLayout->addWidget(page1, 0,  Qt::AlignHCenter);


    QHBoxLayout * nextLayout = new QHBoxLayout;
    nextLayout->setContentsMargins(0,0,0,0);
    auto cancelButton = new QPushButton("Cancel");
    nextLayout->addWidget(cancelButton);
    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);
    nextLayout->addStretch(1);
    prevButton = new QPushButton("Previous");
    prevButton->setEnabled(false);
    nextLayout->addWidget(prevButton);
    connect(prevButton, &QPushButton::clicked, this, &ExperimentalDesignWizard::prev);
    nextButton = new QPushButton("Next");
    nextLayout->addWidget(nextButton);
    connect(nextButton, &QPushButton::clicked, this, &ExperimentalDesignWizard::next);
    mainLayout->addStretch(1);
    mainLayout->addLayout(nextLayout);
    setLayout(mainLayout);
}

void ExperimentalDesignWizard::next()
{
    if (m_page == 1) {
        mainLayout->removeWidget(page1);
        mainLayout->removeItem(mainLayout->itemAt(0)); // remove the stretch in page 1
        page1->hide();
        mainLayout->insertWidget(0, page2);
        setUpPage2();
        page2->show();
        m_page = 2;
        prevButton->setEnabled(true);
    } else if (m_page == 2) {
        mainLayout->removeWidget(page2);
        page2->hide();
        mainLayout->insertWidget(0, page3);
        setUpPage3();
        page3->show();
        m_page = 3;
        nextButton->setText("Finish");
        emit startPositioningCentroids();
    } else if (m_page == 3) {
        emit stopPositioningCentroids();
        m_accepted = true;
        accept();
    }
    emit updateGraphColors();
}

void ExperimentalDesignWizard::prev()
{
    if (m_page == 2) {
        mainLayout->removeWidget(page2);
        page2->hide();
        mainLayout->insertWidget(0, page1, 0, Qt::AlignHCenter);
        mainLayout->insertStretch(1);
        page1->show();
        m_page = 1;
        prevButton->setEnabled(false);
    } else if (m_page == 3) {
        mainLayout->removeWidget(page3);
        page3->hide();
        mainLayout->insertWidget(0, page2);
        setUpPage2();
        page2->show();
        m_page = 2;
        nextButton->setText("Next");
        emit stopPositioningCentroids();
    }
    emit updateGraphColors();
}

void ExperimentalDesignWizard::setUpPage2()
{
    colorListTable->clear();
    //m_colors.clear();
    m_colorButtons.clear();
    colorListTable->setRowCount(m_design->clusterCount());

    for (int i = 0; i < m_design->clusterCount(); ++i) {

        QLabel * label = new QLabel(QString::fromStdString(m_design->clusterLabel(i)));
        QHBoxLayout * layout2 = new QHBoxLayout;
        layout2->setContentsMargins(2,2,2,2);
        layout2->addWidget(label);
        layout2->addStretch(1);
        QWidget * w2 = new QWidget;
        w2->setLayout(layout2);
        QTableWidgetItem * item = new QTableWidgetItem();
        item->setFlags(item->flags() & ~(Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsEnabled));
        colorListTable->setItem(i, 1, item);
        colorListTable->setCellWidget(i, 1, w2);

        QToolButton * btn = new QToolButton();
        m_colorButtons.push_back(btn);
        connect(btn, &QToolButton::clicked, this, &ExperimentalDesignWizard::setColor);
        btn->setStyleSheet("QToolButton {background-color:rgb(" + QString::number(m_colors[i].red()) + "," + QString::number(m_colors[i].green()) + "," + QString::number(m_colors[i].blue()) + ");}");
        btn->setCursor(Qt::CursorShape::PointingHandCursor);
        QHBoxLayout * layout = new QHBoxLayout;
        layout->setContentsMargins(2,2,2,2);
        layout->addWidget(btn);
        layout->addStretch(1);
        QWidget * w = new QWidget;
        w->setLayout(layout);
        QTableWidgetItem * item2 = new QTableWidgetItem();
        colorListTable->setItem(i, 0, item2);
        colorListTable->item(i,0)->setFlags(colorListTable->item(i,1)->flags() & ~(Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsEnabled));
        colorListTable->setCellWidget(i, 0, w);


    }
}

void ExperimentalDesignWizard::setUpPage3()
{

}

void ExperimentalDesignWizard::setCh1TargetCount(int count)
{
    m_design->setCh1TargetCount(count);
    m_colors.resize(m_design->clusterCount());
    for (int i = 0; i < m_design->clusterCount(); ++i)
        m_colors[i] = m_data->colorScheme()->color(i+1);
    while (m_ch1Layout->rowCount() > count + 1)
        m_ch1Layout->removeRow(m_ch1Layout->rowCount() - 1);

    for (int i = m_ch1Layout->rowCount() - 1; i < m_design->ch1TargetCount(); ++i) {
        auto lineEdit = new QLineEdit(QString::fromStdString(m_design->ch1TargetLabel(i)));
        connect(lineEdit, SIGNAL(textEdited(QString)), this, SLOT(updateLabels()));
        m_ch1Layout->addRow("Target " + QString::number(i + 1) + " label:", lineEdit);
    }
    emit layoutChanged();
}

void ExperimentalDesignWizard::setCh2TargetCount(int count)
{
    m_design->setCh2TargetCount(count);
    m_colors.resize(m_design->clusterCount());
    for (int i = 0; i < m_design->clusterCount(); ++i)
        m_colors[i] = m_data->colorScheme()->color(i+1);
    while (m_ch2Layout->rowCount() > count + 1)
        m_ch2Layout->removeRow(m_ch2Layout->rowCount() - 1);
    for (int i = m_ch2Layout->rowCount() - 1; i < m_design->ch2TargetCount(); ++i) {
        auto lineEdit = new QLineEdit(QString::fromStdString(m_design->ch2TargetLabel(i)));
        connect(lineEdit, SIGNAL(textEdited(QString)), this, SLOT(updateLabels()));
        m_ch2Layout->addRow("Target " + QString::number(i + 1) + " label:", lineEdit);
    }
    emit layoutChanged();
}

void ExperimentalDesignWizard::staggeredButtonClicked(int id)
{
    m_design->setStaggering((Design::Staggering)id);
    emit layoutChanged();
}

void ExperimentalDesignWizard::updateLabels()
{
    std::vector<std::string> ch1Labels;
    std::vector<std::string> ch2Labels;
    for (int i = 0; i < m_design->ch1TargetCount(); ++i) {
        QLineEdit * le = (QLineEdit*)(m_ch1Layout->itemAt(i+1, QFormLayout::FieldRole)->widget());
        ch1Labels.push_back(le->text().toStdString());
    }
    for (int i = 0; i < m_design->ch2TargetCount(); ++i) {
        QLineEdit * le = (QLineEdit*)(m_ch2Layout->itemAt(i+1, QFormLayout::FieldRole)->widget());
        ch2Labels.push_back(le->text().toStdString());
    }
    m_design->setTargetLabels(ch1Labels, ch2Labels);
    emit labelsChanged();
}


void ExperimentalDesignWizard::setColor()
{
    QWidget * w = qobject_cast<QWidget *>(sender()->parent());
    m_selectedColor = colorListTable->indexAt(w->pos()).row();
    if (m_selectedColor >= 0) {
        QColorDialog dlg;
        dlg.setWindowFlag(Qt::WindowStaysOnTopHint);
        QColor prevColor = m_colors[m_selectedColor];
        dlg.setCurrentColor(prevColor);
        connect(&dlg, &QColorDialog::currentColorChanged, this, &ExperimentalDesignWizard::currentColorChanged);
        if (dlg.exec() == QDialog::Rejected) {
            currentColorChanged(prevColor);
        } else if (m_colors[m_selectedColor] == Qt::black || m_colors[m_selectedColor] == Qt::white) {
            QMessageBox msg;
            msg.setWindowFlag(Qt::WindowStaysOnTopHint);
            msg.setText("Black and white are reserved for unassigned droplets (on light and dark desktop themes, respectively)");
            msg.exec();
            m_colors[m_selectedColor] = prevColor;
            m_colorButtons[m_selectedColor]->setStyleSheet("QToolButton {background-color:rgb(" + QString::number(prevColor.red()) + "," + QString::number(prevColor.green()) + "," + QString::number(prevColor.blue()) + ");}");
            emit updateGraphColors();
        }
    }
    m_selectedColor = -1;
}

void ExperimentalDesignWizard::currentColorChanged(const QColor & color)
{
    m_colors[m_selectedColor] = color;
    m_colorButtons[m_selectedColor]->setStyleSheet("QToolButton {background-color:rgb(" + QString::number(color.red()) + "," + QString::number(color.green()) + "," + QString::number(color.blue()) + ");}");
    emit updateGraphColors();
}

void ExperimentalDesignWizard::openFromFile()
{
    bool closeImmediately = false;

    QSettings settings;

    auto path = QFileDialog::getOpenFileName(0, "Open File", settings.value(jsonPath, QVariant::fromValue<QString>(QDir::homePath())).toString(), "JSON (*.json)");
    if (!path.isEmpty()) {
        QFile file(path);
        QFileInfo fi(file);
        settings.setValue(jsonPath,fi.absoluteDir().path());
        file.open(QFile::ReadOnly);
        auto ba = file.readAll();
        QJsonDocument doc = QJsonDocument::fromJson(ba);
        if (!doc.isNull() && !doc.isEmpty()) {
            QJsonObject obj = doc.object();
            QString staggering = obj.value("staggering").toString();
            if (!staggering.isEmpty()) {
                m_design->setStaggering(staggering == "unstaggered" ? Design::Unstaggered : (staggering == "staggered1" ? Design::StaggeredInChannel1 : Design::StaggeredInChannel2));
            }
            auto ch1Labels = obj.value("ch1TargetLabels").toArray();
            if (ch1Labels.size() > 0) {
                std::vector<std::string> labels;
                for (auto it : ch1Labels)
                    labels.push_back(it.toString().toStdString());
                m_design->setCh1TargetLabels(labels);
            }
            auto ch2Labels = obj.value("ch2TargetLabels").toArray();
            if (ch2Labels.size() > 0) {
                std::vector<std::string> labels;
                for (auto it : ch2Labels)
                    labels.push_back(it.toString().toStdString());
                m_design->setCh2TargetLabels(labels);
            }
            QList<QColor> colors;
            std::vector<Point> points;
            auto clusters = obj.value("clusters").toArray();
            for (auto it : clusters) {
                auto cobj = it.toObject();
                double x = cobj.value("x").toDouble();
                double y = cobj.value("y").toDouble();
                int r = cobj.value("r").toInt();
                int g = cobj.value("g").toInt();
                int b = cobj.value("b").toInt();
                points.push_back(Point(x,y));
                colors.push_back(QColor(r,g,b));
            }

            if (!closeImmediately) { //todo need to check validity
                setCh1TargetCount(m_design->ch1TargetCount());
                setCh2TargetCount(m_design->ch2TargetCount());
                for (int i = 0; i < m_design->ch1TargetCount(); ++i) {
                    QLineEdit * le = (QLineEdit*)(m_ch1Layout->itemAt(i+1, QFormLayout::FieldRole)->widget());
                    le->setText(QString::fromStdString(m_design->ch1TargetLabel(i)));
                }
                for (int i = 0; i < m_design->ch2TargetCount(); ++i) {
                    QLineEdit * le = (QLineEdit*)(m_ch2Layout->itemAt(i+1, QFormLayout::FieldRole)->widget());
                    le->setText(QString::fromStdString(m_design->ch2TargetLabel(i)));
                }
            }

            m_colors = colors;
            m_design->setClusterCentroids(points);

            if (closeImmediately) {
                if (m_design->isValid() && m_colors.size() == m_design->clusterCount()) {
                    m_accepted = true;
                    accept();
                } else {
                    qDebug("bad file"); // todo
                }
            }
        }


    }
}

void ExperimentalDesignWizard::saveToFile()
{
    QSettings settings;
    auto path = QFileDialog::getSaveFileName(0, "Save File", settings.value(jsonPath, QVariant::fromValue<QString>(QDir::homePath())).toString(), "JSON (*.json)");
    if (!path.isEmpty()) {
        QFile file(path);
        QFileInfo fi(file);
        settings.setValue(jsonPath,fi.absoluteDir().path());
        file.open(QFile::ReadWrite | QFile::Truncate);
        QTextStream ts(&file);
        ts << "{\n";
        QString staggerString = (m_design->staggering() == Design::Unstaggered) ? "unstaggered" : (m_design->staggering() == Design::StaggeredInChannel1 ? "staggered1" : "staggered2");
        ts << "    \"staggering\": \"" + staggerString + "\",\n";
        ts << "    \"ch1TargetLabels\": [\n";
        for (int i = 0; i < m_design->ch1TargetCount(); ++i) {
            auto label = QString::fromStdString(m_design->ch1TargetLabel(i));
            label.replace("\"", "\\\"");
            ts << "        \"" << label << "\"" << (i < m_design->ch1TargetCount() - 1 ? ",\n" : "\n");
        }
        ts << "    ],\n";
        ts << "    \"ch2TargetLabels\": [\n";
        for (int i = 0; i < m_design->ch2TargetCount(); ++i) {
            auto label = QString::fromStdString(m_design->ch2TargetLabel(i));
            label.replace("\"", "\\\"");
            ts << "        \"" << label << "\"" << (i < m_design->ch2TargetCount() - 1 ? ",\n" : "\n");
        }
        ts << "    ],\n";
        ts << "    \"clusters\": [\n";
        for (int i = 0; i < m_design->clusterCount(); ++i) {
            ts << "        {\n";
            ts << "            \"x\": " << m_design->clusterCentroid(i).x() << ",\n";
            ts << "            \"y\": " << m_design->clusterCentroid(i).y() << ",\n";
            ts << "            \"r\": " << m_colors[i].red() << ",\n";
            ts << "            \"g\": " << m_colors[i].green() << ",\n";
            ts << "            \"b\": " << m_colors[i].blue() << "\n";
            ts << "        }" << (i < m_design->clusterCount() - 1 ? ",\n" : "\n");
        }
        ts << "    ]\n";
        ts << "}";
        file.close();
    }
}




























ExperimentalDesignWidget::ExperimentalDesignWidget(Data * data, PaintingWidget * painting, CommandStack * cmdStack, SampleListWidget * list, DropletGraphWidget * graph, QWidget * parent)
    : Plot::ScatterPlotBox2(parent),
    m_list(list),
    m_data(data),
    m_painting(painting),
    m_commandStack(cmdStack),
    m_graph(graph)
{
    setMouseTracking(true);
    if (data->design()->isValid())
        m_design = *(data->design());
    else
        m_design.setTargetCounts(1,1);
    setupFromDataRange(m_data->bounds());


    leftAxis()->setLabel(m_graph->leftAxis()->label());
    leftAxis()->setVisibility(Plot::Axis::Component::Label, m_graph->leftAxis()->isVisible(Plot::Axis::Component::Label));
    leftAxis()->setVisibility(Plot::Axis::Component::MajorTickLabel, m_graph->leftAxis()->isVisible(Plot::Axis::Component::MajorTickLabel));
    leftAxis()->setVisibility(Plot::Axis::Component::MajorTick, m_graph->leftAxis()->isVisible(Plot::Axis::Component::MajorTick));
    leftAxis()->setVisibility(Plot::Axis::Component::MediumTick, m_graph->leftAxis()->isVisible(Plot::Axis::Component::MediumTick));
    leftAxis()->setVisibility(Plot::Axis::Component::MinorTick, m_graph->leftAxis()->isVisible(Plot::Axis::Component::MinorTick));

    bottomAxis()->setLabel(m_graph->leftAxis()->label());
    bottomAxis()->setVisibility(Plot::Axis::Component::Label, m_graph->bottomAxis()->isVisible(Plot::Axis::Component::Label));
    bottomAxis()->setVisibility(Plot::Axis::Component::MajorTickLabel, m_graph->bottomAxis()->isVisible(Plot::Axis::Component::MajorTickLabel));
    bottomAxis()->setVisibility(Plot::Axis::Component::MajorTick, m_graph->bottomAxis()->isVisible(Plot::Axis::Component::MajorTick));
    bottomAxis()->setVisibility(Plot::Axis::Component::MediumTick, m_graph->bottomAxis()->isVisible(Plot::Axis::Component::MediumTick));
    bottomAxis()->setVisibility(Plot::Axis::Component::MinorTick, m_graph->bottomAxis()->isVisible(Plot::Axis::Component::MinorTick));

    topAxis()->setLabel(m_graph->topAxis()->label());
    topAxis()->setVisibility(Plot::Axis::Component::Label, m_graph->topAxis()->isVisible(Plot::Axis::Component::Label));
    topAxis()->setVisibility(Plot::Axis::Component::MajorTickLabel, m_graph->topAxis()->isVisible(Plot::Axis::Component::MajorTickLabel));
    topAxis()->setVisibility(Plot::Axis::Component::MajorTick, m_graph->topAxis()->isVisible(Plot::Axis::Component::MajorTick));
    topAxis()->setVisibility(Plot::Axis::Component::MediumTick, m_graph->topAxis()->isVisible(Plot::Axis::Component::MediumTick));
    topAxis()->setVisibility(Plot::Axis::Component::MinorTick, m_graph->topAxis()->isVisible(Plot::Axis::Component::MinorTick));

    rightAxis()->setLabel(m_graph->rightAxis()->label());
    rightAxis()->setVisibility(Plot::Axis::Component::Label, m_graph->rightAxis()->isVisible(Plot::Axis::Component::Label));
    rightAxis()->setVisibility(Plot::Axis::Component::MajorTickLabel, m_graph->rightAxis()->isVisible(Plot::Axis::Component::MajorTickLabel));
    rightAxis()->setVisibility(Plot::Axis::Component::MajorTick, m_graph->rightAxis()->isVisible(Plot::Axis::Component::MajorTick));
    rightAxis()->setVisibility(Plot::Axis::Component::MediumTick, m_graph->rightAxis()->isVisible(Plot::Axis::Component::MediumTick));
    rightAxis()->setVisibility(Plot::Axis::Component::MinorTick, m_graph->rightAxis()->isVisible(Plot::Axis::Component::MinorTick));

//    setZoomPanEnabled(false);
    m_wizard = new ExperimentalDesignWizard(data, painting, cmdStack, &m_design, this);
    connect(m_wizard, &ExperimentalDesignWizard::layoutChanged, this, &ExperimentalDesignWidget::layoutChanged);
    connect(m_wizard, &ExperimentalDesignWizard::finished, this, &ExperimentalDesignWidget::wizardClosed);
    connect(m_wizard, &ExperimentalDesignWizard::labelsChanged, this, &ExperimentalDesignWidget::labelsChanged);
    connect(m_wizard, &ExperimentalDesignWizard::updateGraphColors, this, &ExperimentalDesignWidget::updateGraphColors);
    connect(m_wizard, &ExperimentalDesignWizard::startPositioningCentroids, this, &ExperimentalDesignWidget::startPositioningCentroids);
    connect(m_wizard, &ExperimentalDesignWizard::stopPositioningCentroids, this, &ExperimentalDesignWidget::stopPositioningCentroids);
    connect(m_data, &Data::selectedSamplesChanged, this, &ExperimentalDesignWidget::selectedSamplesChanged);
}

void ExperimentalDesignWidget::run()
{
    m_wizard->move(parentWidget()->mapToGlobal(parentWidget()->pos()).x(), parentWidget()->mapToGlobal(QPoint(0,0)).y());
    m_wizard->show();
    QTimer::singleShot(0, this, SLOT(init()));
}

void ExperimentalDesignWidget::init()
{
    layoutChanged();
}




ExperimentalDesignWizard::SetDesignCommand::SetDesignCommand(Data * data, PaintingWidget * painting, Design newDesign, QList<QColor> newColors)
    : m_data(data),
    m_painting(painting),
    m_newDesign(newDesign),
    m_oldDesign(*(data->design())),
    m_newColors(newColors)
{
    for (size_t i = 1; i < m_data->colorComponentCount(); ++i) {
        m_oldColors.push_back(m_data->colorScheme()->color(i));
    }
    m_oldFuzzies = m_data->fuzzyColors();
}

void ExperimentalDesignWizard::SetDesignCommand::redo()
{
    for (int i = 0; i < m_newDesign.clusterCount(); ++i) {
        m_data->colorScheme()->setColor(i+1, m_newColors[i].rgb());
    }
    m_data->setDesign(m_newDesign);
    m_painting->beginPaintOperation();
}

void ExperimentalDesignWizard::SetDesignCommand::undo()
{
    for (int i = 0; i < m_oldColors.size(); ++i) {
        m_data->colorScheme()->setColor(i+1, m_oldColors[i].rgb());
    }
    m_data->setDesign(m_oldDesign);
    m_data->setColorComponentCount(m_oldColors.size()+1);
    for (size_t i = 0; i < m_data->pointCount(); ++i) {
        m_data->setColor(i, m_oldFuzzies[i]);
    }
    m_painting->beginPaintOperation();
    emit m_data->fullRepaint();
}

void ExperimentalDesignWidget::wizardClosed()
{
    if (m_wizard->accepted()) {
        m_commandStack->add(new ExperimentalDesignWizard::SetDesignCommand(m_data, m_painting, m_design, m_wizard->colors()));
    }
    emit closed();
}

void ExperimentalDesignWidget::layoutChanged()
{
    m_design.initCentroids(m_data->bounds());

    for (auto & list : m_markers) {
        for (auto * marker : list) {
            removeDynamicPrimitive(marker);
            delete marker;
        }
    }
    for (auto * textMarker : m_textMarkers) {
        removeDynamicPrimitive(textMarker);
        delete textMarker;
    }
    m_markers.clear();
    m_textMarkers.clear();
    m_markers.resize(m_design.clusterCount());
    m_textMarkers.resize((m_design.clusterCount()));
    m_markersCentres.resize(m_markers.size());

    std::random_device seed;
    std::mt19937 rng(seed());
    std::normal_distribution<double> xnorm(0, m_data->bounds().width() / 30);
    std::normal_distribution<double> ynorm(0, m_data->bounds().height() / 30);
    for (int i = 0; i < m_design.clusterCount(); ++i) {
        auto & list = m_markers[i];
        for (int j = 0; j < 400; ++j) {
            list.append(new Plot::Marker2D(Plot::Marker::Circle, horizontalAxis(), verticalAxis(), m_design.clusterCentroid(i).x() + xnorm(rng), m_design.clusterCentroid(i).y() + ynorm(rng)));
            list.back()->setSize(5);
            list.back()->setAutoColor(false);
            addDynamicPrimitive(list.back());
        }
        m_markersCentres[i] = m_design.clusterCentroid(i);
        m_textMarkers[i] = new Plot::TextMarker(horizontalAxis(), verticalAxis(), m_design.clusterCentroid(i).x(), m_design.clusterCentroid(i).y() , QString::fromStdString(m_design.clusterLabel(i)));
        addDynamicPrimitive(m_textMarkers[i]);
    }

    update();
}

void ExperimentalDesignWidget::labelsChanged()
{
    auto labels = m_design.clusterLabels();
    for (int i = 0; i < m_textMarkers.size(); ++i)
        m_textMarkers[i]->setText(QString::fromStdString(labels[i]));
    update();
}

void ExperimentalDesignWidget::updateGraphColors()
{
    if (m_wizard->page() == 2) {
        for (int i = 0; i < m_markers.size(); ++i) {
            for (auto * marker : m_markers[i]) {
                marker->setVisibility(true);
                marker->setBrush(m_wizard->color(i));
            }
        }
    } else if (m_wizard->page() == 1) {
        for (int i = 0; i < m_markers.size(); ++i) {
            for (auto * marker : m_markers[i]) {
                marker->setBrush(Qt::black);
            }
        }
    } else if (m_wizard->page() == 3) {
        for (int i = 0; i < m_markers.size(); ++i) {
            for (auto * marker : m_markers[i]) {
                marker->setVisibility(false);
            }
        }
    }
    update();
}

void ExperimentalDesignWidget::startPositioningCentroids()
{
    m_positioningCentroids = true;
    m_list->setEnabled(true);
    m_pointCloud = new PointCloud(m_data, horizontalAxis(), verticalAxis());
    m_pointCloud->setBaseSize(m_graph->pointCloud()->baseSize());
    m_pointCloud->setQuadTree(m_data->quadTree());
    addStaticPrimitive(m_pointCloud);
    if (m_data->design()->clusterCount() == m_design.clusterCount()) {
        m_design.setClusterCentroids(m_data->design()->clusterCentroids());
    }
    for (int i = 0; i < m_design.clusterCount(); ++i) {
        m_centroidMarkers.push_back(new Plot::RingMarker(horizontalAxis(), verticalAxis(), m_design.clusterCentroid(i).x(), m_design.clusterCentroid(i).y()));
        m_centroidMarkers.back()->setAutoColor(false);
        auto pen = m_centroidMarkers.back()->pen();
        pen.setColor(m_wizard->color(i));
        m_centroidMarkers.back()->setPen(pen);
        addDynamicPrimitive(m_centroidMarkers.back());
        m_textMarkers[i]->setValue(m_centroidMarkers[i]->x(), m_centroidMarkers[i]->y() + m_centroidMarkers[i]->size() * (((Plot::ContinuousAxis*)leftAxis())->valueLength() / leftAxis()->pixelLength()));

    }


    updateStaticPrimitives();
    update();
}

void ExperimentalDesignWidget::stopPositioningCentroids()
{
    m_positioningCentroids = false;
    m_list->setEnabled(false);
    for (int i = 0; i < m_design.clusterCount(); ++i) {
        m_design.setClusterCentroid(i, Point(m_centroidMarkers[i]->x(), m_centroidMarkers[i]->y()));
        removeDynamicPrimitive(m_centroidMarkers[i]);
        delete m_centroidMarkers[i];
        m_textMarkers[i]->setValue(m_markersCentres[i].x(), m_markersCentres[i].y());
    }
    m_centroidMarkers.clear();
    removeStaticPrimitive(m_pointCloud);
    delete m_pointCloud;
    m_pointCloud = nullptr;
    updateStaticPrimitives();
    update();
}

void ExperimentalDesignWidget::selectedSamplesChanged()
{
    updateStaticPrimitives();
    update();
}

void ExperimentalDesignWidget::mouseMoveEvent(QMouseEvent * me)
{
    auto viewport = viewportRect();
    if (m_dragging) {
        m_centroidMarkers[m_selectedCentroid]->setValue(horizontalAxis()->value(me->pos().x() - viewport.left()) + m_dragOffset.x(), verticalAxis()->value(me->pos().y() - viewport.top()) + m_dragOffset.y());
        m_design.setClusterCentroid(m_selectedCentroid, Point(m_centroidMarkers[m_selectedCentroid]->x(),  m_centroidMarkers[m_selectedCentroid]->y()));
        m_textMarkers[m_selectedCentroid]->setValue(m_centroidMarkers[m_selectedCentroid]->x(), m_centroidMarkers[m_selectedCentroid]->y() + m_centroidMarkers[m_selectedCentroid]->size() * (((Plot::ContinuousAxis*)leftAxis())->valueLength() / leftAxis()->pixelLength()));
        update();
    } else {
        m_selectedCentroid = -1;
        for (int i = 0; i < m_centroidMarkers.size(); ++i) {
            Point p(viewport.left() + horizontalAxis()->pixel(m_centroidMarkers[i]->x()), viewport.top() + verticalAxis()->pixel(m_centroidMarkers[i]->y()));
            double D = p.distanceTo(Point(me->pos().x(), me->pos().y()));
            if (D < m_centroidMarkers[i]->size() / 2) {
                m_selectedCentroid = i;
                break;
            }
        }
        setCursor((m_selectedCentroid >= 0) ? Qt::PointingHandCursor : Qt::ArrowCursor);
    }
}

void ExperimentalDesignWidget::mousePressEvent(QMouseEvent * e)
{
    if (m_selectedCentroid >= 0) {
        QMouseEvent * me = (QMouseEvent*)e;
        setCursor(Qt::ClosedHandCursor);
        auto viewport = viewportRect();
        Point p(horizontalAxis()->value(me->position().x() - viewport.left()), verticalAxis()->value(me->position().y() - viewport.top()));
        m_dragOffset = Point(m_centroidMarkers[m_selectedCentroid]->x(), m_centroidMarkers[m_selectedCentroid]->y()) - p;
        m_dragging = true;
    }
}

void ExperimentalDesignWidget::mouseReleaseEvent(QMouseEvent * e)
{
    if (m_dragging) {
        setCursor(Qt::PointingHandCursor);
        m_dragging = false;
    }
}
