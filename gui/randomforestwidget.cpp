#include "randomforestwidget.h"
#include <QFormLayout>
#include <QThread>
#include "../core/data.h"
#include "../ranger/ForestClassification.h"
#include "../ranger/ForestProbability.h"
#include "../ranger/globals.h"
#include <fstream>
#include <QFile>
#include <QTextStream>
#include <QDir>
#include <QCoreApplication>
#include <random>
#include <QSpinBox>

RandomForestWorker::RandomForestWorker(Data * data)
    : m_data(data)
{
}

RandomForestWorker::~RandomForestWorker()
{
}

void RandomForestWorker::go()
{
    bool sourceIsFuzzy = false;
    for (auto source : m_sourceIndices) {
        if (!m_data->fuzzyColor(source).isFixed()) {
            sourceIsFuzzy = true;
            break;
        }
    }

    std::string rangerPath = QDir::tempPath().toStdString();
    if (rangerPath.ends_with('/'))
        rangerPath.resize(rangerPath.size() - 1);

    // step one, generate a forest from the data

    m_numTrees = 1;

    if (sourceIsFuzzy) {

        std::random_device rd;
        std::mt19937 rng(rd());
        std::uniform_real_distribution<> dist(0.0, 1.0);
        std::vector<FuzzyColor> cols(m_targetIndices.size(), FuzzyColor(m_data->colorComponentCount()));

        for (int i = 0; i < 100; ++i) {

            std::ofstream infile { rangerPath + "/ranger.txt"};
            infile << "X Y target" << std::endl;
            for (auto source : m_sourceIndices) {
                auto col = m_data->fuzzyColor(source);
                if (col.isFixed()) {
                    infile << m_data->point(source).x() << " " << m_data->point(source).y() << " " << m_data->fuzzyColor(source).dominantComponent() << std::endl;
                } else {
                    double p = dist(rng);
                    int k = 0;
                    double pos = col.weight(0);
                    while (pos < p) {
                        ++k;
                        pos += col.weight(k);
                    }
                    infile << m_data->point(source).x() << " " << m_data->point(source).y() << " " << k << std::endl;
                }
            }
            infile.close();

            useRanger(rangerPath, false);
            QFile f(QString::fromStdString(rangerPath) + "/ranger.prediction");
            f.open(QFile::ReadOnly);
            QTextStream ts(&f);
            ts.readLine();
            auto c = ts.readLine().split(" ", Qt::SkipEmptyParts);
            QList<int> components;
            for (auto & val : c) {
                components.push_back(val.toInt());
            }
            ts.readLine();
            int linePos = 0;
            while (!ts.atEnd()) {
                auto probs = ts.readLine().split(" ", Qt::SkipEmptyParts);
                if (probs.size() == components.size()) {
                    for (int k = 0; k < probs.size(); ++k) {
                        cols[linePos].setWeight(components[k], cols[linePos].weight(components[k]) + probs[k].toDouble());
                    }
                    ++linePos;
                    if (linePos == m_targetIndices.size()) break;
                }
            }

            emit updateProgress(100 * (double)i / 100);
        }
        for (int i = 0; i < cols.size(); ++i) {
            cols[i].normalize();
            m_data->setColor(m_targetIndices[i], cols[i]);
        }



    } else {

        std::ofstream infile { rangerPath + "/ranger.txt"};
        infile << "X Y target" << std::endl;
        for (auto source : m_sourceIndices)
            infile << m_data->point(source).x() << " " << m_data->point(source).y() << " " << m_data->fuzzyColor(source).dominantComponent() << std::endl;
        infile.close();
        useRanger(rangerPath, true);
        QFile f(QString::fromStdString(rangerPath) + "/ranger.prediction");
        f.open(QFile::ReadOnly);
        QTextStream ts(&f);
        ts.readLine();
        auto c = ts.readLine().split(" ", Qt::SkipEmptyParts);
        QList<int> components;
        for (auto & val : c) {
            components.push_back(val.toInt());
        }
        ts.readLine();
        int i = 0;
        while (!ts.atEnd()) {
            auto probs = ts.readLine().split(" ", Qt::SkipEmptyParts);
            if (probs.size() == components.size()) {
                FuzzyColor col(m_data->colorComponentCount());
                for (int k = 0; k < probs.size(); ++k) {
                    col.setWeight(components[k], probs[k].toDouble());
                }
                m_data->setColor(m_targetIndices[i], col);
                ++i;
                if (i == m_targetIndices.size()) break;
            }
        }
    }

    emit finished();
}

void RandomForestWorker::useRanger(const std::string & rangerPath, bool connection)
{
    // step one, grow trees
    {
    ranger::ForestProbability forest;
    if (connection)
        connect(&forest, &ranger::Forest::updateProgress, this, &RandomForestWorker::updateProgress);
    std::ofstream logfile {  rangerPath + "/ranger.log" };
    forest.initCpp("target", // dependent_variable_name
                   ranger::MemoryMode::MEM_DOUBLE,  // memory mode
                   rangerPath + "/ranger.txt",
                   0, // mtry ***
                   rangerPath + "/ranger", // output prefix
                   m_numTrees, // num_trees,
                   &logfile,
                   0, // seed,
                   QThread::idealThreadCount(), // num_threads,
                   "", // load forest filename
                   ranger::DEFAULT_IMPORTANCE_MODE, // importance mode
                   ranger::DEFAULT_MIN_NODE_SIZE_CLASSIFICATION, // min_node_size,
                   ranger::DEFAULT_MIN_BUCKET,//  min_bucket,
                   "", //split_select_weights_file
                   std::vector<std::string>(), // always_split_variable_names,
                   "", // status_variable_name (only for survival)
                   true, // bool sample_with_replacement,
                   std::vector<std::string>(), //& unordered_variable_names,
                   false, // bool memory_saving_splitting,
                   ranger::DEFAULT_SPLITRULE, // SplitRule splitrule,
                   "", // case_weights_file, only if holdout is true
                   false, // bool predict_all,
                   ranger::DEFAULT_SAMPLE_FRACTION_REPLACE, // or, NOREPLACE, double sample_fraction,
                   ranger::DEFAULT_ALPHA, //double alpha,
                   ranger::DEFAULT_MINPROP, // double minprop,
                   false, // bool holdout,
                   ranger::DEFAULT_PREDICTIONTYPE, //PredictionType prediction_type,
                   ranger::DEFAULT_NUM_RANDOM_SPLITS, // uint num_random_splits,
                   ranger::DEFAULT_MAXDEPTH, // uint max_depth,
                   std::vector<double>(), // const std::vector<double>& regularization_factor,
                   false); //bool regularization_usedepth
    forest.run(true, true);
    forest.saveToFile();
    forest.writeOutput();
    }

    // step two, calc probabilities
    {
    std::ofstream infile { rangerPath + "/ranger.txt"};
    infile << "X Y target" << std::endl;
    for (auto target : m_targetIndices) {
        infile << m_data->point(target).x() << " " << m_data->point(target).y() << " " << 1 << std::endl;
    }
    infile.close();
    std::ofstream logfile {  rangerPath + "/ranger.log" };
    ranger::ForestProbability forest;
    if (connection)
        connect(&forest, &ranger::Forest::updateProgress, this, &RandomForestWorker::updateProgress);
    forest.initCpp("target", // dependent_variable_name
                   ranger::MemoryMode::MEM_DOUBLE,  // memory mode
                   rangerPath + "/ranger.txt",
                   0, // mtry ***
                   rangerPath + "/ranger", // output prefix
                   m_numTrees, // num_trees,
                   &logfile,
                   0, // seed,
                   QThread::idealThreadCount(), // num_threads,
                   rangerPath + "/ranger.forest", // load forest filename
                   ranger::DEFAULT_IMPORTANCE_MODE, // importance mode
                   ranger::DEFAULT_MIN_NODE_SIZE_CLASSIFICATION, // min_node_size,
                   ranger::DEFAULT_MIN_BUCKET,//  min_bucket,
                   "", //split_select_weights_file
                   std::vector<std::string>(), // always_split_variable_names,
                   "", // status_variable_name (only for survival)
                   true, // bool sample_with_replacement,
                   std::vector<std::string>(), //& unordered_variable_names,
                   false, // bool memory_saving_splitting,
                   ranger::DEFAULT_SPLITRULE, // SplitRule splitrule,
                   "", // case_weights_file, only if holdout is true
                   false, // bool predict_all,
                   ranger::DEFAULT_SAMPLE_FRACTION_REPLACE, // or, NOREPLACE, double sample_fraction,
                   ranger::DEFAULT_ALPHA, //double alpha,
                   ranger::DEFAULT_MINPROP, // double minprop,
                   false, // bool holdout,
                   ranger::DEFAULT_PREDICTIONTYPE, //PredictionType prediction_type,
                   ranger::DEFAULT_NUM_RANDOM_SPLITS, // uint num_random_splits,
                   ranger::DEFAULT_MAXDEPTH, // uint max_depth,
                   std::vector<double>(), // const std::vector<double>& regularization_factor,
                   false); //bool regularization_usedepth
    forest.run(true, true);
    forest.saveToFile();
    forest.writeOutput();
    }

}

void RandomForestWorker::cancel()
{
    m_cancel = true;
}


RandomForestWidget::RandomForestWidget(Data * data, QWidget * parent)
    : m_data(data)
{
    QFormLayout * form = new QFormLayout;
    form->setContentsMargins(0,0,0,0);
    m_numTreesSpinBox = new QSpinBox;
    m_numTreesSpinBox->setRange(10, 5000);
    m_numTreesSpinBox->setValue(500);
    m_numTreesSpinBox->setSingleStep(50);
    form->addRow("Number of Trees", m_numTreesSpinBox);
    setLayout(form);
}

void RandomForestWidget::run(const QList<size_t> & sourceIndices, const QList<size_t> & targetIndices)
{
    if (!assignmentWorkerThread) {
        emit beginAssignment();
        setEnabled(false);
        assignmentWorkerThread = new QThread;
        RandomForestWorker * worker = new RandomForestWorker(m_data);
        worker->setParams(m_numTreesSpinBox->value(), sourceIndices, targetIndices);
        worker->moveToThread(assignmentWorkerThread);
        connect(this, &RandomForestWidget::startAssignment, worker, &RandomForestWorker::go);
        connect(worker, &RandomForestWorker::finished, this, &RandomForestWidget::assignmentThreadFinished);
        connect(worker, &RandomForestWorker::updateProgress, this, &RandomForestWidget::updateProgress);
        connect(assignmentWorkerThread, &QThread::finished, worker, &QObject::deleteLater);
        connect(this, &RandomForestWidget::cancelAssignment, worker, &RandomForestWorker::cancel);
        assignmentWorkerThread->start();
        emit startAssignment();
    }
}

void RandomForestWidget::cancel()
{
    emit cancelAssignment();
    assignmentThreadFinished();
}

void RandomForestWidget::assignmentThreadFinished()
{
    if (assignmentWorkerThread) {
        assignmentWorkerThread->quit();
        assignmentWorkerThread->wait();
        assignmentWorkerThread->deleteLater();
        assignmentWorkerThread = nullptr;
        emit endAssignment();
        setEnabled(true);
    }
}
