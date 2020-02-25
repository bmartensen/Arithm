#include "dialog.h"
#include "ui_dialog.h"

const QString g_styleActive = "QLineEdit { padding: 5px; border: 0px solid gray; border-radius: 4px; color: #000; }";
const QString g_styleHint = "QLineEdit { padding: 5px; border: 0px solid gray; border-radius: 4px; color: #ccc; }";

Dialog::Dialog(QWidget *parent)
    : QDialog(parent), ui(new Ui::Dialog), m_Chart(new QChart()),
      m_Settings(new QSettings("Arithm.ini", QSettings::IniFormat))
{
    ui->setupUi(this);
    setWindowFlags(Qt::Window);

    ui->input->setFocus();
    ui->input->setStyleSheet(g_styleActive);

    // read default visualization parameters
    m_A = m_Settings->value("a", -5).toFloat();
    m_B = m_Settings->value("b", 5).toFloat();
    m_I = m_Settings->value("i", 256).toFloat();

    // independent variable
    m_Symbols.add_variable("x", m_X);

    // dependent variables
    m_Symbols.add_variable("f", m_F);
    m_Symbols.add_variable("g", m_G);
    m_Symbols.add_variable("h", m_H);

    // constraints
    m_Symbols.add_variable("a", m_A);
    m_Symbols.add_variable("b", m_B);
    m_Symbols.add_variable("i", m_I);

    m_Symbols.add_constants();
    m_Expression.register_symbol_table(m_Symbols);

    m_Chart->addSeries(new QLineSeries());
    m_Chart->createDefaultAxes();

    if(!m_Chart->axes(Qt::Horizontal).isEmpty())
        m_Chart->axes(Qt::Horizontal).first()->setTitleText("X");

    if(!m_Chart->axes(Qt::Vertical).isEmpty())
        m_Chart->axes(Qt::Vertical).first()->setTitleText("Y");

    m_Chart->setTheme(QChart::ChartTheme(m_Settings->value("theme", 2).toInt()));
    ui->chart->setFocusPolicy(Qt::NoFocus);

    ui->chart->setChart(m_Chart);
    ui->chart->setRenderHint(QPainter::Antialiasing);

    Calculate("");
}

Dialog::~Dialog()
{
    m_Chart->removeAllSeries();

    delete m_Chart;
    delete m_Settings;

    delete ui;
}

void Dialog::Calculate(std::string expression)
{
    m_Chart->removeAllSeries();

    // initialize dependent and independent variables
    m_F = m_G = m_H = m_X = std::nanl("1");

    // prepare plot boundary settings
    m_Parser.compile(expression, m_Expression);
    long double result = m_Expression.value();

    if(m_Parser.error_count() == 0)
    {
        if(std::isnan(result))
        {
            QLineSeries *Fs = new QLineSeries();
            QLineSeries *Gs = new QLineSeries();
            QLineSeries *Hs = new QLineSeries();

            int samples = int(m_I);
            bool isFs = false;

            long double a = m_A;
            long double b = m_B;

            // prevent overflow
            if(samples > 8192) samples = 8192;
            if(samples < 2) samples = 2;

            for(int i = 0; i < samples; i++)
            {
                m_X = a + i * (b - a) / (samples - 1);

                m_Parser.compile(expression, m_Expression);
                result = m_Expression.value();

                if(std::isnan(m_F))
                {
                    Fs->append(m_X, result);
                }
                else
                {
                    isFs = true;
                    Fs->append(m_X, m_F);
                }

                if(!std::isnan(m_G))
                    Gs->append(m_X, m_G);

                if(!std::isnan(m_H))
                    Hs->append(m_X, m_H);
            }

            if(Fs->points().count() > 0)
            {
                if(isFs)
                {
                    Fs->setName("f(x)");
                    m_Chart->addSeries(Fs);
                }
                else
                {
                    // lazy expression fallback, only use when g(x) and h(x) are not defined
                    if(Gs->points().count() == 0 && Hs->points().count() == 0)
                    {
                        Fs->setName("expression(x)");
                        m_Chart->addSeries(Fs);
                    }
                    else
                        delete Fs;
                }
            }
            else
                delete Fs;

            if(Gs->points().count() > 0)
            {
                Gs->setName("g(x)");
                m_Chart->addSeries(Gs);
            }
            else
                delete Gs;

            if(Hs->points().count() > 0)
            {
                Hs->setName("h(x)");
                m_Chart->addSeries(Hs);
            }
            else
                delete Hs;

            m_Chart->createDefaultAxes();
            if(!m_Chart->axes(Qt::Horizontal).isEmpty())
                m_Chart->axes(Qt::Horizontal).first()->setTitleText("X");

            if(!m_Chart->axes(Qt::Vertical).isEmpty())
                m_Chart->axes(Qt::Vertical).first()->setTitleText("Y");

            ui->output->setText(QString::fromUtf8("Results for %1 ≤ x ≤ %2").arg(double(m_A)).arg(double(m_B)));
            ui->output->setStyleSheet(g_styleHint);
            ui->output->setToolTip("");
        }
        else
        {
            // don't display a plot for non-dependent expression result
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
}

void Dialog::on_input_textChanged()
{
    Calculate(ui->input->text().toStdString());
}
