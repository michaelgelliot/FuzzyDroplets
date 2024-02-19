#ifndef FUZZYDROPLETS_GUI_SAMPLELISTWIDGET_H
#define FUZZYDROPLETS_GUI_SAMPLELISTWIDGET_H

#include "core/data.h"
#include "generic/command.h"
#include <QWidget>
#include <QTemporaryFile>

class QTableWidget;
class CommandStack;

class SampleListWidget : public QWidget
{
    Q_OBJECT

    class SetSampleTypeCommand : public Command<SetSampleTypeCommand>
    {
    public:

        SetSampleTypeCommand(Data * data, std::vector<size_t> samples, std::vector<Data::SampleType> oldTypes, Data::SampleType newType)
            : m_data(data), m_samples(samples), m_oldTypes(oldTypes), m_newType(newType) {}

        void redo();
        void undo();

    private:

        Data * m_data;
        std::vector<size_t> m_samples;
        std::vector<Data::SampleType> m_oldTypes;
        Data::SampleType m_newType;
    };

public:

    SampleListWidget(Data * data, CommandStack * commandStack, QWidget * parent = nullptr);
    ~SampleListWidget();

    void showContextMenu(const QPoint& pos);

    void removeRows(QList<int> rows);

    Data * data() {return m_data;}
    const Data * data() const {return m_data;}

public slots:

    void guiColorSchemeChanged();
    void itemSelectionChanged();
    void cellChanged(int row, int column);
    void parallelWorkStarted();
    void parallelWorkFinished();
    void selectedSamplesChanged();
    void sampleTypesChanged(std::vector<size_t> samples);
    void samplesAdded(std::vector<size_t> samples);
    void invertSelection();
    void selectAll();
    void selectExperimentalSamples();
    void selectPositiveControls();
    void selectNegativeControls();
    void selectNonTemplateControls();

private:

    size_t sampleFromRow(int i) const;
    int rowFromSample(size_t i) const;
    QList<int> sortedSelectedRows() const;

    Data * m_data;
    QTableWidget * m_tableWidget;
    QMap<Data::SampleType, QString> m_dataTypeToLabel;

    CommandStack * m_commandStack;
    bool m_reportSelectionChanged{true};
    bool m_reportSampleTypeChanged{true};
};

#endif // FUZZYDROPLETS_GUI_SAMPLELISTWIDGET_H
