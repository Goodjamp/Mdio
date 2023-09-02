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

void TcControl::setTextStateList(QStringList textList)
{
    stateTextList = textList;
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
    if (targetStateIndex >= stateTextList.size()) {
        qDebug()<<"TcControl targetStateIndex";
        return;
    }
    ui->lStateText->setText(stateTextList[targetStateIndex]);
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

