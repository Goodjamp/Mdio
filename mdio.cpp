#include "mdio.h"
#include "ui_mdio.h"

#include <QDebug>
#include <QMap>
#include <QThread>
#include <QMessageBox>
#include <dialogconnectionsettings.h>
#include <modbusrtumaster.h>

#define TC_STATIC_NUMBER    2
#define TC_PULS_NUMBER      1

QStringList brList = {"1200", "2400", "4800", "9600", "14400", "19200", "28800", "38400", "57600"};
QStringList parityList = {"None", "Even", "Odd"};
QStringList stopBitsList = {"1", "2"};

void Mdio::initCustomUi()
{
    /*
     * Add TeleControl statuc/control items
     */
    tcPuls = new TcControl("Імпульсне");
    ui->vlTcControlMonitorInternal->addWidget(tcPuls);
    for (uint32_t k = 0; k < TC_STATIC_NUMBER; k++) {
        tcStatic.append(new TcControl("Статичне " + QString::number(k + 1)));
        ui->vlTcControlMonitorInternal->addWidget(tcStatic[tcStatic.size() - 1]);
    }
    tcLayoutSpacer = new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Expanding);
    ui->vlTcControlMonitorInternal->addItem(tcLayoutSpacer);

    /*
     * Add TeleSignalisation configuration items
     */
    for (uint32_t k = 0; k < TELESIGNAL_NUMBERS; k++) {
        tsSetings.append(new TsSettings(k + 1));
        ui->vlTeleSignalSettings->addWidget(tsSetings[tsSetings.size() - 1]);
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
    setWindowTitle("МДВВ-4-2 конфігуратор");
    setWindowIcon((QIcon)":/Resources/CompanyIcon.png");

    ui->cbBaudRate->addItems(brList);
    ui->cbStopBits->addItems(stopBitsList);
    ui->cbParity->addItems(parityList);
}


Mdio::Mdio(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::Mdio)
{
    ui->setupUi(this);

    commmunicationThread = new QThread();
    communicaiton = new Communication();

    initCustomUi();
    updateUiConnectionStatusStr(false);
    updateUiDeviceMetaInfStr(false);

    connect(this, &Mdio::connectSlave, communicaiton, &Communication::connectSlaveSlot);
    connect(this, &Mdio::disconnectSlave, communicaiton, &Communication::disconnectSlaveSlot);
    connect(this, &Mdio::readConfiguration, communicaiton, &Communication::readConfigurationSlot);
    connect(this, &Mdio::apply, communicaiton, &Communication::reloadSlot);
    connect(this, &Mdio::reload, communicaiton, &Communication::reloadSlot);
    connect(this, &Mdio::readState, communicaiton, &Communication::readStateSlot);
    connect(this, &Mdio::setTeleControl, communicaiton, &Communication::setTeleControlSlot);
    connect(this, &Mdio::readMetaInformation, communicaiton, &Communication::readMetaInformationSlot);

    /*
     * Reply sugnal/slot family
     */
    connect(communicaiton, &Communication::connectSlaveReply, this, &Mdio::connectSlaveResult);
    connect(communicaiton, &Communication::reloadReply, this, &Mdio::reloadResult);
    connect(communicaiton, &Communication::readMetaInformationReply, this, &Mdio::readMetaInformationResult);
    connect(communicaiton, &Communication::readConfigurationReply, this, &Mdio::readConfigurationResult);

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
    qDebug()<<"MDIO constructor";
}

Mdio::~Mdio()
{
    delete ui;
}

void Mdio::updateUiConnectionStatusStr(bool isConnect)
{
    QString connectionSettingsStr;
    if (isConnect == true) {
        QStringList comList = Communication::getPortsList();
        connectionSettingsStr = comList[connectPortIndex]
                                + " "
                                + brList[connectBrIndex]
                                + " "
                                + "8"
                                + parityList[connectParityIndex]
                                + stopBitsList[connectStopBitsIndex]
                                + " "
                                + "Адр."
                                + QString::number(connectSlaveAddress);
    } else {
        connectionSettingsStr = "Від'єднано";
    }

    ui->lConnectionSettings->setText(connectionSettingsStr);
}

void Mdio::updateUiDeviceMetaInfStr(bool isConnect)
{
    QString metaInfStr;
    if (isConnect == true) {
        metaInfStr = "v."
                     + QString::number(connectDeviceVersion)
                     + " "
                     + QString::number(connectDeviceConfDay) + "."
                     + QString::number(connectDeviceConfMonth) + "."
                     + QString::number(connectDeviceConfYear);
    } else {
        metaInfStr = "v.0 0.0.0";
    }


    ui->lDeviceMetaInfo->setText(metaInfStr);
}

void Mdio::updateUiConfiguration(void)
{

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

void Mdio::connectSlaveResult(bool result)
{
    /*
     * Release (give) semaphore to unblok code that waite to complete
     */
    communicationSyncSem.release(1);
    communicationResult = result;
    qDebug()<<"Connect rez:"<<result;
}

void Mdio::readMetaInformationResult(bool result, int fwVersion,
                                     int yearConf, int monthConf, int dayConf)
{
    /*
     * Release (give) semaphore to unblok code that waite to complete
     */
    communicationResult = result;
    qDebug()<<"Read meta information result: "<<result;
    if (result == true) {
        connectDeviceVersion = fwVersion;
        connectDeviceConfYear = yearConf;
        connectDeviceConfMonth = monthConf;
        connectDeviceConfDay = dayConf;
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

    }
    communicationSyncSem.release(1);
}


void Mdio::reloadResult(bool result)
{
    qDebug()<<"Restart result: "<<result;
}

void Mdio::applyConnectionSettings(QVector<int> connectionSettings)
{
    connectPortIndex = connectionSettings[DialogConnectionSettings::PORT];
    connectBrIndex = connectionSettings[DialogConnectionSettings::BR];
    connectParityIndex = connectionSettings[DialogConnectionSettings::PARITY];
    connectStopBitsIndex = connectionSettings[DialogConnectionSettings::STOP_BITS];
    connectSlaveAddress = connectionSettings[DialogConnectionSettings::ADDRESS];
}

bool Mdio::processingCommunicatitonResult(QString headr, QString detailed)
{
    if (communicationSyncSem.tryAcquire(1, COMMUNICATION_COMPLETE_TIMEOUTE)
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

void Mdio::on_pbConnectionSettings_clicked()
{
    QStringList comList = Communication::getPortsList();
    DialogConnectionSettings *dialogConnectionSettings = new DialogConnectionSettings(comList, brList, parityList, stopBitsList);
    SerialCommunication::SerialPortParity parity;
    SerialCommunication::SerialPortStopBits stopBits;

    dialogConnectionSettings->setModal(true);
    connect(dialogConnectionSettings, &DialogConnectionSettings::applySettings, this, &Mdio::applyConnectionSettings);
    needConnectSlave = false;
    dialogConnectionSettings->show();

    /*
     * If user push
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
    emit connectSlave(comList[connectPortIndex],
                      brList[connectBrIndex].toInt(),
                      parity,
                      stopBits);
    /*
     * Waite to complete connection
     */
    if (processingCommunicatitonResult("Неможливо приєднатися",
                                        "Порт недоступний") == false) {
        return;
    }

    /*
     * Read meta information
     */
    emit readMetaInformation(connectSlaveAddress);
    if (processingCommunicatitonResult("Неможливо приєднатися",
                                       "Помилка считування метаінформації") == false) {
        return;
    }

    /*
     * Read configuration
     */
    emit readConfiguration(connectSlaveAddress);
    if (processingCommunicatitonResult("Неможливо приєднатися",
                                       "Помилка считування мета конфігурації") == false) {
        return;
    }

    updateUiConnectionStatusStr(true);
    updateUiDeviceMetaInfStr(true);
}

void Mdio::on_pbApplySettings_clicked()
{

}

void Mdio::on_pbDisconnect_clicked()
{
    emit disconnectSlave();
}

void Mdio::on_pbReload_clicked()
{
    emit reload(connectSlaveAddress);
}

void Mdio::on_pbReadSettings_clicked()
{
    emit readMetaInformation(connectSlaveAddress);
}
