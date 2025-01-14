#include "toogle.h"
#include "ui_toogle.h"

#include <QStyle>
#include <QDebug>
#include <QGraphicsItem>

#define DEFINE_SWITCHER_SIZE    40
#define DEFINE_TEXT_WIDTH       160
#define DEFINE_TEXT_HEIGHT      30

Toogle::Toogle(QString onLable, QString offLable, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Toogle)
{
    ui->setupUi(this);

    ui->lToogleOn->setText(onLable);
    ui->lToogleOff->setText(offLable);

    /*
     * Name
     */

    ui->pbToogle->setCheckable(true);
    ui->pbToogle->setChecked(false);
    on_pbToogle_toggled(false);

    ui->pbToogle->setMinimumSize(DEFINE_SWITCHER_SIZE, DEFINE_SWITCHER_SIZE);
    ui->pbToogle->setMaximumSize(ui->pbToogle->minimumSize());

    ui->lToogleOn->setMinimumSize(DEFINE_TEXT_WIDTH, DEFINE_TEXT_HEIGHT);
    ui->lToogleOn->setMaximumSize(ui->lToogleOn->minimumSize());

    ui->lToogleOff->setMinimumSize(DEFINE_TEXT_WIDTH, DEFINE_TEXT_HEIGHT);
    ui->lToogleOff->setMaximumSize(ui->lToogleOff->minimumSize());
}

Toogle::~Toogle()
{
    delete ui;
}

void Toogle::addToolTip(QString onToolTip, QString offTooTip)
{
    ui->lToogleOn->setToolTip(onToolTip);
    ui->lToogleOff->setToolTip(offTooTip);
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
        if (child == (QWidget *)ui->lToogleOn) {
            ui->pbToogle->setChecked(true);
        } else if (child == (QWidget *)ui->lToogleOff) {
            ui->pbToogle->setChecked(false);
        }
    }
}

void Toogle::on_pbToogle_toggled(bool checked)
{
    if (checked) {
        ui->lToogleOn->setProperty("ToogleState", "On");
        ui->lToogleOff->setProperty("ToogleState", "Disable");
    } else {
        ui->lToogleOn->setProperty("ToogleState", "Disable");
        ui->lToogleOff->setProperty("ToogleState", "Off");
    }

    ui->lToogleOn->style()->unpolish(ui->lToogleOn);
    ui->lToogleOn->style()->polish(ui->lToogleOn);

    ui->lToogleOff->style()->unpolish(ui->lToogleOff);
    ui->lToogleOff->style()->polish(ui->lToogleOff);
}

