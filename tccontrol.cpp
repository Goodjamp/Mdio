#include <QStyle>
#include <QString>
#include <QDebug>
#include "tccontrol.h"
#include "ui_tccontrol.h"

TcControl::TcControl(QString name, int index, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TcControl)
{
    ui->setupUi(this);
    userIndex = index;

    ui->lTcName->setText(name);
    stateIndex = 0;
}

void TcControl::setName(QString name)
{
    ui->lTcName->setText(name);
}

void TcControl::setTextStateList(QStringList textListStr1, QStringList textListStr2)
{
    stateTextListStr1 = textListStr1;
    stateTextListStr2 = textListStr2;
    setStateTextIndication(stateIndex);
}

QPushButton *TcControl::getOnButtonPointer()
{
    return ui->pbOn;
}

QPushButton *TcControl::getOffButtonPointer()
{
    return ui->pbOff;
}

TcControl::~TcControl()
{
    delete ui;
}

void TcControl::setStateTextIndication(int targetStateIndex)
{   
    if (targetStateIndex >= stateTextListStr1.size()
        || targetStateIndex >= stateTextListStr2.size()) {
        qDebug()<<"TcControl targetStateIndex error: "<<targetStateIndex;
        return;
    }
    ui->lStateTextStr1->setText(stateTextListStr1[targetStateIndex]);
    ui->lStateTextStr2->setText(stateTextListStr2[targetStateIndex]);
    stateIndex = targetStateIndex;
}


void TcControl::on_pbOn_clicked()
{
    emit setState(userIndex, true);
}


void TcControl::on_pbOff_clicked()
{
    emit setState(userIndex, false);
}

