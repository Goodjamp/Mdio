#include "mdio.h"
#include "ui_mdio.h"

#include <QFile>
#include <QDebug>
#include <QMap>
#include <QDate>
#include <QThread>
#include <QMessageBox>
#include <QValidator>
#include <QByteArray>
#include <QRegExp>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonArray>
#include <dialogconnectionsettings.h>
#include <modbusrtumaster.h>


#define STR_CAST(str)    static_cast<QString>(str)
#define SW_NAME          STR_CAST("МВВ-4-2 конфігуратор")

void Mdio::updateLanguage(QString language)
{
    QFile uiSettingsJson(":/UiSettings.json");
    QByteArray uiSettingsRaw;
    QJsonDocument uiDescrJson;
    QJsonObject rootObj;
    QJsonArray temJsonArray;
    QJsonArray relayStateString;
    QJsonObject temObj;

    uiSettingsJson.open(QFile::ReadOnly);

    uiSettingsRaw = uiSettingsJson.readAll();
    uiDescrJson = QJsonDocument::fromJson(uiSettingsRaw);
    rootObj = uiDescrJson.object();
    temJsonArray = rootObj.take("Language").toObject().take(language).toObject().take("TC").toArray();
    for (int k = 0; k < temJsonArray.size(); k++) {
        tcMonitorList[k]->setName(temJsonArray[k].toObject().take("Name").toString());
        tcMonitorList[k]->setTextStateList(temJsonArray[k].toObject().take("Relay").toArray().toVariantList()[0].toStringList());
    }
}

void Mdio::enableSettingsControl()
{
    foreach(auto item,  settingsItemsList) {
        item->setEnabled(true);
    }
    ui->pbConnect->setEnabled(false);
    ui->pbConnectionSettings->setEnabled(false);
}

void Mdio::disableSettingsControl()
{
    foreach(auto item,  settingsItemsList) {
        item->setEnabled(false);
    }
    ui->cbBaudRate->setCurrentIndex(-1);
    ui->cbParity->setCurrentIndex(-1);
    ui->cbStopBits->setCurrentIndex(-1);
    ui->pbConnect->setEnabled(true);
    ui->pbConnectionSettings->setEnabled(false);
}

void Mdio::initCustomUi()
{
    relayCOntrolButtonsList = new QButtonGroup();
    QRegExpValidator *numericValidator3D = new QRegExpValidator((QRegExp)"\\d{1,3}", this);
    QRegExpValidator *numericValidator4D = new QRegExpValidator((QRegExp)"\\d{1,4}", this);

    /*
     * Add validation to the numeric fields
     */
    ui->leSilentInterval->setValidator(numericValidator3D);
    ui->leReplyDelay->setValidator(numericValidator3D);
    ui->leDebounceInterval->setValidator(numericValidator3D);
    ui->lePulsDuration->setValidator(numericValidator4D);

    /*
     * Add TeleControl status/control items
     */
    for (uint32_t k = 0; k < TELECONTROL_TOTAL_NUMBERS; k++) {
        tcMonitorList.append(new TcControl("",
                                    k,
                                    this));
        ui->vlTcControlMonitorInternal->addWidget(tcMonitorList[tcMonitorList.size() - 1]);
        relayCOntrolButtonsList->addButton(tcMonitorList[tcMonitorList.size() - 1]->getOffButtonPointer());
        relayCOntrolButtonsList->addButton(tcMonitorList[tcMonitorList.size() - 1]->getOnButtonPointer());
    }
    tcLayoutSpacer = new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Expanding);
    ui->vlTcControlMonitorInternal->addItem(tcLayoutSpacer);

    /*
     * Add TeleSignalisation configuration items
     */
    for (uint32_t k = 0; k < TELESIGNAL_NUMBERS; k++) {
        tsSetingsList.append(new TsSettings(k + 1));
        ui->vlTeleSignalSettings->addWidget(tsSetingsList[tsSetingsList.size() - 1]);
    }
    tsLayoutSpacer = new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Expanding);
    ui->vlTeleSignalSettings->addItem(tsLayoutSpacer);

    /*
     * Add TeleSignalisation status items
     */

    for (uint32_t k = 0; k < TELESIGNAL_NUMBERS; k++) {
        tsStatus.append(new TsStatus(k + 1));
        ui->vlTeleSignalStatus->addWidget(tsStatus[tsStatus.size() - 1]);
    }
    tsStatusLayoutSpacer = new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Expanding);
    ui->vlTeleSignalStatus->addItem(tsStatusLayoutSpacer);

    /*
     *  Title bar: icon name
     */
    setWindowTitle(SW_NAME);
    setWindowIcon((QIcon)":/Resources/CompanyIcon.png");

    ui->cbBaudRate->addItems(brValueToStrLUT.values());
    ui->cbStopBits->addItems(stopBitsSerialToStrLUT.values());
    ui->cbParity->addItems(paritySerialToStrLUT.values());

    ui->pbVoltageOnTcStatus->setEnabled(false);
    ui->pbEepromStatus->setEnabled(false);
    ui->pbTransactionStatus->setEnabled(false);
    ui->pbEepromClearStatus->setEnabled(false);

    /*
     * Set UI text settings according to the target language
     */
    updateLanguage("UA");

    /*
     * Add all UI element to control enbling
     */
    settingsItemsList.push_back(static_cast<QWidget *>(ui->cbBaudRate));
    settingsItemsList.push_back(static_cast<QWidget *>(ui->cbParity));
    settingsItemsList.push_back(static_cast<QWidget *>(ui->cbStopBits));
    settingsItemsList.push_back(static_cast<QWidget *>(ui->leDebounceInterval));
    settingsItemsList.push_back(static_cast<QWidget *>(ui->lePulsDuration));
    settingsItemsList.push_back(static_cast<QWidget *>(ui->leReplyDelay));
    settingsItemsList.push_back(static_cast<QWidget *>(ui->leSilentInterval));
    settingsItemsList.push_back(static_cast<QWidget *>(ui->pbApplySettings));
    settingsItemsList.push_back(static_cast<QWidget *>(ui->pbReadSettings));
    settingsItemsList.push_back(static_cast<QWidget *>(ui->pbReload));
    settingsItemsList.push_back(static_cast<QWidget *>(ui->pbDisconnect));
    foreach(auto item, tcMonitorList) {
        settingsItemsList.push_back(static_cast<QWidget *>(item->getOnButtonPointer()));
        settingsItemsList.push_back(static_cast<QWidget *>(item->getOffButtonPointer()));
    }
    foreach(auto item, tsSetingsList) {
         settingsItemsList.push_back(static_cast<QWidget *>(item->getComboBoxPointer()));
    }
    disableSettingsControl();
    ui->pbConnect->setEnabled(false);
}


Mdio::Mdio(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::Mdio)
{
    ui->setupUi(this);


    commmunicationThread = new QThread();
    communicaiton = new Communication();
    readStateTimer = new QTimer();
    readStateTimer->setInterval(READ_STATE_PERIOD_MS);

    connect(readStateTimer, &QTimer::timeout, this, &Mdio::readSlaveState);
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

    communicationSyncSem.acquire();

    /*
     * The pair of the signal/slot updateUiStateSignal/updateUiStateSlot is used to
     * pass the results of reading the state of slave from the CB
     * function, called from the external thread to the Mdio thread.
     */
    connect(this, &Mdio::updateUiStateSignal, this, &Mdio::updateUiStateSlot);

    /*
     * Default (initial) connection settings
     */
    lastConnectionUserSettings.address = 1;
    lastConnectionUserSettings.brIndex = 3;
    lastConnectionUserSettings.parityIndex = 0;
    lastConnectionUserSettings.stopBitsIndex = 0;
    lastConnectionUserSettings.portIndex = 0;
}

Mdio::~Mdio()
{
    delete ui;
}

void Mdio::updateUiConnectionStatusStr(void)
{
    QString connectionSettingsStr;
    QStringList comList = Communication::getPortsList();

    if (isSlaveConnect == true) {
        connectionSettingsStr = comList[connectPortIndex]
                                + " "
                                + brValueToStrLUT.values()[connectBrIndex]
                                + " "
                                + "8"
                                + paritySerialToStrLUT.values()[connectParityIndex][0]
                                + stopBitsSerialToStrLUT.values()[connectStopBitsIndex]
                                + " "
                                + "Адр."
                                + QString::number(connectSlaveAddress);
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
                 + QString::number(connectDeviceConfDay) + "."
                 + QString::number(connectDeviceConfMonth) + "."
                 + QString::number(connectDeviceConfYear);


    ui->lDeviceMetaInfo->setText(metaInfStr);
}

bool Mdio::updateUiConfiguration(void)
{
    if (brValueToStrLUT.contains(connectDeviceConf.communication.baudRate) == false) {
        errorMessage("Помилка конфігурації", "Помилка швидкості");
        return false;
    }
    if (paritySerialToStrLUT.contains(connectDeviceConf.communication.parity) == false) {
        errorMessage("Помилка конфігурації", "Помилка паритету");
        return false;
    }
    if (paritySerialToStrLUT.contains(connectDeviceConf.communication.parity) == false) {
        errorMessage("Помилка конфігурації", "Помилка паритету");
        return false;
    }

    /*
     * Show configuration on the UI
     */
    ui->cbBaudRate->setCurrentText(brValueToStrLUT.value(connectDeviceConf.communication.baudRate));
    ui->cbParity->setCurrentText(paritySerialToStrLUT.value(connectDeviceConf.communication.parity));
    ui->cbStopBits->setCurrentText(stopBitsSerialToStrLUT.value(connectDeviceConf.communication.stopBits));
    ui->leReplyDelay->setText(QString::number(connectDeviceConf.communication.replyDelay));
    ui->leSilentInterval->setText(QString::number(connectDeviceConf.communication.silentInterval));
    ui->leDebounceInterval->setText(QString::number(connectDeviceConf.signalisation.debounsInterval));
    for (uint32_t k = 0; k < TELESIGNAL_NUMBERS; k++) {
        tsSetingsList[k]->setInver(connectDeviceConf.signalisation.isInvers[k]);
    }
    ui->lePulsDuration->setText(QString::number(connectDeviceConf.control.pulsDuration));

    connectDeviceConfYear = connectDeviceConf.year;
    connectDeviceConfMonth = connectDeviceConf.month;
    connectDeviceConfDay = connectDeviceConf.day;

    /*
     * Update last configuration date information
     */
    updateUiDeviceMetaInfStr();
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
    connectPortIndex = 0;
    connectBrIndex = DEFAULT_CONNECT_BR_INDEX;
    connectParityIndex = DEFAULT_CONNECT_PARITY_INDEX;
    connectStopBitsIndex = DEFAULT_CONNECT_STOP_BITS_INDEX;
    connectSlaveAddress = DEFAULT_CONNECT_SLAVE_ADDRESS;
    connectDeviceVersion = DEFAULT_CONNECT_VERSION;
    connectDeviceConfYear = DEFAULT_CONNECT_YEAR;
    connectDeviceConfMonth = DEFAULT_CONNECT_MONTH;
    connectDeviceConfDay = DEFAULT_CONNECT_DATE;
    isSlaveConnect = false;
}

void Mdio::applyConnectionSettings(DialogConnectionSettings::UserSettingsList connectionSettings)
{
    connectPortIndex = connectionSettings.portIndex;
    connectBrIndex =connectionSettings.brIndex;
    connectParityIndex =connectionSettings.parityIndex;
    connectStopBitsIndex =connectionSettings.stopBitsIndex;
    connectSlaveAddress =connectionSettings.address;
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
    qDebug()<<"Connect rez:"<<result;
    communicationSyncSem.release(1);

}

void  Mdio::writeConfigurationResult(bool result)
{
    /*
     * Release (give) semaphore to unblok code that waite to complete
     */
    communicationResult = result;
    qDebug()<<"writeConfigurationResult result: "<<result;
    communicationSyncSem.release(1);
}

void Mdio::setTeleControlResult(bool result)
{
    /*
     * Release (give) semaphore to unblok code that waite to complete
     */
    communicationResult = result;
    qDebug()<<"setTeleControlResult result: "<<result;
    communicationSyncSem.release(1);
}


void Mdio::readMetaInformationResult(bool result, int fwVersion)
{
    /*
     * Release (give) semaphore to unblok code that waite to complete
     */
    communicationResult = result;
    qDebug()<<"Read meta information result: "<<result;
    if (result == true) {
        connectDeviceVersion = fwVersion;
    }
    communicationSyncSem.release(1);
}

void  Mdio::readConfigurationResult(bool result, Communication::SlaveConfiguration configuration)
{
    /*
     * Release (give) semaphore to unblok code that waite to complete
     */
    communicationResult = result;
    qDebug()<<"readConfigurationResult result: "<<result;
    if (result == true) {
        connectDeviceConf = configuration;
    }
    communicationSyncSem.release(1);
}


void Mdio::reloadResult(bool result)
{
    /*
     * Release (give) semaphore to unblok code that waite to complete
     */
    communicationResult = result;
    qDebug()<<"reloadResult result: "<<result;
    if (result == true) {
        /*
         * Do we need apply new communicaiotn settings ?
         */
    }
    communicationSyncSem.release(1);
}

void Mdio::readStateResult(bool result, Communication::SlaveState state)
{
    qDebug()<<"readStateResult result: "<<result;
    emit this->updateUiStateSignal(result, state);
}

void Mdio::on_pbConnectionSettings_clicked()
{
    DialogConnectionSettings::UiFilingList dialoConnectUiFillList;
    QStringList comList = Communication::getPortsList();
    SerialCommunication::SerialPortParity parity;
    SerialCommunication::SerialPortStopBits stopBits;

    dialoConnectUiFillList.comList = comList;
    dialoConnectUiFillList.brList = brValueToStrLUT.values();
    dialoConnectUiFillList.defBr = 3;
    dialoConnectUiFillList.parityList = paritySerialToStrLUT.values();
    dialoConnectUiFillList.defParity = 0;
    dialoConnectUiFillList.stopBitsList = stopBitsSerialToStrLUT.values();
    dialoConnectUiFillList.defStopBits = 0;
    if (lastConnectionUserSettings.portIndex >= comList.size()) {
        lastConnectionUserSettings.portIndex = 0;
    }

    DialogConnectionSettings dialogConnectionSettings(dialoConnectUiFillList,
                                                      lastConnectionUserSettings);

    dialogConnectionSettings.setModal(true);
    connect(&dialogConnectionSettings, &DialogConnectionSettings::applySettings, this, &Mdio::applyConnectionSettings);
    needConnectSlave = false;

    dialogConnectionSettings.exec();

    /*
     * If user push botton *Close*, we don't need continue connection to slave
     */
    if (needConnectSlave ==false) {
        return;
    }

    /*
     * Open connection
     */
    if(parityUiToSerilaLUT.contains(connectParityIndex)) {
        parity = parityUiToSerilaLUT.value(connectParityIndex);
    } else {
        return;
    }

    if(stopUiToSerilaLUT.contains(connectStopBitsIndex)) {
        stopBits = stopUiToSerilaLUT.value(connectStopBitsIndex);
    } else {
        return;
    }
    emit connectSlave(CB_WRAP_1(Mdio, connectSlaveResult),
                      comList[connectPortIndex],
                      brValueToStrLUT.keys()[connectBrIndex],
                      parity,
                      stopBits);
    /*
     * Waite to complete connection
     */
    if (processingCommunicatitonResult("Неможливо приєднатися",
                                        "Порт недоступний", true) == false) {
        return;
    }

    /*
     * Read meta information
     */
    emit readMetaInformation(CB_WRAP_2(Mdio, readMetaInformationResult), connectSlaveAddress);
    if (processingCommunicatitonResult("Неможливо приєднатися",
                                       "Помилка зчитування метаінформації", false) == false) {
        return;
    }

    /*
     * Read configuration
     */
    emit readConfiguration(CB_WRAP_2(Mdio, readConfigurationResult), connectSlaveAddress);
    if (processingCommunicatitonResult("Неможливо приєднатися",
                                       "Помилка считування конфігурації", false) == false) {
        return;
    }

    if (updateUiConfiguration() == false) {
        return;
    }

    /*
     * Start timer to read the slave state
     */
    stateRequestCnt = 0;
    stateReplyCnt = 0;
    readStateTimer->start();

    isSlaveConnect = true;
    updateUiConnectionStatusStr();
    enableSettingsControl();
}

void Mdio::on_pbApplySettings_clicked()
{
    Communication::SlaveConfiguration configuration;

    /*
     * Read user configuration and serialiase it to the SlaveConfiguration
     * structure
     */

    if (VALUE_IN_RANGE(ui->leReplyDelay->text().toUInt(),
                       SILENT_INTERVAL_MIN_MS, SILENT_INTERVAL_MAX_MS) == false ) {
        errorMessage("Помилка конфігурації",
                     "Час затримки відповіді повинено бути в діапазоні [" + QString::number(REPLAY_DELAY_MIN_MS)
                     + "-" + QString::number(REPLAY_DELAY_MAX_MS) + "] мс");
        return;
    }

    if (VALUE_IN_RANGE(ui->leSilentInterval->text().toUInt(),
                       SILENT_INTERVAL_MIN_MS, SILENT_INTERVAL_MAX_MS) == false ) {
        errorMessage("Помилка конфігурації",
                     "Інтервал тиші повинено бути в діапазоні [" + QString::number(SILENT_INTERVAL_MIN_MS)
                     + "-" + QString::number(SILENT_INTERVAL_MAX_MS) + "] мс");
        return;
    }

    if (VALUE_IN_RANGE(ui->leDebounceInterval->text().toUInt(),
                       DEBOUNCE_INTARVAL_MIN_MS, DEBOUNCE_INTARVAL_MAX_MS) == false ) {
        errorMessage("Помилка конфігурації",
                     "Тривалість брязкіту повинно бути в діапазоні [" + QString::number(DEBOUNCE_INTARVAL_MIN_MS)
                     + "-" + QString::number(DEBOUNCE_INTARVAL_MAX_MS) + "] мс");
        return;
    }

    if (VALUE_IN_RANGE(ui->lePulsDuration->text().toUInt(),
                       PULS_DURATION_MIN_MS, PULS_DURATION_MMAX_MS) == false ) {
        errorMessage("Помилка конфігурації",
                     "Тривалість імпульсу ТК повинно бути в діапазоні [" + QString::number(PULS_DURATION_MIN_MS)
                     + "-" + QString::number(PULS_DURATION_MMAX_MS) + "] мс");
        return;
    }

    configuration.communication.baudRate = brValueToStrLUT.key(ui->cbBaudRate->currentText());
    configuration.communication.parity = paritySerialToStrLUT.key(ui->cbParity->currentText());
    configuration.communication.stopBits = stopBitsSerialToStrLUT.key(ui->cbStopBits->currentText());
    configuration.communication.replyDelay = ui->leReplyDelay->text().toInt();
    configuration.communication.silentInterval = ui->leSilentInterval->text().toInt();
    configuration.signalisation.debounsInterval = ui->leDebounceInterval->text().toInt();
    for (uint32_t k = 0; k < TELESIGNAL_NUMBERS; k++) {
        configuration.signalisation.isInvers[k] = tsSetingsList[k]->isInvert();
    }
    configuration.control.pulsDuration = ui->lePulsDuration->text().toInt();
    configuration.year = QDate::currentDate().year() - 2000;
    configuration.month = QDate::currentDate().month();
    configuration.day = QDate::currentDate().day();

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

void Mdio::on_pbReadSettings_clicked()
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
         * Update Tele signalisarion and status indication
         */
        for (uint32_t k = 0; k < TELESIGNAL_NUMBERS; k++) {
            tsStatus[k]->setStatus(state.signalisation[k]);
        }

        ui->pbVoltageOnTcStatus->setChecked(state.error220);
        ui->pbEepromStatus->setChecked(state.errorEeprom);
        ui->pbTransactionStatus->setChecked(state.errorTransaction);
        ui->pbEepromClearStatus->setChecked(state.errorEepromClear);

        /*
         * Update Tele control indication
         *
         * The communicaiotn return 3 registers (puls and 2 static)
         * but we need indicaiotn only 2 (static)
         */
        for (int k = 0; k < TELECONTRO_STATIC_NUMBERS; k++) {
            //tcStatic[k]->setStaticState(state.control[k + 1]);
        }
    }
}

void Mdio::readSlaveState(void)
{
    emit readState(CB_WRAP_2(Mdio, readStateResult), connectSlaveAddress);
    stateRequestCnt++;
    updateUiCommunicationStatisticStr();
}

void Mdio::tcSetTcSlot(int index, bool enable)
{
    emit this->setTeleControl(CB_WRAP_1(Mdio, setTeleControlResult), connectSlaveAddress, index, enable);

    if (processingCommunicatitonResult("Телекерування",
                                       "Помилка передачі команди\nдля статичного телекерування", false) == false) {
        return;
    }
}
