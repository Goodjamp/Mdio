#ifndef MDIO_H
#define MDIO_H

#include <QMainWindow>
#include <QThread>
#include <QDebug>
#include <QVector>
#include <QRandomGenerator>
#include <QSpacerItem>
#include <QThread>
#include <QSemaphore>
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
#define COMMUNICATION_COMPLETE_TIMEOUTE    2000


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
    void readConfiguration(int slaveAddress);
    void apply(int slaveAddress, Communication::SlaveConfiguration configuration);
    void reload(int slaveAddress);
    void readState(int slaveAddress);
    void setTeleControl(int slaveAddress, int index, bool enable);
    void readMetaInformation(int slaveAddress);

private:
    void initCustomUi();
    void updateUiConnectionStatusStr(bool isConnect);
    void updateUiDeviceMetaInfStr(bool isConnect);
    void updateUiConfiguration(void);
    void errorMessage(QString headr, QString detailed);
    bool processingCommunicatitonResult(QString headr, QString detailed);


private slots:

    void on_pbConnectionSettings_clicked();

    void on_pbApplySettings_clicked();

    void on_pbDisconnect_clicked();

    void on_pbReload_clicked();

    void on_pbReadSettings_clicked();

    void connectSlaveResult(bool result);
    void readMetaInformationResult(bool result, int fwVersion,
                                   int yearConf, int monthConf, int dayConf);
    void readConfigurationResult(bool result, Communication::SlaveConfiguration configuration);
    void applyConnectionSettings(QVector<int>);
    void reloadResult(bool result);

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

    bool needConnectSlave;
    uint connectPortIndex = 0;
    uint connectBrIndex = DEFAULT_CONNECT_BR_INDEX;
    uint connectParityIndex = DEFAULT_CONNECT_PARITY_INDEX;
    uint connectStopBitsIndex = DEFAULT_CONNECT_STOP_BITS_INDEX;
    uint connectSlaveAddress = DEFAULT_CONNECT_SLAVE_ADDRESS;
    uint connectDeviceVersion = DEFAULT_CONNECT_VERSION;
    uint connectDeviceConfYear = DEFAULT_CONNECT_YEAR;
    uint connectDeviceConfMonth = DEFAULT_CONNECT_MONTH;
    uint connectDeviceConfDay = DEFAULT_CONNECT_DATE;

    uint configBr;
    uint configParity;
    uint configStop;

    QSemaphore communicationSyncSem{1};
    bool communicationResult;
    /*
    QSemaphore connecteSem{1};
    QSemaphore readConfigurationSem{1};
    QSemaphore writeConfigurationSem{1};
    QSemaphore readMetaInformationSem{1};
    QSemaphore readStatusSem{1};
    QSemaphore setTeleCOntrolSem{1};
    QSemaphore reloadSem{1};
    */


    const QMap<int, SerialCommunication::SerialPortParity> parityUiToSerilaLUT{
            {0, SerialCommunication::NONE},
            {1, SerialCommunication::EVEN},
            {2, SerialCommunication::ODD}
        };

    const QMap<int, SerialCommunication::SerialPortStopBits> stopUiToSerilaLUT{
            {0, SerialCommunication::ONE},
            {1, SerialCommunication::TWOO}
        };
    QStringList brList = {"1200", "2400", "4800", "9600", "14400", "19200", "28800", "38400", "57600"};
    QStringList parityList = {"None", "Even", "Odd"};
    QStringList stopBitsList = {"1", "2"};
};
#endif // MDIO_H
