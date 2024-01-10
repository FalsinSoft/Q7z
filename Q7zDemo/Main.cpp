#include "Q7zDemo.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Q7zDemo w;
    w.show();
    return a.exec();
}
