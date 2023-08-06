#include "mdio.h"
#include "ui_mdio.h"

#include <QDebug>
#include <dialogconnectionsettings.h>
#include <modbusrtumaster.h>

#define TC_STATIC_NUMBER    2
#define TC_PULS_NUMBER      1

QStringList brList = {"1200", "2400", "4800", "9600", "14400", "19200", "28800", "38400", "57600"};
QStringList parityList = {"None", "Even", "Odd"};
QStringList stopBitsList = {"1", "2"};

void Mdio::initCustomUi()
{
    tcPuls = new TcControl("Імпульсне");
    ui->vlTcControlMonitorInternal->addWidget(tcPuls);
    for (uint32_t k = 0; k < TC_STATIC_NUMBER; k++) {
        tcStatic.append(new TcControl("Статичне " + QString::number(k + 1)));
        ui->vlTcControlMonitorInternal->addWidget(tcStatic[tcStatic.size() - 1]);
    }
    tcLayoutSpacer = new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Expanding);
    ui->vlTcControlMonitorInternal->addItem(tcLayoutSpacer);

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

    communicaiton = new Communication(5);
    initCustomUi();
    updateConnectionStatusStr();
    updateDeviceMetaInfStr();

    //ui->pushButton->setText("RUN");
}

Mdio::~Mdio()
{
    delete ui;
}

void Mdio::testCbFun(int) {
    volatile int cnt = 0;
    static int cnt2 = 0;
    qDebug()<<"Bgin Mdio testCbFun";
    while (cnt++ < 2000000000){}
    //ui->pushButton_2->setText(QString::number(cnt2++));
    qDebug()<<"Complete Mdio testCbFun";
}

/*
void Mdio::on_pushButton_clicked()
{
    classB->fun1(CB_WRAP(Mdio, testCbFun));
}

void Mdio::on_pushButton_2_clicked()
{
    QRect geom = ui->pushButton_2->geometry();
    static int cnt = 0;
    int x = rand.bounded(0, ui->centralwidget->geometry().width() - geom.width());
    int y = rand.bounded(0, ui->centralwidget->geometry().height() - geom.height());
    geom.setRect(x, y, geom.width(), geom.height());

    ui->pushButton_2->setGeometry(geom);


    qDebug()<<"cnt = "<<cnt++;
}
*/

void Mdio::updateConnectionStatusStr(void)
{
    QStringList comList = Communication::getPortsList();
    QString connectionSettingsStr = comList[connectPortIndex]
                                    + " "
                                    + brList[connectBrIndex]
                                    + " "
                                    + "8"
                                    + parityList[connectParityIndex]
                                    + stopBitsList[connectStopBitsIndex]
                                    + " "
                                    + "Адр."
                                    + QString::number(connectAddress);
    /*
                                    + "  "
                                    + "v."
                                    + QString::number(connectDeviceVersion)
                                    + " "
                                    + QString::number(connectDeviceConfDay) + "."
                                    + QString::number(connectDeviceConfMonth) + "."
                                    + QString::number(connectDeviceConfYear);
*/
    ui->lConnectionSettings->setText(connectionSettingsStr);
}

void Mdio::updateDeviceMetaInfStr()
{
    QString metaInfStr = "v."
                         + QString::number(connectDeviceVersion)
                         + " "
                         + QString::number(connectDeviceConfDay) + "."
                         + QString::number(connectDeviceConfMonth) + "."
                         + QString::number(connectDeviceConfYear);

    ui->lDeviceMetaInfo->setText(metaInfStr);
}

void Mdio::applyConnectionSettings(QVector<int> connectionSettings)
{
    connectPortIndex = connectionSettings[DialogConnectionSettings::PORT];
    connectBrIndex = connectionSettings[DialogConnectionSettings::BR];
    connectParityIndex = connectionSettings[DialogConnectionSettings::PARITY];
    connectStopBitsIndex = connectionSettings[DialogConnectionSettings::STOP_BITS];
    connectAddress = connectionSettings[DialogConnectionSettings::ADDRESS];

    updateConnectionStatusStr();
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
