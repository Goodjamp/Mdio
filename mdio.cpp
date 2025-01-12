#include "mdio.h"
#include "ui_mdio.h"


#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFile>
#include <QDebug>
#include <QMap>
#include <QDate>
#include <QThread>
#include <QMessageBox>
#include <QValidator>
#include <QByteArray>
#include <QRegularExpression>
#include <QFrame>
#include <QFile>
#include <QFileDialog>
#include <dialogconnectionsettings.h>
#include <modbusrtumaster.h>
#include "Version.h"
#include "SwDefSettings.h"
#include "SwSettings.h"

#define SETTINGS_FILE_EXTANTION    ".json"
#define SETTINGS_FILE_NAME         "MBB"
#define STR_CAST(str)              static_cast<QString>(str)
#define SW_NAME                    STR_CAST("МВВ-4-2 конфігуратор")
#define TS_TEXT_SIMPLE             "Одинарні"
#define TS_TEXT_BINARY             "Подвійні"

void Mdio::getDefaultPcConnectionSettings()
{
    /*
     * Default (initial) connection settings
     */
    lastConnectionUserSettings.br = SwDefSettings::getBrDefaultString();
    lastConnectionUserSettings.parity = SwDefSettings::getParityDefault();
    lastConnectionUserSettings.stopBits = SwDefSettings::getStopBitsDefaultStr();
    lastConnectionUserSettings.address = SwDefSettings::getAddressDefault();
    lastConnectionUserSettings.replyTimeout = SwDefSettings::getTimeoutReplyDefaultPc();
    lastConnectionUserSettings.silentInterval = SwDefSettings::getSilentIntervalDefaultPc();
    lastConnectionUserSettings.port = "";
}

/*
 * Device connected
 */
void Mdio::enableSettingsControl()
{
    foreach(auto item,  settingsItemsList) {
        item->setEnabled(true);
    }
    ui->leBinaryTsSwitchTime->setEnabled(false);
    ui->pbConnect->setEnabled(false);
    ui->pbConnectionSettings->setEnabled(false);
}

void Mdio::disableSettingsControl()
{
    foreach(auto item,  settingsItemsList) {
        item->setEnabled(false);
    }
    ui->pbConnect->setEnabled(true);
    ui->pbConnectionSettings->setEnabled(true);
}

void Mdio::addTsConfigBinaryGroupUi(void)
{
    QHBoxLayout *serviceLayoute;
    QSize size(120, 25);

    /*
     * Add the Tele Signalisation configuration.
     * The combination of 2 TS sinals represents the Binary Ts.
     */
    for (uint32_t k = 0; k < TELESIGNAL_NUMBERS; k++) {
        teleSignalConfigBinaryFrameList.push_back(new QFrame());

        /*
         * Apply style to the TS frame as Simple TS (by default)
         */
        teleSignalConfigBinaryFrameList.last()->setProperty("tsBinary", false);
        teleSignalConfigBinaryFrameList.last()->style()->unpolish(teleSignalConfigBinaryFrameList.last());
        teleSignalConfigBinaryFrameList.last()->style()->polish(teleSignalConfigBinaryFrameList.last());

        teleSignalConfigBinaryLayoutList.push_back(new QVBoxLayout);
        teleSignalConfigBinaryFrameList.last()->setLayout(teleSignalConfigBinaryLayoutList.last());

        ui->vlTeleSignalSettings->addWidget(teleSignalConfigBinaryFrameList.last());

        /*
         * Add TS settings configuration #k
         */
        tsSetingsList.append(new TsSettings(k + 1));
        teleSignalConfigBinaryLayoutList.last()->addWidget(tsSetingsList.last());
        k++;

        /*
         * Add simple/binary TS settings type
         */
        tsTypeConfigList.append(new QComboBox());
        tsTypeConfigList.last()->addItems({TS_TEXT_SIMPLE, TS_TEXT_BINARY});
        tsTypeConfigList.last()->setStyleSheet("font: bold 12px;");
        tsTypeConfigList.last()->setFixedSize(size);
        tsTypeConfigList.last()->setCurrentIndex(-1);
        serviceLayoute = new QHBoxLayout();
        serviceLayoute->addItem(new QSpacerItem(20, 0, QSizePolicy::Expanding, QSizePolicy::Expanding));
        serviceLayoute->addWidget(tsTypeConfigList.last());
        teleSignalConfigBinaryLayoutList.last()->addLayout(serviceLayoute);

        /*
         * Add TS settings configuration #(k + 1)
         */
        tsSetingsList.append(new TsSettings(k + 1));
        teleSignalConfigBinaryLayoutList.last()->addWidget(tsSetingsList.last());
    }

    foreach(auto item, tsTypeConfigList) {
        connect(item, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &Mdio::on_cbTsType_currentIndexChanged);
    }

    tsLayoutSpacer = new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Expanding);
    ui->vlTeleSignalSettings->addItem(tsLayoutSpacer);
}


void Mdio::addTsStatusBinaryGroupUi(void)
{
    tsPollingSwitcher = new TsPollSwitcher();
    ui->vlTeleSignalStatus->addWidget(tsPollingSwitcher);

    for (uint32_t k = 0; k < TELESIGNAL_NUMBERS; k++) {
        teleSignalStatusBinaryFrameList.push_back(new QFrame());

        /*
         * Apply style to the TS frame as Simple TS (by default)
         */
        teleSignalStatusBinaryFrameList.last()->setProperty("tsBinary", false);
        teleSignalStatusBinaryFrameList.last()->style()->unpolish(teleSignalStatusBinaryFrameList.last());
        teleSignalStatusBinaryFrameList.last()->style()->polish(teleSignalStatusBinaryFrameList.last());


        teleSignalStatusBinaryLayoutList.push_back(new QVBoxLayout);
        teleSignalStatusBinaryFrameList.last()->setLayout(teleSignalStatusBinaryLayoutList.last());


        ui->vlTeleSignalStatus->addWidget(teleSignalStatusBinaryFrameList.last());

        /*
         * Add TS status number K
         */
        tsStatus.append(new TsStatus(k + 1));
        teleSignalStatusBinaryLayoutList.last()->addWidget(tsStatus.last());
        k++;


        /*
         * Add TS status number K + 1
         */
        tsStatus.append(new TsStatus(k + 1));
        teleSignalStatusBinaryLayoutList.last()->addWidget(tsStatus.last());
    }

    tsStatusLayoutSpacer = new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Expanding);
    ui->vlTeleSignalStatus->addItem(tsStatusLayoutSpacer);
}


void Mdio::on_cbTsType_currentIndexChanged(int index)
{
    QObject *senderObj = sender();
    bool isSimpleTs = false;

    for (int k = 0; k < tsTypeConfigList.size(); k++) {
        if (tsTypeConfigList[k]->currentText() == TS_TEXT_BINARY) {
            isSimpleTs = true;
        }
        if (senderObj == tsTypeConfigList[k]) {
            /*
             * Apply style according to the TS state
             */
            teleSignalConfigBinaryFrameList[k]->setProperty("tsBinary", index == 1);
            teleSignalConfigBinaryFrameList[k]->style()->unpolish(teleSignalConfigBinaryFrameList[k]);
            teleSignalConfigBinaryFrameList[k]->style()->polish(teleSignalConfigBinaryFrameList[k]);

            teleSignalStatusBinaryFrameList[k]->setProperty("tsBinary", index == 1);
            teleSignalStatusBinaryFrameList[k]->style()->unpolish(teleSignalStatusBinaryFrameList[k]);
            teleSignalStatusBinaryFrameList[k]->style()->polish(teleSignalStatusBinaryFrameList[k]);
        }
    }

    /*
     * Update active state of the BinaryTsSwitchingTime configuration
     */
     ui->leBinaryTsSwitchTime->setEnabled(isSimpleTs);
}

void Mdio::addToolTip()
{
    ui->leBinaryTsSwitchTime->setToolTip("За умовчуванням " + SwDefSettings::getBinaryTsSwitchTimeDefaultString() + " мс\n"
                                         + "Допустимй діапазон: "
                                         + SwDefSettings::getSwitchTimeMinString()
                                         + "-"
                                         + SwDefSettings::getSwitchTimeMaxString() + " мс");
    ui->leDebounceInterval->setToolTip("За умовчуванням " + SwDefSettings::getDebounceDefaultString() + " мс\n"
                                       + "Допустимй діапазон: "
                                       + SwDefSettings::getDebounceMinString()
                                       + "-"
                                       + SwDefSettings::getDebounceMaxString() + " мс");
    ui->lePulsDuration->setToolTip("За умовчуванням " + SwDefSettings::getPulsDurationDefaultString() + " мс");
    ui->leReplyDelay->setToolTip("За умовчуванням " + SwDefSettings::getTimeoutReplyDefaultString());
    ui->cbBaudRate->setToolTip("За умовчуванням " + SwDefSettings::getBrDefaultString() + " біт/с");
    ui->cbParity->setToolTip("За умовчуванням " + SwDefSettings::getParityDefault());
    ui->cbStopBits->setToolTip("За умовчуванням " + SwDefSettings::getStopBitsDefaultStr());
}

void Mdio::initCustomUi()
{
    /*
     *  Title bar: icon name
     */
    setWindowTitle(SW_NAME + " V"
                   + SW_VERSION_STR);
    setWindowIcon((QIcon)":/Resources/CompanyIcon.png");

    relayControlListTc1 = new QButtonGroup();
    relayControlListTc2 = new QButtonGroup();
    QRegularExpressionValidator *numericValidator3D = new QRegularExpressionValidator((QRegularExpression)"^$|[\\d]{1,3}", this);
    QRegularExpressionValidator *numericValidator4D = new QRegularExpressionValidator((QRegularExpression)"^$|[\\d]{1,4}", this);
    QRegularExpressionValidator *numericValidator5D = new QRegularExpressionValidator((QRegularExpression)"^$|[\\d]{1,5}", this);

    addToolTip();

    /*
     * Add validation to the numeric UI items
     */
    ui->leSilentInterval->setValidator(numericValidator3D);
    ui->leReplyDelay->setValidator(numericValidator3D);
    ui->leDebounceInterval->setValidator(numericValidator3D);
    ui->lePulsDuration->setValidator(numericValidator4D);
    ui->leBinaryTsSwitchTime->setValidator(numericValidator5D);

    /*
     * Add TeleControl status/control items
     */
    ui->lTcPulsDurationRange->setText("("
                                      + SwDefSettings::getPulsDurationMinString()
                                      + "-"
                                      + SwDefSettings::getPulsDurationMaxString()
                                      + ")");
    for (uint32_t k = 0; k < TELECONTROL_TOTAL_NUMBERS; k++) {
        tcMonitorList.append(new TcControl("", k, this));
        ui->vlTcControlMonitorInternal->addWidget(tcMonitorList.last());
    }

    tcLayoutSpacer = new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Expanding);
    ui->vlTcControlMonitorInternal->addItem(tcLayoutSpacer);

    QVector<SwDefSettings::MonitorDesr> monitorDescriptor = SwDefSettings::getMonitorDescr();
    for (int k = 0; k < SwDefSettings::getMonitorDescr().size(); k++) {
        tcMonitorList[k]->setName(monitorDescriptor[k].name);
        tcMonitorList[k]->setTextStateList(monitorDescriptor[k].rel1State, monitorDescriptor[k].rel2State);
    }

    /*
     * Add TeleSignalisation configuration items
     */
    ui->lTsDebounceTimeRange->setText("("
                                      + SwDefSettings::getDebounceMinString()
                                      + "-"
                                      + SwDefSettings::getDebounceMaxString()
                                      + ")");
    ui->lTsDoubleSwitchTimeRange->setText("("
                                          + SwDefSettings::getSwitchTimeMinString()
                                          + "-"
                                          + SwDefSettings::getSwitchTimeMaxString()
                                          + ")");
    addTsConfigBinaryGroupUi();

    /*
     * Add TeleSignalisation status items
     */

    addTsStatusBinaryGroupUi();

    brStrList = SwDefSettings::getBrListString();
    parityStr = SwDefSettings::getParityListString();
    stopBitsStrList = SwDefSettings::getStopBitListString();

    ui->cbBaudRate->addItems(brStrList);
    ui->cbStopBits->addItems(stopBitsStrList);
    ui->cbParity->addItems(parityStr);

    ui->pbVoltageOnTcStatus->setEnabled(false);
    ui->pbEepromStatus->setEnabled(false);
    ui->pbEepromClearStatus->setEnabled(false);

    /*
     * Add all UI element to control enabling
     */
    settingsItemsList.push_back(static_cast<QWidget *>(ui->pbApplySettings));       // Write to device
    settingsItemsList.push_back(static_cast<QWidget *>(ui->pbReadSettings));        // Read from the device
    settingsItemsList.push_back(static_cast<QWidget *>(ui->pbReload));              // Reload
    settingsItemsList.push_back(static_cast<QWidget *>(ui->pbDisconnect));          //

    foreach(auto item, tcMonitorList) {
        settingsItemsList.push_back(static_cast<QWidget *>(item->getOnButtonPointer()));
        settingsItemsList.push_back(static_cast<QWidget *>(item->getOffButtonPointer()));
    }

    setDefaultSettings();
    ui->tabWidget->setCurrentIndex(0);
    disableSettingsControl();
}


Mdio::Mdio(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::Mdio)
{
    ui->setupUi(this);


    commmunicationThread = new QThread();
    communicaiton = new Communication();
    readStateTimer = new QTimer();
    readStateTimer->setInterval(READ_SLAVE_STATE_PERIOD_MS);

    connect(readStateTimer, &QTimer::timeout, this, &Mdio::readSlaveState);

    /*
     * Read JSON with settings
     */
    getDefaultPcConnectionSettings();
    resetSlaveInformation();
    initCustomUi();
    updateUiConnectionStatusStr();
    updateUiDeviceMetaInfStr();

    connect(this, &Mdio::connectSlave, communicaiton, &Communication::connectSlaveSlot);
    connect(this, &Mdio::disconnectSlave, communicaiton, &Communication::disconnectSlaveSlot);
    connect(this, &Mdio::readConfiguration, communicaiton, &Communication::readConfigurationSlot);
    connect(this, &Mdio::writeConfiguration, communicaiton, &Communication::writeConfigurationSlot);
    connect(this, &Mdio::reload, communicaiton, &Communication::reloadSlot);
    connect(this, &Mdio::readState, communicaiton, &Communication::readStateSlot);
    connect(this, &Mdio::readMetaInformation, communicaiton, &Communication::readMetaInformationSlot);
    connect(this, &Mdio::setTeleControl, communicaiton, &Communication::setTeleControlSlot);
    connect(this, &Mdio::setTeleControlPuls, communicaiton, &Communication::setTeleControlPulsSlot);

    /*
     * Add slots to processing tele control commands
     */
    foreach(auto tcControlItem, tcMonitorList)
    {
        connect(tcControlItem, &TcControl::setState, this, &Mdio::tcSetTcSlot);
    }

    /*
     * Runing communication class on the dedicated thread.
     * All internal variables (for example class serialCommunication and class modbus) should be
     * created after start thread. For this the communicatin class has dedicated slot startCommunication.
     * This slot connected to the *started* signal of the QThread class. The  *started* signal emit
     * immidiatly after start thread.
     */
    connect(commmunicationThread, &QThread::started, communicaiton, &Communication::startCommunication);
    /*
     * connect(worker, finished, thread, quit);
     * connect(worker, finished, worker, deleteLater);
     * connect(thread, finished, thread, deleteLater);
     */
    communicaiton->moveToThread(commmunicationThread);
    commmunicationThread->start();

    /*
     * Take semaphjre.
     * Remove this string on the future
     */
    communicationSyncSem.acquire();

    /*
     * The pair of the signal/slot updateUiStateSignal/updateUiStateSlot is used to
     * pass the results of reading the state of slave from the CB
     * function, called from the external thread to the Mdio thread.
     */
    connect(this, &Mdio::updateUiStateSignal, this, &Mdio::updateUiStateSlot);
}

Mdio::~Mdio()
{
    delete ui;
}

void Mdio::updateUiConnectionStatusStr(void)
{
    QString connectionSettingsStr;

    if (isSlaveConnect == true) {
        connectionSettingsStr = connectPort
                                + " "
                                + lastConnectionUserSettings.br
                                + " "
                                + "8"
                                + lastConnectionUserSettings.parity
                                + lastConnectionUserSettings.stopBits
                                + " "
                                + "Адр."
                                + lastConnectionUserSettings.address;
    } else {
        connectionSettingsStr = "ВІД'ЄДНАНИЙ";
    }

    ui->lConnectionSettings->setText(connectionSettingsStr);
}

void Mdio::updateUiDeviceMetaInfStr(void)
{
    QString metaInfStr;

    metaInfStr = "v"
                 + QString::number(connectDeviceVersion)
                 + "  Дата:"
                 + QString::number(lastConfigurationDay) + "."
                 + QString::number(lastConfigurationMonth) + "."
                 + QString::number(lastConfigurationYear);


    ui->lDeviceMetaInfo->setText(metaInfStr);
}

bool Mdio::updateUiConfiguration(void)
{
    if (brStrList.contains(QString::number(connectDeviceConf.communication.baudRate)) == false) {\
        errorMessage("Помилка конфігурації", "Помилка швидкості");
        return false;
    }
    if (parityUiToSerilaLUT.key(connectDeviceConf.communication.parity, "Error") == "Error") {
        errorMessage("Помилка конфігурації", "Помилка паритету");
        return false;
    }
    if (stopUiToSerilaLUT.key(connectDeviceConf.communication.stopBits, "Error") == "Error") {
        errorMessage("Помилка конфігурації", "Помилка стоп-біт");
        return false;
    }

    /*
     * Show configuration on the UI
     */
    ui->cbBaudRate->setCurrentText(QString::number(connectDeviceConf.communication.baudRate));
    ui->cbParity->setCurrentText(parityUiToSerilaLUT.key(connectDeviceConf.communication.parity, "Error"));
    ui->cbStopBits->setCurrentText(stopUiToSerilaLUT.key(connectDeviceConf.communication.stopBits, "Error"));
    ui->leReplyDelay->setText(QString::number(connectDeviceConf.communication.replyDelay));
    ui->leSilentInterval->setText(QString::number(connectDeviceConf.communication.silentInterval));
    ui->leDebounceInterval->setText(QString::number(connectDeviceConf.signalisation.debounsInterval));
    for (uint32_t k = 0; k < TELESIGNAL_NUMBERS; k++) {
        tsSetingsList[k]->setInvert(connectDeviceConf.signalisation.isInvers[k]);
    }
    ui->leBinaryTsSwitchTime->setText(QString::number(connectDeviceConf.signalisation.binaryTsSwitchingTime));


    for (uint32_t k = 0; k < TELESIGNAL_BINARY_NUMBERS; k++) {
        /*
         * Set the tsTypeConfigList[k] current index equal to -1 to guranted that the on_cbTsType_currentIndexChanged CB
         * will be called
         */
        tsTypeConfigList[k]->setCurrentIndex(-1);
        tsTypeConfigList[k]->setCurrentText(connectDeviceConf.signalisation.isBinary[k]
                                             ? TS_TEXT_BINARY
                                             : TS_TEXT_SIMPLE);
    }
    ui->lePulsDuration->setText(QString::number(connectDeviceConf.control.pulsDuration));

    lastConfigurationYear = connectDeviceConf.configurationYear;
    lastConfigurationMonth = connectDeviceConf.configurationMonth;
    lastConfigurationDay = connectDeviceConf.configurationDay;

    return true;
}

void Mdio::errorMessage(QString headr, QString detailed)
{
    QMessageBox *errorAddressMessage = new QMessageBox(QMessageBox::Critical,
                                                       headr,
                                                       detailed,
                                                       QMessageBox::Ok,
                                                       this);
    errorAddressMessage->setWindowIcon((QIcon)":/Resources/CompanyIcon.png");
    errorAddressMessage->show();
}

void Mdio::resetSlaveInformation(void)
{
    connectDeviceVersion = 0;
    lastConfigurationYear = 0;
    lastConfigurationMonth = 0;
    lastConfigurationDay = 0;
    isSlaveConnect = false;
}

void Mdio::saveConnectionSettings(DialogConnectionSettings::UserSettingsList connectionSettings)
{
    connectPort = connectionSettings.port;
    connectBr = connectionSettings.br.toInt();
    connectParity = parityUiToSerilaLUT.value(connectionSettings.parity);
    connectStopBits = stopUiToSerilaLUT.value(connectionSettings.stopBits);
    connectSlaveAddress = connectionSettings.address.toInt();
    connectSilentInterval = connectionSettings.silentInterval.toInt();
    connectReplyTimeout = connectionSettings.replyTimeout.toInt();

    /*
     * Caching user settings for the nex DialogConnectSettings calling
     */
    lastConnectionUserSettings = connectionSettings;

    needConnectSlave = true;
}

bool Mdio::processingCommunicatitonResult(QString headr, QString detailed, bool openPort)
{
    if (communicationSyncSem.tryAcquire(1, openPort
                                           ? OPEN_PORT_TIMEOUTE
                                           : COMMUNICATION_COMPLETE_TIMEOUTE)
        == false) {
        errorMessage(headr, "Апаратний збій");
        return false;
    };
    if (communicationResult == false) {
        errorMessage(headr, detailed);
        return false;
    }

    return true;
}

void Mdio::connectSlaveResult(bool result)
{
    /*
     * Release (give) semaphore to unblok code that waite to complete
     */
    communicationResult = result;
    if (result == false) {
        qDebug()<<"Error connect";
    }

    communicationSyncSem.release(1);

}

void  Mdio::writeConfigurationResult(bool result)
{
    /*
     * Release (give) semaphore to unblok code that waite to complete
     */
    communicationResult = result;

    if (result == false) {
        qDebug()<<"Error write Configuration";
    }
    communicationSyncSem.release(1);
}

void Mdio::setTeleControlResult(bool result)
{
    /*
     * Release (give) semaphore to unblok code that waite to complete
     */
    communicationResult = result;
    if (result == false) {
        qDebug()<<"Error set tele control";
    }

    communicationSyncSem.release(1);
}

void Mdio::setDefaultSettings()
{
    ui->cbBaudRate->setCurrentText(SwDefSettings::getBrDefaultString());
    ui->cbParity->setCurrentText(SwDefSettings::getParityDefault());
    ui->cbStopBits->setCurrentText(SwDefSettings::getStopBitsDefaultStr());
    ui->leSilentInterval->setText(SwDefSettings::getSilentIntervalDefaultString());
    ui->leReplyDelay->setText(SwDefSettings::getTimeoutReplyDefaultString());
    ui->leDebounceInterval->setText(SwDefSettings::getDebounceDefaultString());
    ui->lePulsDuration->setText(SwDefSettings::getPulsDurationDefaultString());
    ui->leBinaryTsSwitchTime->setText(SwDefSettings::getBinaryTsSwitchTimeDefaultString());
    foreach(auto item, tsSetingsList) {
        item->setInvert(false);
    }
    foreach(auto item, tsTypeConfigList) {
        item->setCurrentIndex(0);
    }
}


void Mdio::readMetaInformationResult(bool result, Communication::MetaInformation metaInformation)
{
    /*
     * Release (give) semaphore to unblok code that waite to complete
     */
    communicationResult = result;
    if (result == true) {
        connectDeviceVersion = metaInformation.fwVersion;
        lastConfigurationDay = metaInformation.configurationDay;
        lastConfigurationMonth = metaInformation.configurationMonth;
        lastConfigurationYear = metaInformation.configurationYear;
    } else {
        qDebug()<<"Error read meta information ";
    }
    communicationSyncSem.release(1);
}

void  Mdio::readConfigurationResult(bool result, Communication::SlaveConfiguration configuration)
{
    /*
     * Release (give) semaphore to unblok code that waite to complete
     */
    communicationResult = result;

    if (result == true) {
        connectDeviceConf = configuration;
    } else {
        qDebug()<<"Error read configuration";
    }
    communicationSyncSem.release(1);
}


void Mdio::reloadResult(bool result)
{
    /*
     * Release (give) semaphore to unblok code that waite to complete
     */
    communicationResult = result;
    if (result == true) {
        /*
         * Do we need apply new communicaiotn settings ?
         */
    }
    communicationSyncSem.release(1);
}

void Mdio::readStateResult(bool result, Communication::SlaveState state)
{
    if (result == false) {
        qDebug()<<"Error read state";
    }

    emit this->updateUiStateSignal(result, state);
}

bool Mdio::connectWithSettings()
{
    QStringList comList = Communication::getPortsList();

    if (comList.contains(connectPort) == false) {
        errorMessage("Помилка конфігурації",
                     "СOM порт не доступний");
        return false;
    }

    emit connectSlave(CB_WRAP_1(Mdio, connectSlaveResult),
                      connectPort,
                      connectBr,
                      connectParity,
                      connectStopBits,
                      connectReplyTimeout + connectSilentInterval);
    /*
     * Waite to complete connection
     */
    if (processingCommunicatitonResult("Неможливо приєднатися",
                                        "Порт недоступний", true) == false) {
        emit disconnectSlave();
        return false;
    }

    /*
     * Read meta information
     */
    emit readMetaInformation(CB_WRAP_2(Mdio, readMetaInformationResult), connectSlaveAddress);
    if (processingCommunicatitonResult("Неможливо приєднатися",
                                       "Помилка зчитування метаінформації", false) == false) {
        emit disconnectSlave();
        return false;
    }

    /*
     * Read state only for update TC button state
     */
    updateTcButtonState = true;
    emit readState(CB_WRAP_2(Mdio, readStateResult), connectSlaveAddress);


    /*
     * Start timer to read the slave state
     */
    stateRequestCnt = 0;
    stateReplyCnt = 0;
    readStateTimer->start();

    isSlaveConnect = true;
    updateUiConnectionStatusStr();

    /*
     * Update last configuration date information
     */
    updateUiDeviceMetaInfStr();

    /*
     * Enable UI items
     */
    enableSettingsControl();

    return true;
}

void Mdio::on_pbConnectionSettings_clicked()
{
    DialogConnectionSettings::UiFilingList dialoConnectUiFillList;
    QStringList comList = Communication::getPortsList();

    dialoConnectUiFillList.comList = comList;
    if (comList.contains(lastConnectionUserSettings.port) == false) {
        lastConnectionUserSettings.port = comList.size() > 0 ? comList[0] : "";
    }

    DialogConnectionSettings dialogConnectionSettings(dialoConnectUiFillList,
                                                      lastConnectionUserSettings);

    dialogConnectionSettings.setModal(true);
    connect(&dialogConnectionSettings, &DialogConnectionSettings::applySettings, this, &Mdio::saveConnectionSettings);
    needConnectSlave = false;

    dialogConnectionSettings.exec();

    /*
     * If user push botton *Close*, we don't need continue connection to slave
     */
    if (needConnectSlave ==false) {
        return;
    }

    if (connectWithSettings()) {
        readSettings();
    }
}

void Mdio::verifyAndModifyNumber(QLineEdit *item, int min, int max, int def)
{
    QString text = item->text();

    if (text.isEmpty()) {
        item->setText(QString::number(def));
    } else if(text.toInt() < min) {
        item->setText(QString::number(min));
    } else if(text.toInt() > max) {
        item->setText(QString::number(max));
    }
}

void Mdio::verifyNumber(QLineEdit *item, int min, int max)
{
    QString text = item->text();

    if (text.isEmpty()
        || text.toInt() < min
        || text.toInt() > max) {
        item->setStyleSheet("background-color: rgb(243, 193, 255);");
    } else {
        item->setStyleSheet("background-color: rgb(255, 255, 255);");
    }
}

void Mdio::on_pbApplySettings_clicked()
{
    Communication::SlaveConfiguration configuration;


    /*
     * Read user configuration from the UI and serialiase it to the SlaveConfiguration
     * structure
     */

    /*
     *  Communication settings
     */
    configuration.communication.baudRate = ui->cbBaudRate->currentText().toInt();
    configuration.communication.parity = parityUiToSerilaLUT.value(ui->cbParity->currentText());
    configuration.communication.stopBits = stopUiToSerilaLUT.value(ui->cbStopBits->currentText());
    configuration.communication.replyDelay = ui->leReplyDelay->text().toInt();
    configuration.communication.silentInterval = ui->leSilentInterval->text().toInt();

    /*
     *  Tele Signalisation settings
     */
    configuration.signalisation.debounsInterval = ui->leDebounceInterval->text().toInt();
    for (uint32_t k = 0; k < TELESIGNAL_NUMBERS; k++) {
        configuration.signalisation.isInvers[k] = tsSetingsList[k]->isInvert();
    }

    /*
     *  Set the correct value of the binaryTsSwitchingTime. If the user configures any of the tele signalisation input to the Binary mode,
     *  the binaryTsSwitchingTime value will be overwritten by the user settings
     */
    configuration.signalisation.binaryTsSwitchingTime = SwDefSettings::getBinaryTsSwitchTimeDefault();
    for (uint32_t k = 0; k < TELESIGNAL_BINARY_NUMBERS; k++) {
        configuration.signalisation.isBinary[k] = (tsTypeConfigList[k]->currentText() == TS_TEXT_BINARY);
        if (configuration.signalisation.isBinary[k]) {
            configuration.signalisation.binaryTsSwitchingTime = ui->leBinaryTsSwitchTime->text().toInt();
        }

    }

    /*
     *  Tele Control settings
     */
    configuration.control.pulsDuration = ui->lePulsDuration->text().toInt();

    /*
     *  Configuration date
     */
    configuration.configurationYear = QDate::currentDate().year() - 2000;
    configuration.configurationMonth = QDate::currentDate().month();
    configuration.configurationDay = QDate::currentDate().day();

    emit writeConfiguration(CB_WRAP_1(Mdio, writeConfigurationResult), connectSlaveAddress, configuration);
    if (processingCommunicatitonResult("Оновлення конфігурації",
                                       "Помилка оновлення конфігурації", false) == false) {
        return;
    }
}

void Mdio::on_pbDisconnect_clicked()
{
    emit disconnectSlave();
    readStateTimer->stop();
    resetSlaveInformation();
    updateUiConnectionStatusStr();
    updateUiDeviceMetaInfStr();
    disableSettingsControl();
}

void Mdio::on_pbReload_clicked()
{
    /*
     * Clear the semaphore
     */
    communicationSyncSem.tryAcquire(1);
    emit reload(CB_WRAP_1(Mdio,reloadResult), connectSlaveAddress);
    if (processingCommunicatitonResult("Перезавантаження пристрою",
                                       "Помилка перезавантаження", false) == false) {
        return;
    }
}

void Mdio::readSettings()
{
    /*
     * Read configuration
     */
    emit readConfiguration(CB_WRAP_2(Mdio, readConfigurationResult), connectSlaveAddress);
    if (processingCommunicatitonResult("Зчитування конфігурації",
                                       "Помилка считування мета конфігурації", false) == false) {
        return;
    }

    updateUiConfiguration();
    updateUiDeviceMetaInfStr();
}

void Mdio::on_pbReadSettings_clicked()
{
    readSettings();
}

void Mdio::updateUiCommunicationStatisticStr(void)
{
    ui->lCommunicationStatistic->setText(QString::number(stateRequestCnt) + "/" + QString::number(stateReplyCnt));
}

void Mdio::updateUiStateSlot(bool result, Communication::SlaveState state)
{
    if (result == true) {
        /*
         * Update communicaiotn statistic indication
         */
        stateReplyCnt++;
        updateUiCommunicationStatisticStr();


        /*
         * Update Tele signalisation and status indication
         */
        TsPollSwitcher::PollingType pollingType = tsPollingSwitcher->getPollingType();
        for (uint32_t k = 0; k < TELESIGNAL_NUMBERS; k++) {
            if (state.error220 == false) {
                tsStatus[k]->setStatus((pollingType == TsPollSwitcher::POLLING_TYPE_REGISTER
                                        ? state.signalisation[k]
                                        : state.signalisationBinary[k])
                                        ? TsStatus::TS_OK_ON : TsStatus::TS_OK_OFF);
            } else {
                tsStatus[k]->setStatus(TsStatus::TS_ERROR);
            }

        }

        ui->pbVoltageOnTcStatus->setChecked(state.error220);
        ui->pbEepromStatus->setChecked(state.errorEeprom);
        ui->pbEepromClearStatus->setChecked(state.errorEepromClear);

        /*
         * Update Tele control indication
         *
         * The communicaiotn return 3 registers (1 puls and 2 static)
         */
        for (int k = 0; k < TELECONTROL_TOTAL_NUMBERS; k++) {
            tcMonitorList[k]->setStateTextIndication(state.control[k]);
        }

        /*
         * Update blinker
         */
        ui->pbConnectionStatus->setChecked(true);

        /*
         * If this function call first time after connection, we need update the state of the
         * button on the pannel of the TC control
         */
        if (updateTcButtonState == true) {
            updateTcButtonState = false;

            /*
             * For the puls TC type (TC0) we skip current state all the time
             */
            tcMonitorList[0]->setOnButtonState(false);
            tcMonitorList[0]->setOffButtonState(false);

            /*
             * For the other TC we need set actual state
             */
            for (int k = 1; k < TELECONTROL_TOTAL_NUMBERS; k++) {
                switch (state.control[k]) {
                case Communication::TELECONTROL_STATE_ON:
                    tcMonitorList[k]->setOnButtonState(true);
                    tcMonitorList[k]->setOffButtonState(false);
                    break;

                case Communication::TELECONTROL_STATE_OFF:
                    tcMonitorList[k]->setOnButtonState(false);
                    tcMonitorList[k]->setOffButtonState(true);
                    break;

                case Communication::TELECONTROL_STATE_UNDEFINED:
                    tcMonitorList[k]->setOnButtonState(false);
                    tcMonitorList[k]->setOffButtonState(false);
                    break;
                }
            }

        }
    }
}

void Mdio::readSlaveState(void)
{
    emit readState(CB_WRAP_2(Mdio, readStateResult), connectSlaveAddress);
    stateRequestCnt++;
    updateUiCommunicationStatisticStr();
    ui->pbConnectionStatus->setChecked(false);
}

void Mdio::tcSetTcSlot(int index, bool enable)
{
    if (index == 0) {
        tcMonitorList[1]->unchekAllButton();
        tcMonitorList[2]->unchekAllButton();
        emit this->setTeleControlPuls(CB_WRAP_1(Mdio, setTeleControlResult), connectSlaveAddress, enable);
    } else {
        tcMonitorList[0]->unchekAllButton();
        emit this->setTeleControl(CB_WRAP_1(Mdio, setTeleControlResult), connectSlaveAddress, index - 1, enable);\
    }

    if (processingCommunicatitonResult("Телекерування",
                                       "Помилка передачі команди\nдля статичного телекерування", false) == false) {
        return;
    }
}

void Mdio::on_pbSetDefaultSettings_clicked()
{
    setDefaultSettings();
}

void Mdio::on_pbConnect_clicked()
{
    if (connectWithSettings()) {
        readSettings();
    }
}

void Mdio::on_cbBaudRate_currentIndexChanged(int index)
{
    if (index >= SwDefSettings::getSilentIntervalDefaultListString().size()
        || index < 0) {
        return;
    }
    ui->leSilentInterval->setToolTip("За умовчуванням " + SwDefSettings::getSilentIntervalDefaultListString().at(index) + " мс");
    ui->leSilentInterval->setText(SwDefSettings::getSilentIntervalDefaultListString().at(index));
    ui->lSilentIntervalRange->setText(" ("
                                      + SwDefSettings::getSilentIntervalMinListString().at(index)
                                      + "-"
                                      + SwDefSettings::getSilentIntervalMaxListString().at(index)
                                      + ")");
}

void Mdio::on_pbSaveSettingsFile_clicked()
{
    CommunicationConfig comConfig(ui->cbBaudRate->currentText().toInt(), CommunicationConfig::PARITY_NONE,
                                  CommunicationConfig::STOP_BITS_1,
                                  ui->leSilentInterval->text().toInt(),
                                  ui->leReplyDelay->text().toInt());
    TsConfig tsConfig(ui->leBinaryTsSwitchTime->text().toInt(),
                      ui->leDebounceInterval->text().toInt(),
                      QVector<bool>{tsSetingsList[0]->isInvert(), tsSetingsList[1]->isInvert(), tsSetingsList[2]->isInvert(), tsSetingsList[3]->isInvert()},
                      QVector<bool>{tsTypeConfigList[0]->currentText() == TS_TEXT_BINARY, tsTypeConfigList[1]->currentText() == TS_TEXT_BINARY});
    TcConfig tcConfig(ui->lePulsDuration->text().toInt());

    SwSettings settings;
    settings.addCommunicationSettings(comConfig);
    settings.addTsSettings(tsConfig);
    settings.addTcSettings(tcConfig);

    QFileDialog getPathDialog;
    getPathDialog.setModal(true);
    QDate date = QDate::currentDate();
    QTime time = QTime::currentTime();
    QString dateTimeName = QString::number(date.month()) + "_"
                           + QString::number(date.day()) + "_"
                           + QString::number(date.year()) + "_"
                           + QString::number(time.hour()) + "_"
                           + QString::number(time.minute()) + "_"
                           + QString::number(time.second());
    QString fileName = getPathDialog.getSaveFileName(this,
                                                     tr("Save  as"),
                                                     SETTINGS_FILE_NAME + tr("_") + dateTimeName + SETTINGS_FILE_EXTANTION,
                                                     tr("Settings(*.json)"));

    if (fileName.isEmpty()) {
        return;
    }

    QByteArray settingsBuff{settings.getJsonFile()};
    QFile fw{fileName};

    fw.open(QIODevice::WriteOnly);
    fw.write(settingsBuff);
    fw.close();
}

void Mdio::on_pbOpenSettingsFile_clicked()
{
    QFileDialog getPathDialog;
    getPathDialog.setModal(true);
    QString fileName = getPathDialog.getOpenFileName(this,
                                                     tr("Open settings"),
                                                     NULL,
                                                     tr("Settings(*.json)"));
    if (fileName.isEmpty()) {
        return;
    }

    QFile settings(fileName);

    settings.open(QIODeviceBase::ReadOnly);
    QByteArray settingJson = settings.readAll();
    settings.close();
    QString errorStr;
    SwSettings newSettings(settingJson);

    if (!newSettings.getState(errorStr)) {
        errorMessage("Помилка файлу конфігурації", errorStr);
        return;
    };

    /*
     * Move new settings to UI
     */
    ui->cbBaudRate->setCurrentText(QString::number(newSettings.getBr()));
    ui->cbParity->setCurrentText(newSettings.getParity());
    ui->cbStopBits->setCurrentText(QString::number(newSettings.getStopBits()));
    ui->leBinaryTsSwitchTime->setText(QString::number(newSettings.getSwitchTime()));
    ui->leDebounceInterval->setText(QString::number(newSettings.getDebounceTime()));
    ui->leSilentInterval->setText(QString::number(newSettings.getSilentInterval()));
    ui->lePulsDuration->setText(QString::number(newSettings.getPulsDuration()));
    ui->leReplyDelay->setText(QString::number(newSettings.getReplyDelay()));

    for (qsizetype i = 0; i < tsTypeConfigList.size(); i++) {
        tsTypeConfigList.at(i)->setCurrentIndex(newSettings.getDoubleSign().at(i) ? 1 : 0);
    }
    for (qsizetype i = 0; i < tsSetingsList.size(); i++) {
        tsSetingsList.at(i)->setInvert(newSettings.getInversionSign().at(i));
    }
}

void Mdio::on_leSilentInterval_editingFinished()
{
    int brIndex = SwDefSettings::getBrList().lastIndexOf(ui->cbBaudRate->currentText().toInt());

    verifyAndModifyNumber((QLineEdit *)this->sender(),
                          SwDefSettings::getSilentIntervalMinList().at(brIndex),
                          SwDefSettings::getSilentIntervalMaxList().at(brIndex),
                          SwDefSettings::getSilentIntervalDefaultList().at(brIndex));
}

void Mdio::on_leSilentInterval_textEdited(const QString &arg1)
{
    (void)arg1;
    int brIndex = SwDefSettings::getBrList().lastIndexOf(ui->cbBaudRate->currentText().toInt());

    verifyNumber((QLineEdit *)this->sender(),
                 SwDefSettings::getSilentIntervalMinList().at(brIndex),
                 SwDefSettings::getSilentIntervalMaxList().at(brIndex));
}

void Mdio::on_leReplyDelay_editingFinished()
{
    verifyAndModifyNumber((QLineEdit *)this->sender(),
                          SwDefSettings::getTimeoutReplyMin(),
                          SwDefSettings::getTimeoutReplyMax(),
                          SwDefSettings::getTimeoutReplyDefault());
}

void Mdio::on_leReplyDelay_textEdited(const QString &arg1)
{
    (void)arg1;
    verifyNumber((QLineEdit *)this->sender(),
                 SwDefSettings::getTimeoutReplyMin(),
                 SwDefSettings::getTimeoutReplyMax());
}

void Mdio::on_leDebounceInterval_editingFinished()
{
    verifyAndModifyNumber((QLineEdit *)this->sender(),
                          SwDefSettings::getDebounceMin(),
                          SwDefSettings::getDebounceMax(),
                          SwDefSettings::getDebounceDefault());
}

void Mdio::on_leDebounceInterval_textEdited(const QString &arg1)
{
    (void)arg1;

    verifyNumber((QLineEdit *)this->sender(),
                 SwDefSettings::getDebounceMin(),
                 SwDefSettings::getDebounceMax());
}

void Mdio::on_leBinaryTsSwitchTime_editingFinished()
{
    verifyAndModifyNumber((QLineEdit *)this->sender(),
                          SwDefSettings::getSwitchTimeMin(),
                          SwDefSettings::getSwitchTimeMax(),
                          SwDefSettings::getBinaryTsSwitchTimeDefault());
}


void Mdio::on_leBinaryTsSwitchTime_textEdited(const QString &arg1)
{
    (void)arg1;
    verifyNumber((QLineEdit *)this->sender(),
                 SwDefSettings::getSwitchTimeMin(),
                 SwDefSettings::getSwitchTimeMax());
}

void Mdio::on_lePulsDuration_editingFinished()
{
    verifyAndModifyNumber((QLineEdit *)this->sender(),
                          SwDefSettings::getPulsDurationMin(),
                          SwDefSettings::getPulsDurationMax(),
                          SwDefSettings::getPulsDurationDefault());
}

void Mdio::on_lePulsDuration_textEdited(const QString &arg1)
{
    (void)arg1;
    verifyNumber((QLineEdit *)this->sender(),
                 SwDefSettings::getPulsDurationMin(),
                 SwDefSettings::getPulsDurationMax());
}

