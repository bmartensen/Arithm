#pragma once

#include <QString>

// default styles
#define STYLE_ACTIVE            "QLineEdit { padding: 5px; border: 0px solid gray; border-radius: 4px; color: #000; }"
#define STYLE_HINT              "QLineEdit { padding: 5px; border: 0px solid gray; border-radius: 4px; color: #ccc; }"

// History
#define HISTORY_SAVE_KEY        "History/Save"
#define HISTORY_SAVE_DEFAULT    1

#define HISTORY_COUNT_KEY       "History/Count"
#define HISTORY_COUNT_DEFAULT   10

#define HISTORY_ENTRY_KEY      "History/Entry_"

// Plot
#define PLOT_X_MIN_KEY          "Plot/X_Min"
#define PLOT_X_MIN_DEFAULT      -6

#define PLOT_X_MAX_KEY          "Plot/X_Max"
#define PLOT_X_MAX_DEFAULT      6

#define PLOT_ZOOM_FACTOR        1.2

#define PLOT_SAMPLES_KEY        "Plot/Samples"
#define PLOT_SAMPLES_DEFAULT    2048
#define PLOT_SAMPLES_MIN        2
#define PLOT_SAMPLES_MAX        16384

#define PLOT_THEME_KEY          "Plot/Theme"
#define PLOT_THEME_DEFAULT      QChart::ChartThemeLight
