#include "dialog.h"
#include "ui_dialog.h"

Dialog::Dialog(QWidget *parent)
    : QDialog(parent), ui(new Ui::Dialog),
      m_Series(new QLineSeries()), m_Chart(new QChart()),
      m_Settings(new QSettings("Arithm.ini", QSettings::IniFormat))
{    
    ui->setupUi(this);
    setWindowFlags(Qt::Window);

    ui->input->setFocus();
    ui->input->setStyleSheet("QLineEdit { padding: 5px; border: 0px solid gray; border-radius: 4px;}");

    // read default visualization parameters
    m_A = m_Settings->value("a", -5).toFloat();
    m_B = m_Settings->value("b", 5).toFloat();
    m_I = m_Settings->value("i", 256).toFloat();

    // add variables
    m_Symbols.add_variable("x", m_X);

    m_Symbols.add_variable("a", m_A);
    m_Symbols.add_variable("b", m_B);
    m_Symbols.add_variable("i", m_I);

    m_Symbols.add_constants();
    m_Expression.register_symbol_table(m_Symbols);

    m_Chart->legend()->hide();
    m_Chart->addSeries(m_Series);
    m_Chart->createDefaultAxes();

    m_Chart->setTheme(QChart::ChartTheme(m_Settings->value("theme", 2).toInt()));

    ui->chart->setStyleSheet("padding: 0px;");
    ui->chart->setFocusPolicy(Qt::NoFocus);

    ui->chart->setChart(m_Chart);
    ui->chart->setRenderHint(QPainter::Antialiasing);

    Calculate("");
}

Dialog::~Dialog()
{
    delete ui;

    delete m_Series;
    delete m_Chart;

    delete m_Settings;
}

void Dialog::Calculate(std::string expression)
{
    m_Chart->removeSeries(m_Series);
    m_Series->clear();

    // prepare plot boundary settings
    m_Parser.compile(expression, m_Expression);
    m_Expression.value();

    if(m_Parser.error_count() == 0)
    {
        int samples = int(m_I);

        long double a = m_A;
        long double b = m_B;

        long double min, max, result;

        // prevent overflow
        if(samples > 8192) samples = 8192;
        if(samples < 2) samples = 2;

        for(int i = 0; i < samples; i++)
        {
            m_X = a + i * (b - a) / (samples - 1);

            m_Parser.compile(expression, m_Expression);
            result = m_Expression.value();

            m_Series->append(m_X, result);

            if(i == 0)
            {
                min = max = result;
            }
            else
            {
                if(result > max)
                    max = result;
                if(result < min)
                    min = result;
            }
        }

        if(max == min)
        {
            // display no plot if results are constant
            m_Series->clear();

            ui->output->setText(QString::number(m_Expression.value(), 'G', 12));
            ui->output->setStyleSheet("QLineEdit { padding: 5px; border: 0px solid gray; border-radius: 4px; color: #000; }");
            ui->output->setToolTip("");

            m_Chart->setTitle("");
        }
        else
        {
            // display just a plot
            ui->output->setText(QString::fromUtf8("Results for %1 ≤ x ≤ %2").arg(double(m_A)).arg(double(m_B)));
            ui->output->setStyleSheet("QLineEdit { padding: 5px; border: 0px solid gray; border-radius: 4px; color: #ccc; }");
            ui->output->setToolTip("");

            m_Chart->setTitle(ui->output->text());
        }
    }
    else
    {
        ui->output->setText(tr("Please enter a valid expression"));
        ui->output->setStyleSheet("QLineEdit { padding: 5px; border: 0px solid gray; border-radius: 4px; color: #ccc; }");
        ui->output->setToolTip(QString::fromStdString(m_Parser.error()));

        m_Chart->setTitle("");
    }

    m_Chart->addSeries(m_Series);
    m_Chart->createDefaultAxes();
}

void Dialog::on_input_textChanged()
{
    Calculate(ui->input->text().toStdString());
}
