#include "arithm_dialog.h"
#include "ui_arithm_dialog.h"

#include <QMessageBox>

// default styles
const QString g_styleActive = "QLineEdit { padding: 5px; border: 0px solid gray; border-radius: 4px; color: #000; }";
const QString g_styleHint = "QLineEdit { padding: 5px; border: 0px solid gray; border-radius: 4px; color: #ccc; }";

ArithmDialog::ArithmDialog(QWidget *parent)
    : QDialog(parent), ui(new Ui::Dialog), m_Chart(new QChart()),
      m_Settings(new QSettings("Arithm.ini", QSettings::IniFormat))
{
    ui->setupUi(this);
    setWindowFlags(Qt::Window);

    ui->input->setFocus();
    ui->input->setStyleSheet(g_styleActive);

    // read default visualization parameters
    m_A = m_Settings->value("a", -6).toFloat();
    m_B = m_Settings->value("b", 6).toFloat();
    m_Samples = m_Settings->value("samples", 512).toInt();

    // limit plot sample parameter
    if(m_Samples < 2)
        m_Samples = 2;
    if(m_Samples > 16384)
        m_Samples = 16384;

    // independent variable
    m_Symbols.add_variable("x", m_X);

    // dependent variables
    m_Symbols.add_variable("f", m_F);
    m_Symbols.add_variable("g", m_G);
    m_Symbols.add_variable("h", m_H);

    // constraints
    m_Symbols.add_variable("a", m_A);
    m_Symbols.add_variable("b", m_B);

    m_Symbols.add_constants();
    m_Expression.register_symbol_table(m_Symbols);

    ResetPlot();

    Calculate("");
}

ArithmDialog::~ArithmDialog()
{
    m_Chart->removeAllSeries();

    delete m_Chart;
    delete m_Settings;

    delete ui;
}

bool ArithmDialog::Prepare(std::string expression)
{
    // collect user variables (x, f, g, h, ...)
    m_Parser.dec().collect_variables() = true;

    // set all variables to not detected
    m_isLazy = m_isF = m_isG = m_isH = false;

    bool isX = false;

    if(m_Parser.compile(expression, m_Expression))
    {
        std::deque<symbol_t> symbol_list;
        m_Parser.dec().symbols(symbol_list);

        for (std::size_t i = 0; i < symbol_list.size(); ++i)
        {
            QString found = QString::fromStdString(symbol_list[i].first);

            // evaluate relevant user variables
            if(found == "x") isX = true;
            if(found == "f") m_isF = true;
            if(found == "g") m_isG = true;
            if(found == "h") m_isH = true;
        }

        if(isX && !m_isF && !m_isG && !m_isH)
            m_isLazy = true;

        // parser compile ok
        return true;
    }

    // parser compile error
    return false;
}

void ArithmDialog::AddPair(QtCharts::QLineSeries *series, const arithm_double x, const arithm_double y, arithm_pair *minMax)
{
    // add point to plot
    series->append(x, y);

    // determine minimum
    if(std::isnan(minMax->first))
        minMax->first = y;
    else
        minMax->first = std::min(minMax->first, y);

    // determine maximum
    if(std::isnan(minMax->second))
        minMax->second = y;
    else
        minMax->second = std::max(minMax->second, y);
}

void ArithmDialog::Calculate(std::string expression)
{
    if(Prepare(expression))
    {
        // reset symbols prior to expression evaluation
        ResetSymbols();

        // evaluate expression with default symbols
        arithm_double result = m_Expression.value();

        if(m_isLazy || m_isF || m_isG || m_isH)
        {
            // temporarily disable updates
            ui->chart->setUpdatesEnabled(false);

            // track min/max for plot range settings
            arithm_pair minMax(std::nanl("1"), std::nanl("1"));

            QLineSeries *Fs = new QLineSeries();
            QLineSeries *Gs = new QLineSeries();
            QLineSeries *Hs = new QLineSeries();

            // fixed horizontal range (we don't want a, b to be dependent on x)
            arithm_double a = m_A;
            arithm_double b = m_B;

            // perform calculations
            for(int i = 0; i < m_Samples; i++)
            {
                m_X = a + i * (b - a) / (m_Samples - 1);

                // re-calculate with new m_X value
                result = m_Expression.value();

                if(m_isF)
                    AddPair(Fs, m_X, m_F, &minMax);

                if(m_isG)
                    AddPair(Gs, m_X, m_G, &minMax);

                if(m_isH)
                    AddPair(Hs, m_X, m_H, &minMax);

                // lazy function plots (e.g. "sin(x)" instead of "f := sin(x)")
                if(m_isLazy)
                    AddPair(Fs, m_X, result, &minMax);
            }

            // remove previous plots
            m_Chart->removeAllSeries();
            m_Chart->legend()->show();

            // add proper or lazy line series
            if(m_isF || m_isLazy)
            {
                if(m_isLazy)
                {
                    // hide legend for lazy function plots
                    m_Chart->legend()->hide();
                }

                Fs->setName("f(x)");
                m_Chart->addSeries(Fs);
            }
            else
                delete Fs;

            if(m_isG)
            {
                Gs->setName("g(x)");
                m_Chart->addSeries(Gs);
            }
            else
                delete Gs;

            if(m_isH)
            {
                Hs->setName("h(x)");
                m_Chart->addSeries(Hs);
            }
            else
                delete Hs;

            // plot display settings
            m_Chart->createDefaultAxes();

            if(!m_Chart->axes(Qt::Horizontal).isEmpty())
            {
                m_Chart->axes(Qt::Horizontal).first()->setTitleText("x");
                m_Chart->axes(Qt::Horizontal).first()->setRange(double(m_A), double(m_B));
            }

            if(!m_Chart->axes(Qt::Vertical).isEmpty())
            {
                m_Chart->axes(Qt::Vertical).first()->setTitleText("function(x)");

                // Re-evaluate plot range
                minMax = EvaluateRange(minMax);
                m_Chart->axes(Qt::Vertical).first()->setRange(double(minMax.first), double(minMax.second));
            }

            // re-enable display updates and implicitely call update()
            ui->chart->setUpdatesEnabled(true);

            // display plot boundaries info [a, b]            
            ui->output->setText(QString::fromUtf8("Results for %1 ≤ x ≤ %2").arg(double(m_A)).arg(double(m_B)));
            ui->output->setStyleSheet(g_styleHint);
            ui->output->setToolTip("");
        }
        else
        {
            // results only
            ui->output->setText(QString::number(m_Expression.value(), 'G', 12));
            ui->output->setStyleSheet(g_styleActive);
            ui->output->setToolTip("");

            ResetPlot();
        }
    }
    else
    {
        // invalid expression
        ui->output->setText(tr("Please enter a valid expression"));
        ui->output->setStyleSheet(g_styleHint);
        ui->output->setToolTip(QString::fromStdString(m_Parser.error()));

        ResetPlot();
    }
}

arithm_pair ArithmDialog::EvaluateRange(arithm_pair minMax)
{
    arithm_pair result = minMax;
    arithm_double delta = minMax.second - minMax.first;
    arithm_double factor = 1.0;

    // restrict plotting of functions to available rounding ranges [1µ, 1M]
    // plot smaller ranges as constants and indicate this by vertical scale
    if(delta < 0.000001)
    {
        // make some room
        result.first -= 0.5;
        result.second += 0.5;

        // recalculate delta
        delta = result.second - result.first;
    }
    else
    {
        // "the manufactured rounding table"
        if(delta < 0.00005) factor = 0.000001; // 1µ
        if(delta >= 0.00005 && delta < 0.0005) factor = 0.00001;
        if(delta >= 0.0005 && delta < 0.005) factor = 0.0001;
        if(delta >= 0.005 && delta < 0.05) factor = 0.001;
        if(delta >= 0.05 && delta < 0.5) factor = 0.01;
        if(delta >= 0.5 && delta < 5.0) factor = 0.1;
        if(delta >= 5.0 && delta < 50.0) factor = 1.0;
        if(delta >= 50.0 && delta < 500.0) factor = 10.0;
        if(delta >= 500.0 && delta < 5000.0) factor = 100.0;
        if(delta >= 5000.0 && delta < 50000.0) factor = 1000.0;
        if(delta >= 50000.0 && delta < 500000.0) factor = 10000.0;
        if(delta >= 500000.0 && delta < 5000000.0) factor = 100000.0;
        if(delta >= 5000000.0) factor = 1000000.0; // 1M
    }

    // execute rounding (factor = 1.0 for small deltas)
    result.first = std::floorl(result.first / factor) * factor;
    result.second = std::ceill(result.second / factor) * factor;

    return result;
}

void ArithmDialog::ResetSymbols()
{
    m_F = std::nanl("1");
    m_G = std::nanl("1");
    m_H = std::nanl("1");
    m_X = std::nanl("1");
}

void ArithmDialog::ResetPlot()
{
    // temporarily disable display updates
    ui->chart->setUpdatesEnabled(false);

    m_Chart->removeAllSeries();

    m_Chart->addSeries(new QLineSeries());
    m_Chart->createDefaultAxes();

    m_Chart->legend()->hide();

    if(!m_Chart->axes(Qt::Horizontal).isEmpty())
    {
        m_Chart->axes(Qt::Horizontal).first()->setTitleText("x");
        m_Chart->axes(Qt::Horizontal).first()->setRange(double(m_A), double(m_B));
    }

    if(!m_Chart->axes(Qt::Vertical).isEmpty())
    {
        m_Chart->axes(Qt::Vertical).first()->setTitleText("function(x)");
        m_Chart->axes(Qt::Vertical).first()->setRange(0.0, 1.0);
    }

    m_Chart->setTheme(QChart::ChartTheme(m_Settings->value("theme", 2).toInt()));
    ui->chart->setFocusPolicy(Qt::NoFocus);

    ui->chart->setChart(m_Chart);
    ui->chart->setRenderHint(QPainter::Antialiasing);

    // re-enable display updates and implicitely call update()
    ui->chart->setUpdatesEnabled(true);
}

void ArithmDialog::on_input_textChanged()
{
    Calculate(ui->input->text().toStdString());
}
