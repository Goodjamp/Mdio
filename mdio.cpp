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
#include <QRegExp>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonArray>
#include <QFrame>
#include <dialogconnectionsettings.h>
#include <modbusrtumaster.h>
#include "Version.h"


#define STR_CAST(str)    static_cast<QString>(str)
#define SW_NAME          STR_CAST("МВВ-4-2 конфігуратор")

void Mdio::getSettingsFromJson()
{
    QFile jsonFile(":/UiSettings.json");
    QByteArray jsonContent;
    QJsonDocument uiDescrJson;

    /*
     * Rread JSON file with UI text settings
     */
    jsonFile.open(QFile::ReadOnly);
    jsonContent = jsonFile.readAll();
    jsonFile.close();

    uiDescrJson = QJsonDocument::fromJson(jsonContent);
    rootJsonObj = uiDescrJson.object();

    /*
     * Default (initial) connection settings
     */
    lastConnectionUserSettings.br = rootJsonObj.value("Port").toObject().value("BrDefault").toString();
    lastConnectionUserSettings.parity = rootJsonObj.value("Port").toObject().value("ParityDefault").toString();
    lastConnectionUserSettings.stopBits = rootJsonObj.value("Port").toObject().value("StopBitsDefault").toString();
    lastConnectionUserSettings.address = rootJsonObj.value("Modbus").toObject().value("AddressDefault").toString();
    lastConnectionUserSettings.replyTimeout = rootJsonObj.value("Modbus").toObject().value("TimeoutReplyDefaultPc").toString();
    lastConnectionUserSettings.silentInterval = rootJsonObj.value("Modbus").toObject().value("SilentIntervalDefaultPc").toString();
    lastConnectionUserSettings.port = "";
    foreach(auto item, rootJsonObj.value("Modbus").toObject().value("SilentIntervalList").toArray().toVariantList()){
        silentIntervalLIst.push_back(item.toString());
    }
}

void Mdio::enableSettingsControl()
{
    foreach(auto item,  settingsItemsList) {
        item->setEnabled(true);
    }
    foreach(auto item, tsSetingsList) {
         item->setEnableCb(true);
    }
    foreach(auto item, tsTypeControlList) {
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
    foreach(auto item, tsSetingsList) {
         item->setEnableCb(false);
    }
    foreach(auto item, tsTypeControlList) {
        item->setEnabled(false);
    }

    ui->pbConnect->setEnabled(true);
    ui->pbConnectionSettings->setEnabled(true);
}

void Mdio::skipAllSettings()
{
    ui->cbBaudRate->setCurrentIndex(-1);
    ui->cbParity->setCurrentIndex(-1);
    ui->cbStopBits->setCurrentIndex(-1);
    ui->leDebounceInterval->setText("");
    ui->lePulsDuration->setText("");
    ui->leReplyDelay->setText("");
    ui->leSilentInterval->setText("");
    ui->leDoubleTsSwitchTime->setText("");
}

void Mdio::addTsBinaryGroupUi(void)
{
    QHBoxLayout *serviceLayoute;
    QSize size(120, 25);

    for (uint32_t k = 0; k < TELESIGNAL_NUMBERS; k++) {
        teleSignalBinaryFrameList.push_back(new QFrame());

        /*
         * Apply style to the TS frame as Simple TS
         */

        teleSignalBinaryFrameList.last()->setProperty("tsBinary", false);
        teleSignalBinaryFrameList.last()->style()->unpolish(teleSignalBinaryFrameList.last());
        teleSignalBinaryFrameList.last()->style()->polish(teleSignalBinaryFrameList.last());


        teleSignalBinaryLayoutList.push_back(new QVBoxLayout);
        teleSignalBinaryFrameList.last()->setLayout(teleSignalBinaryLayoutList.last());


        ui->vlTeleSignalSettings->addWidget(teleSignalBinaryFrameList.last());


        /*
         * Add TS settings number K
         */
        tsSetingsList.append(new TsSettings(k + 1));
        teleSignalBinaryLayoutList.last()->addWidget(tsSetingsList.last());
        k++;

        /*
         * Add simple/binary TS settings type
         */
        tsTypeControlList.append(new QComboBox());
        tsTypeControlList.last()->addItems({"Одинарні", "Подвійні"});
        tsTypeControlList.last()->setFixedSize(size);
        tsTypeControlList.last()->setCurrentIndex(-1);
        serviceLayoute = new QHBoxLayout();
        serviceLayoute->addItem(new QSpacerItem(20, 0, QSizePolicy::Expanding, QSizePolicy::Expanding));
        serviceLayoute->addWidget(tsTypeControlList.last());
        teleSignalBinaryLayoutList.last()->addLayout(serviceLayoute);

        /*
         * Add TS settings number K + 1
         */
        tsSetingsList.append(new TsSettings(k + 1));
        teleSignalBinaryLayoutList.last()->addWidget(tsSetingsList.last());
    }

    foreach(auto item, tsTypeControlList) {
          connect(item, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &Mdio::on_cbTsType_currentIndexChanged);
    }

    tsLayoutSpacer = new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Expanding);
    ui->vlTeleSignalSettings->addItem(tsLayoutSpacer);
}

void Mdio::on_cbTsType_currentIndexChanged(int index)
{
    QObject *senderObj = sender();
    static bool state = true;

    for (uint32_t k = 0; k < tsTypeControlList.size(); k++) {
        if (senderObj == tsTypeControlList[k]) {
            /*
             * Apply style according to the TS state
             */
            teleSignalBinaryFrameList[k]->setProperty("tsBinary", index == 1);
            teleSignalBinaryFrameList[k]->style()->unpolish(teleSignalBinaryFrameList[k]);
            teleSignalBinaryFrameList[k]->style()->polish(teleSignalBinaryFrameList[k]);
            qDebug()<<"State = "<<state;
            qDebug()<<"Sender search Ok";
            break;
        }
    }
}

void Mdio::initCustomUi(QString language)
{
    QJsonArray temJsonArray;
    QStringList strListRelayStr1;
    QStringList strListRelayStr2;
    relayControlListTc1 = new QButtonGroup();
    relayControlListTc2 = new QButtonGroup();
    QRegExpValidator *numericValidator3D = new QRegExpValidator((QRegExp)"\\d{1,3}", this);
    QRegExpValidator *numericValidator4D = new QRegExpValidator((QRegExp)"\\d{1,4}", this);

    /*
     * Add validation to the numeric UI items
     */
    ui->leSilentInterval->setValidator(numericValidator3D);
    ui->leReplyDelay->setValidator(numericValidator3D);
    ui->leDebounceInterval->setValidator(numericValidator3D);
    ui->lePulsDuration->setValidator(numericValidator4D);
    ui->leDoubleTsSwitchTime->setValidator(numericValidator4D);

    /*
     * Add TeleControl status/control items
     */
    for (uint32_t k = 0; k < TELECONTROL_TOTAL_NUMBERS; k++) {
        tcMonitorList.append(new TcControl("", k, this));
        ui->vlTcControlMonitorInternal->addWidget(tcMonitorList.last());
    }

    tcLayoutSpacer = new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Expanding);
    ui->vlTcControlMonitorInternal->addItem(tcLayoutSpacer);

    temJsonArray = rootJsonObj.value("Language").toObject().value(language).toObject().value("TC").toArray();
    for (int k = 0; k < temJsonArray.size(); k++) {
        strListRelayStr1.clear();
        strListRelayStr2.clear();
        tcMonitorList[k]->setName(temJsonArray[k].toObject().value("Name").toString());
        foreach(auto item, temJsonArray[k].toObject().value("RelayStr1").toArray().toVariantList()) {
           strListRelayStr1.push_back(item.toString());
        }
        foreach(auto item, temJsonArray[k].toObject().value("RelayStr2").toArray().toVariantList()) {
           strListRelayStr2.push_back(item.toString());
        }
        tcMonitorList[k]->setTextStateList(strListRelayStr1, strListRelayStr2);
    }

    /*
     * Add TeleSignalisation configuration items
     */
    /*
    for (uint32_t k = 0; k < TELESIGNAL_NUMBERS; k++) {
        teleSignalBinaryFrameList.push_back(new QFrame);
        teleSignalBinaryFrameList.last()->setProperty("tsBinary", true);
        teleSignalBinaryFrameList.last()->style()->unpolish(teleSignalBinaryFrameList.last());
        teleSignalBinaryFrameList.last()->style()->polish(teleSignalBinaryFrameList.last());

        teleSignalBinaryLayoutList.push_back(new QVBoxLayout);
        teleSignalBinaryFrameList.last()->setLayout(teleSignalBinaryLayoutList.last());

        ui->vlTeleSignalSettings->addWidget(teleSignalBinaryFrameList.last());


        tsSetingsList.append(new TsSettings(k + 1));
        teleSignalBinaryLayoutList.last()->addWidget(tsSetingsList.last());
        tsTypeControlList.append(new QComboBox());
        teleSignalBinaryLayoutList.last()->addWidget(tsTypeControlList.last());
        k++;
        tsSetingsList.append(new TsSettings(k + 1));
        teleSignalBinaryLayoutList.last()->addWidget(tsSetingsList.last());
    }
    tsLayoutSpacer = new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Expanding);
    ui->vlTeleSignalSettings->addItem(tsLayoutSpacer);
    */
    addTsBinaryGroupUi();

    /*
     * Add TeleSignalisation status items
     */

    for (uint32_t k = 0; k < TELESIGNAL_NUMBERS; k++) {
        tsStatus.append(new TsStatus(k + 1));
        ui->vlTeleSignalStatus->addWidget(tsStatus.last());
    }
    tsStatusLayoutSpacer = new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Expanding);
    ui->vlTeleSignalStatus->addItem(tsStatusLayoutSpacer);

    /*
     *  Title bar: icon name
     */
    setWindowTitle(SW_NAME + " V"
                   + QString::number(VERSION_MAJOR) + "."
                   + QString::number(VERSION_MINOR) + "."
                   + QString::number(VERSION_BUILD));
    setWindowIcon((QIcon)":/Resources/CompanyIcon.png");

    foreach(auto item, rootJsonObj.value("Port").toObject().value("BrList").toArray().toVariantList()) {
        brStrList.push_back(item.toString());
    }
    foreach(auto item, rootJsonObj.value("Port").toObject().value("ParityList").toArray().toVariantList()) {
        parityStr.push_back(item.toString());
    }
    foreach(auto item, rootJsonObj.value("Port").toObject().value("StopBitsList").toArray().toVariantList()) {
        stopBitsStrList.push_back(item.toString());
    }

    ui->cbBaudRate->addItems(brStrList);
    ui->cbStopBits->addItems(stopBitsStrList);
    ui->cbParity->addItems(parityStr);

    ui->pbVoltageOnTcStatus->setEnabled(false);
    ui->pbEepromStatus->setEnabled(false);
    ui->pbEepromClearStatus->setEnabled(false);

    /*
     * Add all UI element to control enabling
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
    settingsItemsList.push_back(static_cast<QWidget *>(ui->pbSetDefaultSettings));
    settingsItemsList.push_back(static_cast<QWidget *>(ui->leDoubleTsSwitchTime));
    foreach(auto item, tcMonitorList) {
        settingsItemsList.push_back(static_cast<QWidget *>(item->getOnButtonPointer()));
        settingsItemsList.push_back(static_cast<QWidget *>(item->getOffButtonPointer()));
    }

    disableSettingsControl();
    skipAllSettings();
    ui->pbConnect->setEnabled(false);

    ui->tabWidget->setCurrentIndex(0);
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
    getSettingsFromJson();
    resetSlaveInformation();
    initCustomUi("UA");
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
    QStringList comList = Communication::getPortsList();

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
    QStringList comList = Communication::getPortsList();

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


void Mdio::readMetaInformationResult(bool result, Communication::MetaInformation metaInformation)
{
    /*
     * Release (give) semaphore to unblok code that waite to complete
     */
    communicationResult = result;
    qDebug()<<"Read meta information result: "<<result;
    if (result == true) {
        connectDeviceVersion = metaInformation.fwVersion;
        lastConfigurationDay = metaInformation.configurationDay;
        lastConfigurationMonth = metaInformation.configurationMonth;
        lastConfigurationYear = metaInformation.configurationYear;
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

void Mdio::connectWithSettings()
{
    QStringList comList = Communication::getPortsList();

    if (comList.contains(connectPort) == false) {
        errorMessage("Помилка конфігурації",
                     "СOM порт не доступний");
        return;
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

    connectWithSettings();
}

void Mdio::on_pbApplySettings_clicked()
{
    Communication::SlaveConfiguration configuration;


    /*
     * Make a test of the user configuration
     */

    if (ui->leReplyDelay->text().isDetached()) {
        errorMessage("Помилка конфігурації",
                     "Час затримки відповіді не заданий");
        return;
    } else if (VALUE_IN_RANGE(ui->leReplyDelay->text().toInt(),
                              rootJsonObj.value("Modbus").toObject().value("TimeoutReplyMin").toString().toInt(),
                              rootJsonObj.value("Modbus").toObject().value("TimeoutReplyMax").toString().toInt())
               == false ) {
        errorMessage("Помилка конфігурації",
                     "Час затримки відповіді повинено бути в діапазоні ["
                     + rootJsonObj.value("Modbus").toObject().value("TimeoutReplyMin").toString()
                     + "-"
                     + rootJsonObj.value("Modbus").toObject().value("TimeoutReplyMax").toString()
                     + "] мс");
        return;
    }


    if (ui->leReplyDelay->text().isDetached()) {
        errorMessage("Помилка конфігурації",
                     "Інтервал тиші не заданий");
        return;
    } else if (VALUE_IN_RANGE(ui->leSilentInterval->text().toInt(),
                              rootJsonObj.value("Modbus").toObject().value("SilentIntervalMin").toString().toInt(),
                              rootJsonObj.value("Modbus").toObject().value("SilentIntervalMax").toString().toInt())
               == false ) {
        errorMessage("Помилка конфігурації",
                     "Інтервал тиші повинено бути в діапазоні ["
                     + rootJsonObj.value("Modbus").toObject().value("SilentIntervalMin").toString()
                     + "-"
                     + rootJsonObj.value("Modbus").toObject().value("SilentIntervalMax").toString()
                     + "] мс");
        return;
    }

    if (ui->leReplyDelay->text().isDetached()) {
        errorMessage("Помилка конфігурації",
                     "Тривалість брязкіту не заданий");
        return;
    } else if (VALUE_IN_RANGE(ui->leDebounceInterval->text().toInt(),
                              rootJsonObj.value("TS").toObject().value("DebounceMin").toString().toInt(),
                              rootJsonObj.value("TS").toObject().value("DebounceMax").toString().toInt())
               == false ) {
        errorMessage("Помилка конфігурації",
                     "Тривалість брязкіту повинно бути в діапазоні ["
                     + rootJsonObj.value("TS").toObject().value("DebounceMin").toString()
                     + "-"
                     + rootJsonObj.value("TS").toObject().value("DebounceMax").toString()
                     + "] мс");
        return;
    }

    if (ui->leReplyDelay->text().isDetached()) {
        errorMessage("Помилка конфігурації",
                     "Тривалість імпульсу ТК не заданий");
        return;
    } else if (VALUE_IN_RANGE(ui->lePulsDuration->text().toInt(),
                              rootJsonObj.value("TC").toObject().value("PulsDurationMin").toString().toInt(),
                              rootJsonObj.value("TC").toObject().value("PulsDurationMax").toString().toInt())
               == false ) {
        errorMessage("Помилка конфігурації",
                     "Тривалість імпульсу ТК повинно бути в діапазоні ["
                     + rootJsonObj.value("TC").toObject().value("PulsDurationMin").toString()
                     + "-"
                     + rootJsonObj.value("TC").toObject().value("PulsDurationMax").toString()
                     + "] мс");
        return;
    }

    if (ui->cbBaudRate->currentIndex() == -1) {
        errorMessage("Помилка конфігурації",
                     "Швидкість не задана");
        return;
    }
    if (ui->cbParity->currentIndex() == -1) {
        errorMessage("Помилка конфігурації",
                     "Паритет не заданий");
        return;
    }
    if (ui->cbParity->currentIndex() == -1) {
        errorMessage("Помилка конфігурації",
                     "Кількість стоп бітів не задано");
        return;
    }

    foreach(auto item, tsSetingsList) {
        if (item->isConfigurationSeted() == false) {
            errorMessage("Помилка конфігурації",
                         "Інверсія ТС не задана");
        }
    }

    /*
     * Read user configuration and serialiase it to the SlaveConfiguration
     * structure
     */
    configuration.communication.baudRate = ui->cbBaudRate->currentText().toInt();
    configuration.communication.parity = parityUiToSerilaLUT.value(ui->cbParity->currentText());
    configuration.communication.stopBits = stopUiToSerilaLUT.value(ui->cbStopBits->currentText());
    configuration.communication.replyDelay = ui->leReplyDelay->text().toInt();
    configuration.communication.silentInterval = ui->leSilentInterval->text().toInt();
    configuration.signalisation.debounsInterval = ui->leDebounceInterval->text().toInt();
    for (uint32_t k = 0; k < TELESIGNAL_NUMBERS; k++) {
        configuration.signalisation.isInvers[k] = tsSetingsList[k]->isInvert();
    }
    configuration.control.pulsDuration = ui->lePulsDuration->text().toInt();
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
    updateUiDeviceMetaInfStr();
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
        for (uint32_t k = 0; k < TELESIGNAL_NUMBERS; k++) {
            if (state.error220 == false) {
                tsStatus[k]->setStatus(state.signalisation[k] ? TsStatus::TS_OK_ON : TsStatus::TS_OK_OFF);
            } else {
                tsStatus[k]->setStatus(TsStatus::TS_ERROR);
            }

        }

        ui->pbVoltageOnTcStatus->setChecked(state.error220);
        ui->pbEepromStatus->setChecked(state.errorEeprom);
        ui->pbEepromClearStatus->setChecked(state.errorEepromClear);
        ui->pbConfigurationError->setChecked(state.errorConfiguration);

        /*
         * Update Tele control indication
         *
         * The communicaiotn return 3 registers (1 puls and 2 static)
         */
        for (int k = 0; k < TELECONTROL_TOTAL_NUMBERS; k++) {
            tcMonitorList[k]->setStateTextIndication(state.control[k] == false ? 0 : 1);
        }

        /*
         * Update blinker
         */
        ui->pbConnectionStatus->setChecked(true);
    }
}

void Mdio::readSlaveState(void)
{
    emit readState(CB_WRAP_2(Mdio, readStateResult), connectSlaveAddress);
    stateRequestCnt++;
    updateUiCommunicationStatisticStr();
    //ui->pbConnectionStatus->
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
    ui->cbBaudRate->setCurrentText(rootJsonObj.value("Port").toObject().value("BrDefault").toString());
    ui->cbParity->setCurrentText(rootJsonObj.value("Port").toObject().value("ParityDefault").toString());
    ui->cbStopBits->setCurrentText(rootJsonObj.value("Port").toObject().value("StopBitsDefault").toString());
    ui->leSilentInterval->setText(rootJsonObj.value("Modbus").toObject().value("SilentIntervalDefault").toString());
    ui->leReplyDelay->setText(rootJsonObj.value("Modbus").toObject().value("TimeoutReplyDefault").toString());
    ui->leDebounceInterval->setText(rootJsonObj.value("TS").toObject().value("DebounceDefault").toString());
    ui->lePulsDuration->setText(rootJsonObj.value("TC").toObject().value("PulsDurationDefault").toString());
    ui->lePulsDuration->setText(rootJsonObj.value("DoubleTs").toObject().value("DoubleTsSwitchTimeDefault").toString());
    foreach(auto item, tsSetingsList) {
        item->setInvert(false);
    }
}

void Mdio::on_pbConnect_clicked()
{
    connectWithSettings();
}

void Mdio::on_cbBaudRate_currentIndexChanged(int index)
{
    if (index >= silentIntervalLIst.size()
        || index < 0) {
        return;
    }
    ui->leSilentInterval->setText(silentIntervalLIst[index]);
}
