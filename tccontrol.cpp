#include "tccontrol.h"
#include "ui_tccontrol.h"

TcControl::TcControl(QString name, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TcControl)
{
    ui->setupUi(this);
    ui->lTc->setText(name);
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
