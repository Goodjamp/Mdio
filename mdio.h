#ifndef MDIO_H
#define MDIO_H

#include <QVBoxLayout>
#include <QMainWindow>
#include <QThread>
#include <QDebug>
#include <QVector>
#include <QRandomGenerator>
#include <QSpacerItem>
#include <QThread>
#include <QSemaphore>
#include <QTimer>
#include <QButtonGroup>
#include <QWidget>
#include <QJsonObject>
#include <QComboBox>
#include <QToolTip>

#include "communication.h"
#include "tccontrol.h"
#include "tssettings.h"
#include "tsstatus.h"
#include "dialogconnectionsettings.h"
#include "tspollswitcher.h"

QT_BEGIN_NAMESPACE
namespace Ui { class Mdio; }
QT_END_NAMESPACE

#define COMMUNICATION_COMPLETE_TIMEOUTE    2000
#define OPEN_PORT_TIMEOUTE                 6000
#define READ_SLAVE_STATE_PERIOD_MS         1000

#define VALUE_IN_RANGE(value, min, max)    (((value) >= (min)) && ((value) <= (max)))

class Mdio : public QMainWindow
{
    Q_OBJECT

public:
    Mdio(QWidget *parent = nullptr);
    ~Mdio();
    static void verifyAndModifyNumber(QLineEdit *item, int min, int max, int def);
    static void verifyNumber(QLineEdit *item, int min, int max);

signals:
    void connectSlave(std::function<void(bool result)> cb,
                      QString port, int baudRate,
                      SerialCommunication::SerialPortParity parity,
                      SerialCommunication::SerialPortStopBits stopBits,
                      int readReplyTimeoute);
    void disconnectSlave();
    void readConfiguration(std::function<void(bool result, Communication::SlaveConfiguration settings)> cb,
                           int slaveAddress);
    void writeConfiguration(std::function<void(bool result)> cb, int slaveAddress,
                            Communication::SlaveConfiguration configuration);
    void reload(std::function<void(bool result)> cb,
                int slaveAddress);
    void readState(std::function<void(bool result, Communication::SlaveState state)> cb,
                   int slaveAddress);
    void setTeleControlPuls(std::function<void(bool result)> cb,
                            int slaveAddress, bool enable);
    void setTeleControl(std::function<void(bool result)> cb,
                        int slaveAddress, int index, bool enable);
    void readMetaInformation(std::function<void(bool result, Communication::MetaInformation metaInformation)> cb,
                             int slaveAddress);

private:
    bool connectWithSettings();
    void enableSettingsControl();
    void disableSettingsControl();
    //void skipAllSettings();
    void getDefaultPcConnectionSettings();
    void initCustomUi();
    void addToolTip();
    void updateUiConnectionStatusStr(void);
    void updateUiDeviceMetaInfStr(void);
    void updateUiCommunicationStatisticStr(void);
    bool updateUiConfiguration(void);
    void errorMessage(QString headr, QString detailed);
    bool processingCommunicatitonResult(QString headr, QString detailed, bool openPort);
    void resetSlaveInformation(void);
    void addTsConfigBinaryGroupUi(void);
    void addTsStatusBinaryGroupUi(void);
    void readSettings();

    /*
     * The groupe of CB functin from the Communication class
     */
    void connectSlaveResult(bool result);
    void readMetaInformationResult(bool result, Communication::MetaInformation metaInformation);
    void readConfigurationResult(bool result, Communication::SlaveConfiguration configuration);
    void reloadResult(bool result);
    void writeConfigurationResult(bool result);
    void readStateResult(bool result, Communication::SlaveState state);
    void setTeleControlResult(bool result);
    void setDefaultSettings();

private slots:
    /*
     * The applyConnectionSettings is use to pass the user connection settings
     * from the DialogConectionSettings to the UI space. Also
     * this slot is use to set variable readStateResult.
     */
    void saveConnectionSettings(DialogConnectionSettings::UserSettingsList);

    void readSlaveState(void);

    void tcSetTcSlot(int index, bool enable);

private slots:

    void on_pbConnectionSettings_clicked();

    void on_pbApplySettings_clicked();

    void on_pbDisconnect_clicked();

    void on_pbReload_clicked();

    void on_pbReadSettings_clicked();

    void on_cbTsType_currentIndexChanged(int index);

    void updateUiStateSlot(bool result, Communication::SlaveState state);

    void on_pbSetDefaultSettings_clicked();

    void on_pbConnect_clicked();

    void on_cbBaudRate_currentIndexChanged(int index);

/*
 * The pair of the signal/slot updateUiStateSignal/updateUiStateSlot is used to
 * pass the results of reading the state of slave from the CB
 * function, called from the external thread to the Mdio thread.
 */
    void on_pbOpenSettingsFile_clicked();

    void on_pbSaveSettingsFile_clicked();

    void on_leSilentInterval_editingFinished();

    void on_leReplyDelay_editingFinished();

    void on_leSilentInterval_textEdited(const QString &arg1);

    void on_leReplyDelay_textEdited(const QString &arg1);

    void on_leDebounceInterval_editingFinished();

    void on_leDebounceInterval_textEdited(const QString &arg1);

    void on_leBinaryTsSwitchTime_editingFinished();

    void on_leBinaryTsSwitchTime_textEdited(const QString &arg1);

    void on_lePulsDuration_editingFinished();

    void on_lePulsDuration_textEdited(const QString &arg1);

private:
    signals:
    void updateUiStateSignal(bool result, Communication::SlaveState state);

private:
    Ui::Mdio *ui;
    Communication *communicaiton;
    QVector<TcControl *> tcMonitorList;
    TsPollSwitcher *tsPollingSwitcher;
    QVector<TsSettings *> tsSetingsList;
    QVector<TsStatus *> tsStatus;
    QRandomGenerator rand;
    QSpacerItem *tcLayoutSpacer;
    QSpacerItem *tsLayoutSpacer;
    QSpacerItem *tsStatusLayoutSpacer;
    QThread *commmunicationThread;
    QTimer *readStateTimer;
    QButtonGroup *relayControlListTc1;
    QButtonGroup *relayControlListTc2;
    QVector<QWidget *> settingsItemsList;
    QVector<QFrame *> teleSignalConfigBinaryFrameList;
    QVector<QVBoxLayout *> teleSignalConfigBinaryLayoutList;
    QVector<QComboBox *> tsTypeConfigList;
    QVector<QFrame *> teleSignalStatusBinaryFrameList;
    QVector<QVBoxLayout *> teleSignalStatusBinaryLayoutList;
    QToolTip *debounceToolTip;
    int stateRequestCnt;
    int stateReplyCnt;

    bool needConnectSlave;
    bool isSlaveConnect;
    bool updateTcButtonState;

    /*
     * Connect to slave port (COM port) settings
     */
    QString connectPort;
    int connectBr;
    SerialCommunication::SerialPortParity connectParity;
    SerialCommunication::SerialPortStopBits connectStopBits;
    int connectSlaveAddress;
    int connectSilentInterval;
    int connectReplyTimeout;

    /*
     * Slave meta information
     */
    uint connectDeviceVersion;
    uint lastConfigurationYear;
    uint lastConfigurationMonth;
    uint lastConfigurationDay;

    /*
     * UI test listd
     */
    QStringList brStrList;
    QStringList parityStr;
    QStringList stopBitsStrList;

    /*
     * Services variable. Used to move readed settings from CallBack method to the showUimethod
     */
    Communication::SlaveConfiguration connectDeviceConf;

    /*
     * Last connected port configyration
     */
    DialogConnectionSettings::UserSettingsList lastConnectionUserSettings;

    /*
     * Services varisbles
     */
    QSemaphore communicationSyncSem{1};
    bool communicationResult;

    const QMap<QString, SerialCommunication::SerialPortParity> parityUiToSerilaLUT{
            {"None", SerialCommunication::NONE},
            {"Even", SerialCommunication::EVEN},
            {"Odd", SerialCommunication::ODD}
        };

    const QMap<QString, SerialCommunication::SerialPortStopBits> stopUiToSerilaLUT{
            {"1", SerialCommunication::ONE},
            {"2", SerialCommunication::TWOO}
        };
};
#endif // MDIO_H
