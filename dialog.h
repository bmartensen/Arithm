#pragma once

#include <QDialog>
#include <QtCharts>

#include "exprtk.hpp"

QT_BEGIN_NAMESPACE
namespace Ui { class Dialog; }
QT_END_NAMESPACE

class Dialog : public QDialog
{
    Q_OBJECT

public:
    Dialog(QWidget *parent = nullptr);
    ~Dialog();

    void Calculate(std::string expression);

private slots:
    void on_input_textChanged();

private:
    Ui::Dialog *ui;

private:
    exprtk::symbol_table<long double> m_Symbols;
    exprtk::expression<long double> m_Expression;
    exprtk::parser<long double> m_Parser;

    long double m_X, m_A = -5.0, m_B = 5.0, m_I = 128.0;

private:
    QLineSeries *m_Series;
    QChart *m_Chart;
    QSettings *m_Settings;
};
