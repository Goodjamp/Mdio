#include "mdio.h"

#include <QApplication>
#include <QDebug>
#include <QThread>
#include <QObject>
#include <QMetaType>
#include "communication.h"
#include "QFile"
#include "serialcommunication.h"
#include "stdbool.h"
#include "communication.h"
#include "dialogconnectionsettings.h"

Q_DECLARE_METATYPE(uint8_t);
Q_DECLARE_METATYPE(uint16_t);
Q_DECLARE_METATYPE(SerialCommunication::SerialPortParity);
Q_DECLARE_METATYPE(SerialCommunication::SerialPortStopBits);
Q_DECLARE_METATYPE(Communication::SlaveConfiguration);
Q_DECLARE_METATYPE(Communication::SlaveState);
Q_DECLARE_METATYPE(DialogConnectionSettings::UserSettingsList);
Q_DECLARE_METATYPE(DialogConnectionSettings::UiFilingList);

int main(int argc, char *argv[])
{
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication a(argc, argv);
    Mdio w;

    /*
     * Open and apply style file
     */
    QFile styleF(":/style.css");
    styleF.open(QFile::ReadOnly | QIODevice::Text);
    QString qssStr = styleF.readAll();
    qApp->setStyleSheet(qssStr);

    qRegisterMetaType<std::function<void(bool result)>>("std::function<void(bool result)>");
    qRegisterMetaType<std::function<void(bool result, Communication::SlaveConfiguration settings)>>
        ("std::function<void(bool result, Communication::SlaveConfiguration settings)>");
    qRegisterMetaType<std::function<void(bool result, Communication::SlaveState state)>>
        ("std::function<void(bool result, Communication::SlaveState state)>");
    qRegisterMetaType<std::function<void(bool result, int fwVersion)>>("std::function<void(bool result, int fwVersion)>");
    qRegisterMetaType<SerialCommunication::SerialPortParity>("SerialCommunication::SerialPortParity");
    qRegisterMetaType<SerialCommunication::SerialPortStopBits>("SerialCommunication::SerialPortStopBits");
    qRegisterMetaType<Communication::SlaveConfiguration>("Communication::SlaveConfiguration");
    qRegisterMetaType<Communication::SlaveState>("Communication::SlaveState");
    qRegisterMetaType<DialogConnectionSettings::UserSettingsList>("DialogConnectionSettings::UserSettingsList");
    qRegisterMetaType<DialogConnectionSettings::UiFilingList>("DialogConnectionSettings::UiFilingList");

    w.show();
    return a.exec();
}
