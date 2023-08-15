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
#include <QTimer>
#include "communication.h"
#include "tccontrol.h"
#include "tssettings.h"
#include "tsstatus.h"

QT_BEGIN_NAMESPACE
namespace Ui { class Mdio; }
QT_END_NAMESPACE

#define DEFAULT_CONNECT_BR_INDEX           0
#define DEFAULT_CONNECT_PARITY_INDEX       0
#define DEFAULT_CONNECT_STOP_BITS_INDEX    0
#define DEFAULT_CONNECT_SLAVE_ADDRESS      1
#define DEFAULT_CONNECT_VERSION            0
#define DEFAULT_CONNECT_YEAR               0
#define DEFAULT_CONNECT_MONTH              0
#define DEFAULT_CONNECT_DATE               0
#define COMMUNICATION_COMPLETE_TIMEOUTE    2000
#define SILENT_INTERVAL_MIN_MS             0
#define SILENT_INTERVAL_MAX_MS             100
#define REPLAY_DELAY_MIN_MS                0
#define REPLAY_DELAY_MAX_MS                100
#define DEBOUNCE_INTARVAL_MIN_MS           20
#define DEBOUNCE_INTARVAL_MAX_MS           100
#define PULS_DURATION_MIN_MS               100
#define PULS_DURATION_MMAX_MS              4000
#define READ_STATE_PERIOD_MS               1000

#define VALUE_IN_RANGE(value, min, max)    (((value) >= (min)) && ((value) <= (max)))

class Mdio : public QMainWindow
{
    Q_OBJECT

public:
    Mdio(QWidget *parent = nullptr);
    ~Mdio();

signals:
    void connectSlave(std::function<void(bool result)> cb,
                      QString port, int baudRate,
                      SerialCommunication::SerialPortParity parity,
                      SerialCommunication::SerialPortStopBits stopBits);
    void disconnectSlave();
    void readConfiguration(std::function<void(bool result, Communication::SlaveConfiguration settings)> cb,
                           int slaveAddress);
    void writeConfiguration(std::function<void(bool result)> cb, int slaveAddress,
                            Communication::SlaveConfiguration configuration);
    void reload(std::function<void(bool result)> cb,
                int slaveAddress);
    void readState(std::function<void(bool result, Communication::SlaveState state)> cb,
                   int slaveAddress);
    void setTeleControl(int slaveAddress, int index, bool enable);
    void readMetaInformation(std::function<void(bool result, int fwVersion)> cb,
                             int slaveAddress);

private:
    void initCustomUi();
    void updateUiConnectionStatusStr(void);
    void updateUiDeviceMetaInfStr(void);
    void updateUiCommunicationStatisticStr(void);
    bool updateUiConfiguration(void);
    void errorMessage(QString headr, QString detailed);
    bool processingCommunicatitonResult(QString headr, QString detailed);
    void resetSlaveInformation(void);

    /*
     * The groupe of CB functin from the Communication class
     */
    void connectSlaveResult(bool result);
    void readMetaInformationResult(bool result, int fwVersion);
    void readConfigurationResult(bool result, Communication::SlaveConfiguration configuration);
    void reloadResult(bool result);
    void writeConfigurationResult(bool result);
    void readStateResult(bool result, Communication::SlaveState state);


private slots:
    /*
     * The applyConnectionSettings is use settings set by the user over the dialog to
     * connect to the slave readStateResult
     */
    void applyConnectionSettings(QVector<int>);

    void readSlaveState(void);

private slots:

    void on_pbConnectionSettings_clicked();

    void on_pbApplySettings_clicked();

    void on_pbDisconnect_clicked();

    void on_pbReload_clicked();

    void on_pbReadSettings_clicked();

/*
 * The pair of the signal/slot updateUiStateSignal/updateUiStateSlot is used to
 * pass the results of reading the state of slave from the CB
 * function, called from the external thread to the Mdio thread.
 */
private:
    signals:
    void updateUiStateSignal(bool result, Communication::SlaveState state);

private slots:
    void updateUiStateSlot(bool result, Communication::SlaveState state);


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
    QTimer *readStateTimer;
    int stateRequestCnt;
    int stateReplyCnt;

    bool needConnectSlave;
    bool isSlaveConnect;
    uint connectPortIndex;
    uint connectBrIndex;
    uint connectParityIndex;
    uint connectStopBitsIndex;
    uint connectSlaveAddress;
    uint connectDeviceVersion;
    uint connectDeviceConfYear;
    uint connectDeviceConfMonth;
    uint connectDeviceConfDay;

    Communication::SlaveConfiguration connectDeviceConf;

    uint configBr;
    uint configParity;
    uint configStop;
    QSemaphore communicationSyncSem{1};
    bool communicationResult;

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
    const QMap<SerialCommunication::SerialPortParity, QString>  paritySerialToStrLUT = {
        {SerialCommunication::NONE, "None"},
        {SerialCommunication::EVEN, "Even"},
        {SerialCommunication::ODD, "Odd"}
    };
    const QMap<SerialCommunication::SerialPortStopBits, QString>  stopBitsSerialToStrLUT = {
        {SerialCommunication::ONE, "1"},
        {SerialCommunication::TWOO, "2"}
    };
};
#endif // MDIO_H
