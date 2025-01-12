#include "tspollswitcher.h"
#include "ui_tspollswitcher.h"

#include <QStyle>
#include <QDebug>
#include <QGraphicsItem>

#define DEFINE_SWITCHER_SIZE    40
#define DEFINE_TEXT_WIDTH       150
#define DEFINE_TEXT_HEIGHT      30

TsPollSwitcher::TsPollSwitcher(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::TsPollSwitcher)
{
    ui->setupUi(this);
    ui->pbPollingSwitcher->setCheckable(true);
    on_pbPollingSwitcher_toggled(false);
    ui->pbPollingSwitcher->setMinimumSize(DEFINE_SWITCHER_SIZE, DEFINE_SWITCHER_SIZE);
    ui->pbPollingSwitcher->setMaximumSize(ui->pbPollingSwitcher->minimumSize());

    ui->lTsPollingBinary->setMinimumSize(DEFINE_TEXT_WIDTH, DEFINE_TEXT_HEIGHT);
    ui->lTsPollingBinary->setMaximumSize(ui->lTsPollingBinary->minimumSize());

    ui->lTsPollingRegister->setMinimumSize(DEFINE_TEXT_WIDTH, DEFINE_TEXT_HEIGHT);
    ui->lTsPollingRegister->setMaximumSize(ui->lTsPollingRegister->minimumSize());
}

TsPollSwitcher::~TsPollSwitcher()
{
    delete ui;
}

TsPollSwitcher::PollingType TsPollSwitcher::getPollingType(void)
{
    return ui->pbPollingSwitcher->isChecked()
           ? TsPollSwitcher::POLLING_TYPE_BINARY
           : TsPollSwitcher::POLLING_TYPE_REGISTER;
}

void TsPollSwitcher::mousePressEvent(QMouseEvent *event)
{
    QWidget *child = childAt(event->pos());
    if (event->type() == QInputEvent::MouseButtonPress) {
        if (child == (QWidget *)ui->lTsPollingBinary) {
            ui->pbPollingSwitcher->setChecked(true);
        } else if (child == (QWidget *)ui->lTsPollingRegister) {
            ui->pbPollingSwitcher->setChecked(false);
            qDebug()<<"Register";
        }
    }
}

void TsPollSwitcher::on_pbPollingSwitcher_toggled(bool checked)
{
    if (checked) {
        ui->lTsPollingBinary->setProperty("TsPollingType", "Bin");
        ui->lTsPollingRegister->setProperty("TsPollingType", "Disable");
    } else {
        ui->lTsPollingBinary->setProperty("TsPollingType", "Disable");
        ui->lTsPollingRegister->setProperty("TsPollingType", "Register");
    }

    ui->lTsPollingBinary->style()->unpolish(ui->lTsPollingBinary);
    ui->lTsPollingBinary->style()->polish(ui->lTsPollingBinary);

    ui->lTsPollingRegister->style()->unpolish(ui->lTsPollingRegister);
    ui->lTsPollingRegister->style()->polish(ui->lTsPollingRegister);
}

