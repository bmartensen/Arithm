#pragma once

#include <QDialog>
#include <QtCharts>

#include "exprtk.hpp"
#include "settings.h"

QT_BEGIN_NAMESPACE
namespace Ui { class Dialog; }
QT_END_NAMESPACE

// Exprtk
typedef long double arithm_double;
typedef std::pair<arithm_double, arithm_double> arithm_pair;
typedef exprtk::parser<arithm_double>::dependent_entity_collector::symbol_t arithm_symbol;

class ArithmDialog : public QDialog
{
    Q_OBJECT

public:
    ArithmDialog(QWidget *parent = nullptr);
    ~ArithmDialog();

private slots:
    void on_input_editTextChanged(const QString &arg1);

private:
    bool Prepare();
    void Calculate();

    void ResetSymbols();
    void ResetPlot();

    void LoadHistory();
    void SaveHistory();

    void AddPair(QtCharts::QLineSeries *series, const arithm_double x, const arithm_double y, arithm_pair *minMax);
    arithm_pair EvaluateRange(arithm_pair minMax);

private:
    Ui::Dialog *ui;

private:
    exprtk::symbol_table<arithm_double> m_Symbols;
    exprtk::expression<arithm_double> m_Expression;
    exprtk::parser<arithm_double> m_Parser;

    QChart::ChartTheme m_ChartTheme = PLOT_THEME_DEFAULT;

    int m_Samples = PLOT_SAMPLES_DEFAULT;
    int m_HistoryCount = HISTORY_COUNT_DEFAULT;

    arithm_double m_X;

    arithm_double m_X_Min = PLOT_X_MIN_DEFAULT;
    arithm_double m_X_Max = PLOT_X_MAX_DEFAULT;

    arithm_double m_Y_Min = std::nanl("1");
    arithm_double m_Y_Max = std::nanl("1");

    arithm_double m_F = std::nanl("1"), m_G = std::nanl("1"), m_H = std::nanl("1");

    bool m_isLazy, m_isF, m_isG, m_isH = false;

private:
    QChart *m_Chart;
    QSettings *m_Settings;
};
