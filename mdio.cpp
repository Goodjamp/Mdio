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
    updateConnectionStatusStr(false);
    updateDeviceMetaInfStr(false);

    connect(this, &Mdio::connectSlave, communicaiton, &Communication::connectSlaveSlot);
    connect(this, &Mdio::disconnectSlave, communicaiton, &Communication::disconnectSlaveSlot);
    connect(this, &Mdio::writeConfiguration, communicaiton, &Communication::writeConfigurationSlot);
    connect(this, &Mdio::readConfiguration, communicaiton, &Communication::readConfigurationSlot);
    connect(this, &Mdio::apply, communicaiton, &Communication::applySlot);
    connect(this, &Mdio::reload, communicaiton, &Communication::reloadSlot);
    connect(this, &Mdio::readState, communicaiton, &Communication::readStateSlot);
    connect(this, &Mdio::setTeleControl, communicaiton, &Communication::setTeleControlSlot);
    connect(this, &Mdio::readMetaInformation, communicaiton, &Communication::readMetaInformationSlot);

    /*
     * Reply sugnla/slot family
     */
    connect(communicaiton, &Communication::connectSlaveReply, this, &Mdio::connectSlaveResult);
    connect(communicaiton, &Communication::applyReply, this, &Mdio::restartResult);
    connect(communicaiton, &Communication::readMetaInformationReply, this, &Mdio::readMetaInformationResult);

    connect(commmunicationThread, &QThread::started, communicaiton, &Communication::startCommunication);
    /*
     * connect(worker, finished, thread, quit);
     * connect(worker, finished, worker, deleteLater);
     * connect(thread, finished, thread, deleteLater);
     */
    communicaiton->moveToThread(commmunicationThread);
    commmunicationThread->start();
}

Mdio::~Mdio()
{
    delete ui;
}

void Mdio::updateConnectionStatusStr(bool isConnect)
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

void Mdio::updateDeviceMetaInfStr(bool isConnect)
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

void Mdio::connectSlaveResult(bool result)
{
    if (result == false) {
        QMessageBox *errorAddressMessage = new QMessageBox(QMessageBox::Critical,
                                                           "Неможливо приєднатися",
                                                           "Порт не доступний",
                                                           QMessageBox::Ok,
                                                           this);
        errorAddressMessage->setWindowIcon((QIcon)":/Resources/CompanyIcon.png");
        errorAddressMessage->show();
        updateConnectionStatusStr(false);
        return;
    }

    qDebug()<<"Connect rez:"<<result;
}

void Mdio::readMetaInformationResult(bool result, int fwVersion,
                                     int yearConf, int monthConf, int dayConf)
{
    qDebug()<<"Read meta information result: "<<result;
    if (result == false) {
        return;
    }
    connectDeviceVersion = fwVersion;
    connectDeviceConfYear = yearConf;
    connectDeviceConfMonth = monthConf;
    connectDeviceConfDay = dayConf;

    /*
     * Update string indication of the connection settings
     */
    updateDeviceMetaInfStr(true);
}


void Mdio::restartResult(bool result)
{
    qDebug()<<"Restart result: "<<result;
}

void Mdio::applyConnectionSettings(QVector<int> connectionSettings)
{
    QMap<int, SerialCommunication::SerialPortParity> parityConvert{
        {0, SerialCommunication::NONE},
        {1, SerialCommunication::EVEN},
        {2, SerialCommunication::ODD}
    };
    QMap<int, SerialCommunication::SerialPortStopBits> stopBitsConvert{
        {0, SerialCommunication::ONE},
        {1, SerialCommunication::TWOO}
    };
    connectPortIndex = connectionSettings[DialogConnectionSettings::PORT];
    connectBrIndex = connectionSettings[DialogConnectionSettings::BR];
    connectParityIndex = connectionSettings[DialogConnectionSettings::PARITY];
    connectStopBitsIndex = connectionSettings[DialogConnectionSettings::STOP_BITS];
    connectSlaveAddress = connectionSettings[DialogConnectionSettings::ADDRESS];

    /*
     * Open connection
     */
    SerialCommunication::SerialPortParity parity;
    SerialCommunication::SerialPortStopBits stopBits;
    QStringList comList = Communication::getPortsList();

    if(parityConvert.contains(connectionSettings[DialogConnectionSettings::PARITY])) {
        parity = parityConvert.value(connectionSettings[DialogConnectionSettings::PARITY]);
    } else {
        return;
    }

    if(stopBitsConvert.contains(connectionSettings[DialogConnectionSettings::STOP_BITS])) {
        stopBits = stopBitsConvert.value(connectionSettings[DialogConnectionSettings::STOP_BITS]);
    } else {
        return;
    }
    emit connectSlave(comList[connectionSettings[DialogConnectionSettings::PORT]],
                      brList[connectionSettings[DialogConnectionSettings::BR]].toInt(),
                      parity,
                      stopBits);
}

void Mdio::on_pbConnectionSettings_clicked()
{
    QStringList comList = Communication::getPortsList();
    DialogConnectionSettings *dialogConnectionSettings = new DialogConnectionSettings(comList, brList, parityList, stopBitsList);
    dialogConnectionSettings->setModal(true);
    connect(dialogConnectionSettings, &DialogConnectionSettings::applySettings, this, &Mdio::applyConnectionSettings);
    dialogConnectionSettings->show();
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
    emit apply(connectSlaveAddress);
}

void Mdio::on_pbReadSettings_clicked()
{
    emit readMetaInformation(connectSlaveAddress);
}
