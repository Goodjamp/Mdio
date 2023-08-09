#include "tsstatus.h"
#include "ui_tsstatus.h"

TsStatus::TsStatus(int index, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TsStatus)
{
    ui->setupUi(this);

    ui->lName->setText("ТС" + QString::number(index));
}

TsStatus::~TsStatus()
{
    delete ui;
}

void TsStatus::setStatus(bool set)
{
    ui->pbTsState->setChecked(set);
}
