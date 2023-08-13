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
#define COMMUNICATION_COMPLETE_TIMEOUTE    5000
#define SILENT_INTERVAL_MIN_MS             0
#define SILENT_INTERVAL_MAX_MS             100
#define REPLAY_DELAY_MIN_MS                0
#define REPLAY_DELAY_MAX_MS                100
#define DEBOUNCE_INTARVAL_MIN_MS           20
#define DEBOUNCE_INTARVAL_MAX_MS           100
#define PULS_DURATION_MIN_MS               100
#define PULS_DURATION_MMAX_MS              4000

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
    bool updateUiConfiguration(void);
    void errorMessage(QString headr, QString detailed);
    bool processingCommunicatitonResult(QString headr, QString detailed);


private slots:
    void connectSlaveResult(bool result);
    void readMetaInformationResult(bool result, int fwVersion,
                                   int yearConf, int monthConf, int dayConf);
    void readConfigurationResult(bool result, Communication::SlaveConfiguration configuration);
    void applyConnectionSettings(QVector<int>);
    void reloadResult(bool result);

private slots:

    void on_pbConnectionSettings_clicked();

    void on_pbApplySettings_clicked();

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

    Communication::SlaveConfiguration connectDeviceConf;

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
    const QMap<int, QString> brValueToStrLUT = {
        {2400, "2400"},
        {4800, "4800"},
        {9600, "9600"},
        {19200, "19200"},
        {38400, "38400"},
        {57600, "57600"}
    };
    const QMap<int, QString>  paritySerialToStrLUT = {
        {SerialCommunication::NONE, "None"},
        {SerialCommunication::EVEN, "Even"},
        {SerialCommunication::ODD, "Odd"}
    };
    const QMap<int, QString>  stopBitsSerialToStrLUT = {
        {SerialCommunication::ONE, "1"},
        {SerialCommunication::TWOO, "2"}
    };
};
#endif // MDIO_H
