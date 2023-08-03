#include "mdio.h"

#include <QApplication>
#include <QDebug>
#include <QThread>
#include <QObject>
#include <QMetaType>
#include "a.h"
#include "communication.h"
#include "QFile"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Mdio w;

    /*
     * Open and apply style file
     */
    QFile styleF(":/style.css");
    styleF.open(QFile::ReadOnly | QIODevice::Text);
    QString qssStr = styleF.readAll();
    qApp->setStyleSheet(qssStr);

    w.show();
    qRegisterMetaType< std::function<void(int)> >("std::function<void(int)>");
    return a.exec();
}
