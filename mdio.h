#ifndef MDIO_H
#define MDIO_H

#include <QMainWindow>
#include <QThread>
#include <QDebug>
#include <QVector>
#include <QRandomGenerator>
#include <QSpacerItem>
#include <QThread>
#include "a.h"
#include "communication.h"
#include "tccontrol.h"
#include "tssettings.h"
#include "tsstatus.h"

QT_BEGIN_NAMESPACE
namespace Ui { class Mdio; }
QT_END_NAMESPACE

#define DEFAULT_CONNECT_BR_INDEX           8
#define DEFAULT_CONNECT_PARITY_INDEX       0
#define DEFAULT_CONNECT_STOP_BITS_INDEX    0
#define DEFAULT_CONNECT_SLAVE_ADDRESS      254
#define DEFAULT_CONNECT_VERSION            0
#define DEFAULT_CONNECT_YEAR               0
#define DEFAULT_CONNECT_MONTH              0
#define DEFAULT_CONNECT_DATE               0

class Mdio : public QMainWindow
{
    Q_OBJECT

public:
    Mdio(QWidget *parent = nullptr);
    ~Mdio();

signals:
    void connectSlave(QString port, int baudRate,
                      SerialCommunication::SerialPortParity parity,
                      SerialCommunication::SerialPortStopBits stopBits);
    void disconnectSlave();
    void writeConfiguration(int slaveAddress, Communication::SlaveSettings settings);
    void readConfiguration(int slaveAddress);
    void apply(int slaveAddress);
    void reload(int slaveAddress);
    void readState(int slaveAddress);
    void setTeleControl(int slaveAddress, int index, bool enable);
    void readMetaInformation(int slaveAddress);

private:
    void initCustomUi();
    void updateConnectionStatusStr(bool isConnect);
    void updateDeviceMetaInfStr(bool isConnect);

private slots:

    void on_pbConnectionSettings_clicked();

    void applyConnectionSettings(QVector<int>);

    void on_pbApplySettings_clicked();

    void connectSlaveResult(bool result);
    void readMetaInformationResult(bool result, int fwVersion,
                                   int yearConf, int monthConf, int dayConf);
    void restartResult(bool result);

    void on_pbDisconnect_clicked();

    void on_pbReload_clicked();

    void on_pbReadSettings_clicked();

private:
    Ui::Mdio *ui;
    Communication *communicaiton;
    QVector<TcControl *> tcStatic;
    QVector<TsSettings *> tsSetings;
    QVector<TsStatus *> tsStatus;
    TcControl *tcPuls;
    QRandomGenerator rand;
    QSpacerItem *tcLayoutSpacer;
    QSpacerItem *tsLayoutSpacer;
    QSpacerItem *tsStatusLayoutSpacer;
    QThread *commmunicationThread;

    uint connectPortIndex = 0;
    uint connectBrIndex = DEFAULT_CONNECT_BR_INDEX;
    uint connectParityIndex = DEFAULT_CONNECT_PARITY_INDEX;
    uint connectStopBitsIndex = DEFAULT_CONNECT_STOP_BITS_INDEX;
    uint connectSlaveAddress = DEFAULT_CONNECT_SLAVE_ADDRESS;
    uint connectDeviceVersion = DEFAULT_CONNECT_VERSION;
    uint connectDeviceConfYear = DEFAULT_CONNECT_YEAR;
    uint connectDeviceConfMonth = DEFAULT_CONNECT_MONTH;
    uint connectDeviceConfDay = DEFAULT_CONNECT_DATE;
};
#endif // MDIO_H
