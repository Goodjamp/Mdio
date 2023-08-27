#include "tccontrol.h"
#include "ui_tccontrol.h"

QString redColor = "background-color: rgb(255, 100, 100)";
QString greenColor = "background-color: rgb(50, 200, 50)";

TcControl::TcControl(QString nameStatic, QString namePuls, int index, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TcControl)
{
    ui->setupUi(this);

    ui->pbStaticOn->setText(nameStatic + " ВВІМКНУТИ\n СТАТИЧНЕ");
    ui->pbStaticOff->setText(nameStatic + " ВИМКНУТИ\n СТАТИЧНЕ");
    ui->pbPuls->setText(namePuls + "\n ІМПУЛЬСНЕ");
    ui->pbTcState->setEnabled(false);
    userIndex = index;
    checkButtonsList = new QButtonGroup();
    checkButtonsList->addButton(ui->pbStaticOn);
    checkButtonsList->addButton(ui->pbStaticOff);
    checkButtonsList->addButton(ui->pbPuls);

    //checkItemList = new QLayout();

}

TcControl::~TcControl()
{
    delete ui;
}

void TcControl::setStaticState(bool enable)
{
    ui->pbTcState->setChecked(enable);
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
