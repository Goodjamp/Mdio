#include "tsstatus.h"
#include "ui_tsstatus.h"

#include <QStyle>

TsStatus::TsStatus(int index, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TsStatus)
{
    ui->setupUi(this);

    ui->lName->setText("ТС" + QString::number(index));
    ui->pbTsState->setEnabled(false);
    setStatus(TS_OK_OFF);
}

TsStatus::~TsStatus()
{
    delete ui;
}

void TsStatus::setStatus(TsState state)
{
    ui->pbTsState->setProperty("tsState", tsStyle.value(state));
    ui->pbTsState->style()->unpolish(ui->pbTsState);
    ui->pbTsState->style()->polish(ui->pbTsState);
}
