#include "arithm_dialog.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    ArithmDialog w;
    w.show();

    // nothing to see here, please move along.
    return a.exec();
}
