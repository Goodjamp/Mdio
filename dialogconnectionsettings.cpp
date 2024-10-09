#include "dialogconnectionsettings.h"
#include "ui_dialogconnectionsettings.h"
#include <QRegularExpression>
#include <QValidator>
#include <QMessageBox>
#include <QDebug>
#include <QByteArray>

#include "SwDefSettings.h"

#define VALUE_IN_RANGE(value, min, max)    (((value) >= (min)) && ((value) <= (max)))

void DialogConnectionSettings::getSettingsFromJson()
{
    ui->cbBaudRate->addItems(SwDefSettings::getBrListString());
    ui->cbParity->addItems(SwDefSettings::getParityListString());
    ui->cbStopBits->addItems(SwDefSettings::getStopBitListString());
}

DialogConnectionSettings::DialogConnectionSettings(UiFilingList uiFillingList,
                                                   UserSettingsList currentSettings,
                                                   QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogConnectionSettings)
{
    ui->setupUi(this);

    ui->cbPort->addItems(uiFillingList.comList);
    getSettingsFromJson();

    ui->cbPort->setCurrentText(currentSettings.port);
    ui->cbBaudRate->setCurrentText(currentSettings.br);
    ui->cbParity->setCurrentText(currentSettings.parity);
    ui->cbStopBits->setCurrentText(currentSettings.stopBits);
    ui->leAddress->setText(currentSettings.address);
    ui->leSilentInterval->setText(currentSettings.silentInterval);
    ui->leReplyTimeout->setText(currentSettings.replyTimeout);

    setWindowTitle("Параметри з'єднання з ПК");
    ui->leAddress->setValidator(new QRegularExpressionValidator((QRegularExpression)"\\d{1,3}", this));
    ui->leSilentInterval->setValidator(new QRegularExpressionValidator((QRegularExpression)"\\d{1,3}", this));
    ui->leReplyTimeout->setValidator(new QRegularExpressionValidator((QRegularExpression)"\\d{1,4}", this));

    using ::operator|;
    setWindowFlags(Qt::CustomizeWindowHint | Qt::WindowTitleHint);
}

DialogConnectionSettings::~DialogConnectionSettings()
{
    delete ui;
}

void DialogConnectionSettings::customClose()
{
    this->close();
}

void DialogConnectionSettings::errorMessage(QString headr, QString detailed)
{
    QMessageBox *errorAddressMessage = new QMessageBox(QMessageBox::Critical,
                                                       headr,
                                                       detailed,
                                                       QMessageBox::Ok,
                                                       this);
    errorAddressMessage->setWindowIcon((QIcon)":/Resources/CompanyIcon.png");
    errorAddressMessage->show();
}

void DialogConnectionSettings::on_pbApply_clicked()
{
    if (ui->leAddress->text().toUInt() > 255) {
        QMessageBox *errorAddressMessage = new QMessageBox(QMessageBox::Warning,
                                                           "Помилка адреси пристрою",
                                                           "Адреса пристрою має бути < 255",
                                                           QMessageBox::Ok,
                                                           this);
        errorAddressMessage->setWindowIcon((QIcon)":/Resources/CompanyIcon.png");
        errorAddressMessage->show();
    } else {
        UserSettingsList currentSettings;

        if (VALUE_IN_RANGE(ui->leReplyTimeout->text().toInt(),
                           SwDefSettings::getTimeoutReplyMinPc().toInt(),
                           SwDefSettings::getTimeoutReplyMaxPc().toInt())
                           == false) {
            errorMessage("Помилка конфігурації",
                         "Час таймаут повинен бути в діапазоні ["
                             + SwDefSettings::getTimeoutReplyMinPc()
                         + "-"
                             + SwDefSettings::getTimeoutReplyMaxPc()
                         + "] мс");
            return;
        }

        if (VALUE_IN_RANGE(ui->leSilentInterval->text().toInt(),
                           SwDefSettings::getSilentIntervalMinPc().toInt(),
                           SwDefSettings::getSilentIntervalMaxPc().toInt())
                           == false) {
            errorMessage("Помилка конфігурації",
                         "Час тиші повинено бути в діапазоні ["
                             + SwDefSettings::getSilentIntervalMinPc()
                         + "-"
                             + SwDefSettings::getSilentIntervalMaxPc()
                         + "] мс");
            return;
        }

        currentSettings.port = ui->cbPort->currentText();
        currentSettings.parity = ui->cbParity->currentText();
        currentSettings.stopBits = ui->cbStopBits->currentText();
        currentSettings.br = ui->cbBaudRate->currentText();
        currentSettings.address = ui->leAddress->text();
        currentSettings.replyTimeout = ui->leReplyTimeout->text();
        currentSettings.silentInterval = ui->leSilentInterval->text();

        emit applySettings(currentSettings);

        this->customClose();
    }
}

void DialogConnectionSettings::on_pbClose_clicked()
{
    this->customClose();
}

void DialogConnectionSettings::on_byDefault_clicked()
{
    ui->cbBaudRate->setCurrentText(SwDefSettings::getBrDefaultString());
    ui->cbParity->setCurrentText(SwDefSettings::getParityDefault());
    ui->cbStopBits->setCurrentText(SwDefSettings::getStopBitsDefaultStr());
    ui->leAddress->setText(SwDefSettings::getAddressDefault());
    ui->leSilentInterval->setText(SwDefSettings::getSilentIntervalDefaultPc());
    ui->leReplyTimeout->setText(SwDefSettings::getTimeoutReplyDefaultPc());
}
