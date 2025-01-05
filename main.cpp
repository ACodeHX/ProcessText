#include "src/processtext.h"

#include <QApplication>

int main(
    int argc, char *argv[])
{
    QApplication a(argc, argv);
    ProcessText w;
    w.show();
    return a.exec();
}
