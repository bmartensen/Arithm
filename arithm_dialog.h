#pragma once

#include <QDialog>
#include <QtCharts>

#include "exprtk.hpp"

QT_BEGIN_NAMESPACE
namespace Ui { class Dialog; }
QT_END_NAMESPACE

// exprtk project settings
typedef long double arithm_double;
typedef std::pair<arithm_double, arithm_double> arithm_pair;
typedef exprtk::parser<arithm_double>::dependent_entity_collector::symbol_t symbol_t;

class ArithmDialog : public QDialog
{
    Q_OBJECT

public:
    ArithmDialog(QWidget *parent = nullptr);
    ~ArithmDialog();

private:
    // calculation
    bool Prepare(std::string expression);
    void Calculate(std::string expression);

    // clean-ups
    void ResetSymbols();
    void ResetPlot();

    // convenience
    void AddPair(QtCharts::QLineSeries *series, const arithm_double x, const arithm_double y, arithm_pair *minMax);
    arithm_pair EvaluateRange(arithm_pair minMax);

private slots:
    void on_input_textChanged();

private:
    Ui::Dialog *ui;

private:
    exprtk::symbol_table<arithm_double> m_Symbols;
    exprtk::expression<arithm_double> m_Expression;
    exprtk::parser<arithm_double> m_Parser;

    // plot sample points
    int m_Samples = 512;

    // plot interval
    arithm_double m_X, m_A = -6.0, m_B = 6.0;

    // plot user variables
    arithm_double m_F = std::nanl("1"), m_G = std::nanl("1"), m_H = std::nanl("1");

    bool m_isLazy, m_isF, m_isG, m_isH = false;

private:
    QChart *m_Chart;
    QSettings *m_Settings;
};
