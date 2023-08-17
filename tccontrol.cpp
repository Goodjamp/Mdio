#include "tccontrol.h"
#include "ui_tccontrol.h"

TcControl::TcControl(QString name, int index, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TcControl)
{
    ui->setupUi(this);

    ui->lTc->setText(name);
    ui->pbTcState->setEnabled(false);
    userIndex = index;
}

TcControl::~TcControl()
{
    delete ui;
}

void TcControl::on_pbOn_clicked()
{
    emit setControlState(userIndex, true);
}

void TcControl::on_pbOff_clicked()
{
    emit setControlState(userIndex, false);
}

void TcControl::setState(bool enable)
{

}
