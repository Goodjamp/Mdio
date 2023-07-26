#include "mdio.h"

#include <QApplication>
#include <QDebug>
#include <QThread>
#include <QObject>
#include <QMetaType>
#include "a.h"
#include "communication.h"


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Mdio w;
    w.show();
    qRegisterMetaType< std::function<void(int)> >("std::function<void(int)>");
    return a.exec();
}
