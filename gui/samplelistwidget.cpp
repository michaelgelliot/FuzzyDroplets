#include "samplelistwidget.h"
#include <QTableWidget>
#include <QBoxLayout>
#include <QGuiApplication>
#include <QStyleHints>
#include <QHeaderView>
#include <QFileInfo>
#include <QScrollBar>
#include <QMenu>
#include "generic/comboboxitemdelegate.h"
#include "generic/nofocusitemdelegate.h"
#include "generic/commandstack.h"
#include <QTemporaryFile>
#include <QTextStream>
#include <execution>

void SampleListWidget::SetSampleTypeCommand::redo()
{
    m_data->setSampleType(m_samples, m_newType);
}

void SampleListWidget::SetSampleTypeCommand::undo()
{
    m_data->setSampleType(m_samples, m_oldTypes);
}

SampleListWidget::SampleListWidget(Data * data, CommandStack * commandStack, QWidget * parent)
    : QWidget(parent),
    m_data(data),
    m_commandStack(commandStack)
{
    // init some internal data that tells what text to use to describe different data types
    m_dataTypeToLabel = {{Data::Experimental, "Experimental Sample"},
                         {Data::PositiveControl,    "Positive Control"},
                         {Data::NegativeControl,    "Negative Control"},
                         {Data::NonTemplateControl, "Non-Template"},
                         {Data::UnambiguousSample, "Unambiguous Sample"}};

    // create a table widget and make it fill this widget
    m_tableWidget = new QTableWidget(this);
    m_tableWidget->viewport()->setBackgroundRole(QPalette::Window);
    QVBoxLayout * layout = new QVBoxLayout;
    layout->addWidget(m_tableWidget);
    layout->setContentsMargins(0,0,0,0);
    setLayout(layout);

    // set up the the table widget main options
    m_tableWidget->verticalHeader()->hide();                                    // we do not need a vertical header
    m_tableWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);      // we want to be able to select any combination of samples at once
    m_tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);         // we select an entire row at a time, not individual cells of the table
    m_tableWidget->setItemDelegate(new NoFocusDelegate());                      // we do not draw a focus box on the individual cell that has been clicked (because we select rows - Qt should really do this by itself)
    m_tableWidget->setContextMenuPolicy(Qt::CustomContextMenu);                 // right click on the table and you get a context menu
    m_tableWidget->setSortingEnabled(true);                                     // we allow the user to sort the columns by clicking on the horizontal header
    m_tableWidget->sortByColumn(0, Qt::SortOrder::AscendingOrder);

    // set up the rows
    m_tableWidget->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    // set up the columns
    m_tableWidget->setColumnCount(3);                                                                   // we have three columns
    m_tableWidget->setHorizontalHeaderLabels({"Sample", "Type", "N"});                                  // here are the names of the columns
    m_tableWidget->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);                   // want the first column to stretch to fill any extra space (as it is likely the longest column)
    m_tableWidget->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);          // the second column has a fixed size, whatever is required to fit the combo box
    m_tableWidget->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);          // make the third column as small as possible compatible with its contents

    // set up the special column 1 which uses a combobox for editing
    auto delegate = new ComboBoxItemDelegate({m_dataTypeToLabel[Data::Experimental],
                                              m_dataTypeToLabel[Data::PositiveControl],
                                              m_dataTypeToLabel[Data::NegativeControl],
                                              m_dataTypeToLabel[Data::NonTemplateControl],
                                              m_dataTypeToLabel[Data::UnambiguousSample]}, false);
    m_tableWidget->setItemDelegateForColumn(1, delegate);

    // set up connections between signals from the table widget and slots in this object
    connect(m_tableWidget, &QTableWidget::itemSelectionChanged, this, &SampleListWidget::itemSelectionChanged);       // the user has changed the selection -> call SampleList::itemSelectionChanged
    connect(m_tableWidget, &QTableWidget::cellChanged, this, &SampleListWidget::cellChanged);                         // the user has modified data in one of the cells -> call SampleList::cellChanged
    connect(m_tableWidget, &QTableWidget::customContextMenuRequested, this, &SampleListWidget::showContextMenu);

    // update the list when the theme changes
    connect(qGuiApp->styleHints(), &QStyleHints::colorSchemeChanged, this, &SampleListWidget::guiColorSchemeChanged, Qt::QueuedConnection);

    // set up connections between the data and the table
    connect(data, &Data::samplesAdded, this, &SampleListWidget::samplesAdded);
    connect(data, &Data::selectedSamplesChanged, this, &SampleListWidget::selectedSamplesChanged);
    connect(data, &Data::sampleTypesChanged, this, &SampleListWidget::sampleTypesChanged);
}

SampleListWidget::~SampleListWidget()
{
}

void SampleListWidget::guiColorSchemeChanged()
{
    m_tableWidget->setPalette(qGuiApp->palette());
    m_tableWidget->horizontalHeader()->setPalette(qGuiApp->palette());
    m_tableWidget->horizontalScrollBar()->setPalette(qGuiApp->palette());
    m_tableWidget->verticalScrollBar()->setPalette(qGuiApp->palette());
    for (int i = 0; i < m_tableWidget->rowCount(); ++i) {
        for (int j = 0; j < m_tableWidget->columnCount(); ++j) {
            m_tableWidget->item(i, j)->setBackground(qGuiApp->palette().color(QPalette::Base));
        }
    }
}

size_t SampleListWidget::sampleFromRow(int i) const
{
    return (i >= 0 && i < m_tableWidget->rowCount()) ? m_tableWidget->item(i, 0)->data(Qt::UserRole).template value<size_t>() : (size_t)-1;
}

int SampleListWidget::rowFromSample(size_t i) const // todo, seems a slow way of doing things
{
    for (int row = 0; row < m_tableWidget->rowCount(); ++row) {
        if (m_tableWidget->item(row, 0)->data(Qt::UserRole).template value<int>() == i)
            return row;
    }
    return -1;
}

QList<int> SampleListWidget::sortedSelectedRows() const
{
   auto selectedItems = m_tableWidget->selectedItems();
   QList<int> rows, uniqueRows;
   std::transform(selectedItems.begin(), selectedItems.end(), std::back_inserter(rows), [](const QTableWidgetItem * item)->int{return item->row();});
   std::unique_copy(rows.begin(), rows.end(), std::back_inserter(uniqueRows));
   std::sort(uniqueRows.begin(), uniqueRows.end());
   return uniqueRows;
}

void SampleListWidget::itemSelectionChanged()
{
    if (m_reportSelectionChanged) {
        std::vector<size_t> samples;
        auto indices = m_tableWidget->selectionModel()->selectedRows();
        std::transform(indices.cbegin(), indices.cend(), std::back_inserter(samples), [&](auto & index) {return m_tableWidget->item(index.row(), 0)->data(Qt::UserRole).toInt();});
        m_data->setSelectedSamples(samples);
    }
}

void SampleListWidget::cellChanged(int row, int column)
{
    if (column == 1 && m_reportSampleTypeChanged) {
        auto sample = sampleFromRow(row);
        auto text = m_tableWidget->item(row, column)->text();
        SetSampleTypeCommand * cmd = new SetSampleTypeCommand(m_data, {sample}, {m_data->sampleType(sample)}, m_dataTypeToLabel.key(text));
        m_commandStack->add(cmd);
    }
}

void SampleListWidget::sampleTypesChanged(std::vector<size_t> samples)
{
    m_reportSampleTypeChanged = false;
    for (auto sample : samples) {
        auto row = rowFromSample(sample);
        m_tableWidget->item(row, 1)->setText(m_dataTypeToLabel[m_data->sampleType(sample)]);
    }
    m_reportSampleTypeChanged = true;
}

void SampleListWidget::samplesAdded(std::vector<size_t> samples)
{
    // do not sort the table or emit signals while new data is being added
    auto sortOrder = m_tableWidget->horizontalHeader()->sortIndicatorOrder();
    auto sortSection = m_tableWidget->horizontalHeader()->sortIndicatorSection();
    if (sortSection == m_tableWidget->columnCount()) sortSection = 0;
    m_tableWidget->setSortingEnabled(false);
    disconnect(m_tableWidget, &QTableWidget::cellChanged, this, &SampleListWidget::cellChanged); // do not emit signals related to cells changing while initializing new row

    for (auto sample : samples) {

        // update the item data
        for (size_t j = 0; j < m_tableWidget->rowCount(); ++j) {
            if (m_tableWidget->item((int)j, 0)->data(Qt::UserRole).toInt() >= sample) {
                m_tableWidget->item((int)j, 0)->setData(Qt::UserRole, m_tableWidget->item((int)j, 0)->data(Qt::UserRole).toInt() + 1);
            }
        }

        int row = m_tableWidget->rowCount();
        m_tableWidget->setRowCount(row + 1);

        QFileInfo fi(QString::fromStdString(m_data->samplePath(sample)));
        m_tableWidget->setItem(row, 0, new QTableWidgetItem(fi.fileName()));
        m_tableWidget->item(row, 0)->setTextAlignment(Qt::AlignLeft);
        m_tableWidget->item(row, 0)->setBackground(palette().color(QPalette::Base));
        m_tableWidget->setItem(row, 1, new QTableWidgetItem(m_dataTypeToLabel[m_data->sampleType(sample)]));
        m_tableWidget->setItem(row, 2, new QTableWidgetItem(QString::number(m_data->sampleSize(sample))));

        for (int i = 1; i < m_tableWidget->columnCount(); ++i) {
            m_tableWidget->item(row, i)->setBackground(palette().color(QPalette::Base));
            m_tableWidget->item(row, i)->setTextAlignment(Qt::AlignCenter);
        }

        // store extra data in the 0th column
        m_tableWidget->item(row, 0)->setData(Qt::UserRole, QVariant::fromValue(sample));
        m_tableWidget->item(row, 0)->setToolTip(QString::fromStdString(m_data->samplePath(sample)));

        // set most columns not editable
        m_tableWidget->item(row, 0)->setFlags(m_tableWidget->item(row, 0)->flags() ^ Qt::ItemIsEditable);
        m_tableWidget->item(row, 2)->setFlags(m_tableWidget->item(row, 2)->flags() ^ Qt::ItemIsEditable);
    }

    // resume sorting and emitting signals
    connect(m_tableWidget, &QTableWidget::cellChanged, this, &SampleListWidget::cellChanged);
    m_tableWidget->setSortingEnabled(true);
    m_tableWidget->sortByColumn(sortSection, sortOrder);
}

void SampleListWidget::showContextMenu(const QPoint& pos)
{
    QTableWidgetItem * item = m_tableWidget->itemAt(pos);
    if (item) {
        QMenu menu;
        QMenu * setDataTypeMenu = menu.addMenu("Set Data Type");
        menu.addSeparator();
        QList<QAction*> dataTypeActions;
        auto keys = m_dataTypeToLabel.keys();
        for (auto i : keys) {
            dataTypeActions.append(setDataTypeMenu->addAction(m_dataTypeToLabel[i]));
            dataTypeActions.back()->setData(i);
        }

        auto a = menu.exec(m_tableWidget->viewport()->mapToGlobal(pos));

        if (dataTypeActions.contains(a)) {
            QList<int> uniqueRows = sortedSelectedRows();
            auto type = a->data().value<Data::SampleType>();
            std::vector<size_t> s;
            std::vector<Data::SampleType> oldTypes;
            for (auto i : uniqueRows) {
                auto sample = sampleFromRow(i);
                s.push_back(sample);
                oldTypes.push_back(m_data->sampleType(sample));
            }
            m_reportSampleTypeChanged = false;
            SetSampleTypeCommand * cmd = new SetSampleTypeCommand(m_data, s, oldTypes, type);
            m_commandStack->add(cmd);
            m_reportSampleTypeChanged = true;
        }
    }
}

void SampleListWidget::parallelWorkStarted()
{
    setEnabled(false);
}

void SampleListWidget::parallelWorkFinished()
{
    setEnabled(true);
}

void SampleListWidget::selectedSamplesChanged()
{
   auto indices = m_tableWidget->selectionModel()->selectedRows();
   std::vector<size_t> samples(indices.size());
   std::transform(indices.cbegin(), indices.cend(), samples.begin(), [&](auto & index) {return m_tableWidget->item(index.row(), 0)->data(Qt::UserRole).toInt();});

   if (samples != m_data->selectedSamples()) {
       m_reportSelectionChanged = false;
       m_tableWidget->clearSelection();
       m_tableWidget->setSelectionMode(QAbstractItemView::MultiSelection);
       for (auto sample : m_data->selectedSamples())
           m_tableWidget->selectRow(rowFromSample(sample));
       m_tableWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);
       m_reportSelectionChanged = true;
   }
}

void SampleListWidget::selectAll()
{
    std::vector<size_t> vec(m_data->sampleCount(), 0);
    std::iota(vec.begin(), vec.end(), 0);
    m_data->setSelectedSamples(vec);
}

void SampleListWidget::selectExperimentalSamples()
{
    std::vector<size_t> vec;
    for (size_t i = 0; i < m_data->sampleCount(); ++i)
        if (m_data->sampleType(i) == Data::Experimental)
            vec.push_back(i);
    m_data->setSelectedSamples(vec);
}

void SampleListWidget::selectPositiveControls()
{
    std::vector<size_t> vec;
    for (size_t i = 0; i < m_data->sampleCount(); ++i)
        if (m_data->sampleType(i) == Data::PositiveControl)
            vec.push_back(i);
    m_data->setSelectedSamples(vec);
}

void SampleListWidget::selectNegativeControls()
{
    std::vector<size_t> vec;
    for (size_t i = 0; i < m_data->sampleCount(); ++i)
        if (m_data->sampleType(i) == Data::NegativeControl)
            vec.push_back(i);
    m_data->setSelectedSamples(vec);
}

void SampleListWidget::selectNonTemplateControls()
{
    std::vector<size_t> vec;
    for (size_t i = 0; i < m_data->sampleCount(); ++i)
        if (m_data->sampleType(i) == Data::NonTemplateControl)
            vec.push_back(i);
    m_data->setSelectedSamples(vec);
}

void SampleListWidget::selectUnambiguousSamples()
{
    std::vector<size_t> vec;
    for (size_t i = 0; i < m_data->sampleCount(); ++i)
        if (m_data->sampleType(i) == Data::UnambiguousSample)
            vec.push_back(i);
    m_data->setSelectedSamples(vec);
}

void SampleListWidget::invertSelection()
{
    std::vector<size_t> vec;
    for (size_t i = 0; i < m_data->sampleCount(); ++i) {
        if (!m_data->isSelected(m_data->sampleIndices(i)[0]))
            vec.push_back(i);
    }
    m_data->setSelectedSamples(vec);
}
