#include <QStyle>
#include <QString>
#include "tccontrol.h"
#include "ui_tccontrol.h"

QString redColor = "background-color: rgb(255, 100, 100)";
QString greenColor = "background-color: rgb(50, 200, 50)";

TcControl::TcControl(QString nameStatic, QString namePuls, int index, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TcControl)
{
    ui->setupUi(this);
    userIndex = index;

    ui->pbStaticOn->setText(nameStatic + " ВВІМКНУТИ\n СТАТИЧНО");
    ui->pbStaticOff->setText(nameStatic + " ВИМКНУТИ\n СТАТИЧНО");
    ui->pbPuls->setText(namePuls + "\n ІМПУЛЬСНО");
    setStaticState(false);

    checkButtonsList = new QButtonGroup();
    checkButtonsList->addButton(ui->pbStaticOn);
    checkButtonsList->addButton(ui->pbStaticOff);
    checkButtonsList->addButton(ui->pbPuls);
}

TcControl::~TcControl()
{
    delete ui;
}

void TcControl::setStaticState(bool enable)
{   
    ui->lTcState->setText(" RC"
                          + QString::number(userIndex + 1)
                          + " "
                          + (enable ? "ON" : "OFF"));
    ui->lTcState->setProperty("RemoteControl", enable);
    ui->lTcState->style()->unpolish(ui->lTcState);
    ui->lTcState->style()->polish(ui->lTcState);
    //ui->pbTcState->setChecked(enable);
}

void TcControl::on_pbStaticOn_clicked()
{
    emit setStaticControlState(userIndex, true);
}

void TcControl::on_pbStaticOff_clicked()
{
    emit setStaticControlState(userIndex, false);
}

void TcControl::on_pbPuls_clicked()
{
    emit setPulsControl(userIndex);
}
