#include "toogle.h"
#include "ui_toogle.h"

#include <QStyle>
#include <QDebug>
#include <QGraphicsItem>

#define DEFINE_SWITCHER_SIZE    40
#define DEFINE_TEXT_WIDTH       120
#define DEFINE_TEXT_HEIGHT      50

Toogle::Toogle(QString onLable, QString offLable, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Toogle)
{
    ui->setupUi(this);

    ui->lToogleOff->setText(onLable);
    ui->lToogleOn->setText(offLable);

    ui->pbToogle->setCheckable(true);
    ui->pbToogle->setChecked(false);
    on_pbToogle_toggled(false);

    ui->pbToogle->setMinimumSize(DEFINE_SWITCHER_SIZE, DEFINE_SWITCHER_SIZE);
    ui->pbToogle->setMaximumSize(ui->pbToogle->minimumSize());

    ui->lToogleOff->setMinimumSize(DEFINE_TEXT_WIDTH, DEFINE_TEXT_HEIGHT);
    ui->lToogleOff->setMaximumSize(ui->lToogleOff->minimumSize());

    ui->lToogleOn->setMinimumSize(DEFINE_TEXT_WIDTH, DEFINE_TEXT_HEIGHT);
    ui->lToogleOn->setMaximumSize(ui->lToogleOn->minimumSize());
}

Toogle::~Toogle()
{
    delete ui;
}

void Toogle::addToolTip(QString onToolTip, QString offTooTip)
{
    ui->lToogleOff->setToolTip(onToolTip);
    ui->lToogleOn->setToolTip(offTooTip);
}

Toogle::ToogleState Toogle::getState(void)
{
    return ui->pbToogle->isChecked()
        ? Toogle::TOOGLE_STATE_ON
        : Toogle::TOOGLE_STATE_OFF;
}

void Toogle::mousePressEvent(QMouseEvent *event)
{
    QWidget *child = childAt(event->pos());
    if (event->type() == QInputEvent::MouseButtonPress) {
        if (child == (QWidget *)ui->lToogleOff) {
            ui->pbToogle->setChecked(false);
        } else if (child == (QWidget *)ui->lToogleOn) {
            ui->pbToogle->setChecked(true);
        }
    }
}

void Toogle::on_pbToogle_toggled(bool checked)
{
    if (checked) {
        ui->lToogleOff->setProperty("ToogleState", "Disable");
        ui->lToogleOn->setProperty("ToogleState", "On");
    } else {
        ui->lToogleOff->setProperty("ToogleState", "Off");
        ui->lToogleOn->setProperty("ToogleState", "Disable");
    }

    ui->lToogleOff->style()->unpolish(ui->lToogleOff);
    ui->lToogleOff->style()->polish(ui->lToogleOff);

    ui->lToogleOn->style()->unpolish(ui->lToogleOn);
    ui->lToogleOn->style()->polish(ui->lToogleOn);
}

