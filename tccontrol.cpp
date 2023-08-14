#include "tccontrol.h"
#include "ui_tccontrol.h"

TcControl::TcControl(QString name, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TcControl)
{
    ui->setupUi(this);

    ui->fBackGround->setStyleSheet("QFrame  {background-color:rgb(150, 200, 200);}");
    ui->lTc->setText(name);
    ui->pbTcState->setEnabled(false);
}

TcControl::~TcControl()
{
    delete ui;
}

void TcControl::on_pbOn_clicked()
{

}

void TcControl::on_pbOff_clicked()
{

}
