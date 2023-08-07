#include "tssettings.h"
#include "ui_tssettings.h"

TsSettings::TsSettings(int index, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TsSettings)
{
    ui->setupUi(this);
    ui->lTS->setText("TC" + QString::number(index));
}

TsSettings::~TsSettings()
{
    delete ui;
}

bool TsSettings::isInvert()
{
    return ui->cbTS->currentIndex() == 1;
}

void TsSettings::setInver(bool invert)
{
    ui->cbTS->setCurrentIndex(invert == false ? 0 : 1);
}
