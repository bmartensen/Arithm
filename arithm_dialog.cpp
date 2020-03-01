#include "arithm_dialog.h"
#include "ui_arithm_dialog.h"

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

    m_Chart->addSeries(new QLineSeries());
    m_Chart->createDefaultAxes();
    m_Chart->legend()->hide();

    if(!m_Chart->axes(Qt::Horizontal).isEmpty())
        m_Chart->axes(Qt::Horizontal).first()->setTitleText("x");

    if(!m_Chart->axes(Qt::Vertical).isEmpty())
        m_Chart->axes(Qt::Vertical).first()->setTitleText("function(x)");

    m_Chart->setTheme(QChart::ChartTheme(m_Settings->value("theme", 2).toInt()));
    ui->chart->setFocusPolicy(Qt::NoFocus);

    ui->chart->setChart(m_Chart);
    ui->chart->setRenderHint(QPainter::Antialiasing);

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
        //arithm_double result = m_Expression.value();

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

void ArithmDialog::Calculate(std::string expression)
{
    m_Chart->removeAllSeries();

    if(Prepare(expression))
    {
        // calculate result
        arithm_double result = m_Expression.value();

        // don't show plots for empty expressions (e.g. "f; g; h;")
        m_F = std::nanl("1");
        m_G = std::nanl("1");
        m_H = std::nanl("1");

        if(m_isLazy || m_isF || m_isG || m_isH)
        {
            QLineSeries *Fs = new QLineSeries();
            QLineSeries *Gs = new QLineSeries();
            QLineSeries *Hs = new QLineSeries();

            arithm_double a = m_A;
            arithm_double b = m_B;

            for(int i = 0; i < m_Samples; i++)
            {
                m_X = a + i * (b - a) / (m_Samples - 1);

                // re-calculate with new m_X value
                result = m_Expression.value();

                if(m_isF)
                    Fs->append(m_X, m_F);

                if(m_isG)
                    Gs->append(m_X, m_G);

                if(m_isH)
                    Hs->append(m_X, m_H);

                // lazy function plots (e.g. "sin(x)" instead of "f := sin(x)")
                if(m_isLazy)
                    Fs->append(m_X, result);
            }

            // show legend, it might be that more that one plot is displayed
            m_Chart->legend()->show();

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

            m_Chart->createDefaultAxes();
            if(!m_Chart->axes(Qt::Horizontal).isEmpty())
                m_Chart->axes(Qt::Horizontal).first()->setTitleText("x");

            if(!m_Chart->axes(Qt::Vertical).isEmpty())
                m_Chart->axes(Qt::Vertical).first()->setTitleText("function(x)");

            // display plot boundaries info [a, b]
            ui->output->setText(QString::fromUtf8("Results for %1 ≤ x ≤ %2").arg(double(m_A)).arg(double(m_B)));
            ui->output->setStyleSheet(g_styleHint);
            ui->output->setToolTip("");
        }
        else
        {
            // don't display a plot for calculation results
            ui->output->setText(QString::number(m_Expression.value(), 'G', 12));
            ui->output->setStyleSheet(g_styleActive);
            ui->output->setToolTip("");
        }
    }
    else
    {
        ui->output->setText(tr("Please enter a valid expression"));
        ui->output->setStyleSheet(g_styleHint);
        ui->output->setToolTip(QString::fromStdString(m_Parser.error()));
    }

    // always update
    ui->chart->update();
}

void ArithmDialog::on_input_textChanged()
{
    Calculate(ui->input->text().toStdString());
}
