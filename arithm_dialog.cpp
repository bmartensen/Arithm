#include "arithm_dialog.h"
#include "ui_arithm_dialog.h"

ArithmDialog::ArithmDialog(QWidget *parent)
    : QDialog(parent), ui(new Ui::Dialog), m_Chart(new QChart()),
      m_Settings(new QSettings("Arithm.ini", QSettings::IniFormat))
{
    ui->setupUi(this);
    setWindowFlags(Qt::Window);

    // Plot theme
    m_ChartTheme = QChart::ChartTheme(m_Settings->value(PLOT_THEME_KEY, PLOT_THEME_DEFAULT).toInt());

    // Read default visualization parameters
    m_Samples = m_Settings->value(PLOT_SAMPLES_KEY, PLOT_SAMPLES_DEFAULT).toInt();
    m_HistoryCount = m_Settings->value(HISTORY_COUNT_KEY, HISTORY_COUNT_DEFAULT).toInt();

    ResetSymbols();

    // Limit plot sample parameter to reasonable values
    if(m_Samples < PLOT_SAMPLES_MIN)
        m_Samples = PLOT_SAMPLES_MIN;
    if(m_Samples > PLOT_SAMPLES_MAX)
        m_Samples = PLOT_SAMPLES_MAX;

    // Independent variable
    m_Symbols.add_variable("x", m_X);

    // Dependent variables
    m_Symbols.add_variable("f", m_F);
    m_Symbols.add_variable("g", m_G);
    m_Symbols.add_variable("h", m_H);

    // Constraints
    m_Symbols.add_variable("x_min", m_X_Min);
    m_Symbols.add_variable("x_max", m_X_Max);

    m_Symbols.add_variable("y_min", m_Y_Min);
    m_Symbols.add_variable("y_max", m_Y_Max);

    m_Symbols.add_constants();
    m_Expression.register_symbol_table(m_Symbols);

    // Set focus to input so we can start right away
    ui->input->setFocus();

    LoadHistory();
    Calculate();
}

ArithmDialog::~ArithmDialog()
{
    m_Chart->removeAllSeries();

    SaveHistory();

    delete m_Chart;
    delete m_Settings;

    delete ui;
}

void ArithmDialog::wheelEvent(QWheelEvent *event)
{
    arithm_double newRange;

    if(event->pixelDelta().y() < 0)
    {
        newRange = (m_X_Max - m_X_Min) * PLOT_ZOOM_FACTOR;
    }
    else
    {
        newRange = (m_X_Max - m_X_Min) / PLOT_ZOOM_FACTOR;
    }

    m_X_Min = (m_X_Min + m_X_Max) / 2.0 - newRange / 2.0;
    m_X_Max = m_X_Min + newRange;

    Calculate(false);
}

void ArithmDialog::LoadHistory()
{
    m_Settings->sync();

    ui->input->clear();
    if(m_Settings->value(HISTORY_SAVE_KEY, HISTORY_SAVE_DEFAULT).toInt() != 0)
    {
        for (int i = 0; i < m_HistoryCount; i++)
        {
            QString item = QString(m_Settings->value(QString(HISTORY_ENTRY_KEY) + QString::number(i), "").toString());
            if(!item.isEmpty())
            {
                ui->input->addItem(item);
            }
        }
    }

    ui->input->lineEdit()->clear();
}

void ArithmDialog::SaveHistory()
{
    m_Settings->sync();

    if(m_Settings->value(HISTORY_SAVE_KEY, HISTORY_SAVE_DEFAULT).toInt() != 0 &&
            !ui->input->lineEdit()->text().isEmpty())
    {
        // Allow multiple instances without losing history entries
        QString currentEntry = ui->input->lineEdit()->text();
        LoadHistory();

        bool bFound = false;
        for (int i = 0; i < m_HistoryCount; i++)
        {
            if(ui->input->itemText(i) == currentEntry)
                bFound = true;
        }

        if (bFound == false)
        {
            m_Settings->setValue(QString(HISTORY_ENTRY_KEY + QString::number(0)), currentEntry);

            // Remaining KEY_HISTORY_COUNT-1 items (last item will be discarded)
            for(int i = 0; i < m_HistoryCount-1; i++)
            {
                QString item = ui->input->itemText(i);
                if(!item.isEmpty())
                {
                    m_Settings->setValue(QString(HISTORY_ENTRY_KEY + QString::number(i+1)), item);
                }
            }
        }
    }

    m_Settings->sync();
}

void ArithmDialog::AddPair(QLineSeries *series, const arithm_double x, const arithm_double y, arithm_pair *minMax)
{
    series->append(x, y);

    if(std::isnan(minMax->first))
        minMax->first = y;
    else
        minMax->first = std::min(minMax->first, y);

    if(std::isnan(minMax->second))
        minMax->second = y;
    else
        minMax->second = std::max(minMax->second, y);
}

bool ArithmDialog::Prepare()
{
    // Collect user variables (x, f, g, h, ...)
    m_Parser.dec().collect_variables() = true;

    // Set all variables to not detected
    m_isLazy = m_isF = m_isG = m_isH = false;
    bool isX = false;

    if(m_Parser.compile(ui->input->lineEdit()->text().toStdString(), m_Expression))
    {
        std::deque<arithm_symbol> symbol_list;
        m_Parser.dec().symbols(symbol_list);

        for (std::size_t i = 0; i < symbol_list.size(); ++i)
        {
            QString found = QString::fromStdString(symbol_list[i].first);

            // Evaluate relevant user variables
            if(found == "x") isX = true;

            if(found == "f") m_isF = true;
            if(found == "g") m_isG = true;
            if(found == "h") m_isH = true;
        }

        if(isX && !m_isF && !m_isG && !m_isH)
            m_isLazy = true;

        return true;
    }

    return false;
}

void ArithmDialog::Calculate(bool resetZoom)
{
    if(Prepare())
    {
        // Reset symbols prior to expression evaluation
        ResetSymbols(resetZoom);

        // Evaluate expression with default symbols
        arithm_double result = m_Expression.value();

        if(m_isLazy || m_isF || m_isG || m_isH)
        {
            ui->chart->setUpdatesEnabled(false);

            // Track min/max for plot range settings
            arithm_pair minMax(std::nanl("1"), std::nanl("1"));

            QLineSeries *Fs = new QLineSeries();
            QLineSeries *Gs = new QLineSeries();
            QLineSeries *Hs = new QLineSeries();

            // Fixed horizontal range (we don't want x_min, x_max to be dependent on x)
            arithm_double a = m_X_Min;
            arithm_double b = m_X_Max;

            // Perform calculations
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

            // Remove previous plots
            m_Chart->removeAllSeries();
            m_Chart->legend()->show();

            // Add proper or lazy line series
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

            // Plot display settings
            m_Chart->createDefaultAxes();

            if(!m_Chart->axes(Qt::Horizontal).isEmpty())
            {
                m_Chart->axes(Qt::Horizontal).first()->setTitleText("x");
                m_Chart->axes(Qt::Horizontal).first()->setRange(double(m_X_Min), double(m_X_Max));
            }

            if(!m_Chart->axes(Qt::Vertical).isEmpty())
            {
                m_Chart->axes(Qt::Vertical).first()->setTitleText("function(x)");

                minMax = EvaluateRange(minMax);
                m_Chart->axes(Qt::Vertical).first()->setRange(
                            std::isnan(m_Y_Min) ? double(minMax.first) : double(m_Y_Min),
                            std::isnan(m_Y_Max) ? double(minMax.second): double(m_Y_Max));
            }

            ui->chart->setUpdatesEnabled(true);

            // Display plot boundaries info [x_min, x_max]
            ui->output->setText(QString::fromUtf8("Results for %1 ≤ x ≤ %2").arg(double(m_X_Min)).arg(double(m_X_Max)));
            ui->output->setStyleSheet(STYLE_HINT);
            ui->output->setToolTip("");
        }
        else
        {
            // Display results only
            ui->output->setText(QString::number(m_Expression.value(), 'G', 12));
            ui->output->setStyleSheet(STYLE_ACTIVE);
            ui->output->setToolTip("");

            ResetPlot();
        }
    }
    else
    {
        // Invalid expression
        ui->output->setText(tr("Please enter a valid expression"));
        ui->output->setStyleSheet(STYLE_HINT);
        ui->output->setToolTip(QString::fromStdString(m_Parser.error()));

        ResetPlot();
    }
}

arithm_pair ArithmDialog::EvaluateRange(arithm_pair minMax)
{
    arithm_pair result = minMax;
    arithm_double delta = minMax.second - minMax.first;
    arithm_double factor = 1.0;

    // Restrict plotting of functions to available rounding ranges [1µ, 1M]
    // Plot smaller ranges as constants and indicate this by vertical scale
    if(delta < 0.000001)
    {
        result.first -= 0.5;
        result.second += 0.5;

        delta = result.second - result.first;
    }
    else
    {
        // "The manufactured rounding table"
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

    // Execute rounding (factor = 1.0 for small deltas)
    result.first = std::floor(result.first / factor) * factor;
    result.second = std::ceil(result.second / factor) * factor;

    return result;
}

void ArithmDialog::ResetSymbols(bool resetZoom)
{
    m_F = std::nanl("1");
    m_G = std::nanl("1");
    m_H = std::nanl("1");
    m_X = std::nanl("1");

    // Always use default scaling for y_min, x_max unless specified in current expression
    m_Y_Min = std::nanl("1");
    m_Y_Max = std::nanl("1");

    if(resetZoom)
    {
        m_X_Min = m_Settings->value(PLOT_X_MIN_KEY, PLOT_X_MIN_DEFAULT).toFloat();
        m_X_Max = m_Settings->value(PLOT_X_MAX_KEY, PLOT_X_MAX_DEFAULT).toFloat();
    }
}

void ArithmDialog::ResetPlot()
{
    ui->chart->setUpdatesEnabled(false);

    m_Chart->removeAllSeries();

    m_Chart->addSeries(new QLineSeries());
    m_Chart->createDefaultAxes();

    m_Chart->legend()->hide();

    if(!m_Chart->axes(Qt::Horizontal).isEmpty())
    {
        m_Chart->axes(Qt::Horizontal).first()->setTitleText("x");
        m_Chart->axes(Qt::Horizontal).first()->setRange(double(m_X_Min), double(m_X_Max));
    }

    if(!m_Chart->axes(Qt::Vertical).isEmpty())
    {
        m_Chart->axes(Qt::Vertical).first()->setTitleText("function(x)");
        m_Chart->axes(Qt::Vertical).first()->setRange(0.0, 1.0);
    }

    m_Chart->setTheme(m_ChartTheme);
    ui->chart->setFocusPolicy(Qt::NoFocus);

    ui->chart->setChart(m_Chart);
    ui->chart->setRenderHint(QPainter::Antialiasing);

    ui->chart->setUpdatesEnabled(true);
}

void ArithmDialog::on_input_editTextChanged(const QString& /* arg1 */)
{
    Calculate(true);
}
