#include "dialogconnectionsettings.h"
#include "ui_dialogconnectionsettings.h"
#include <QRegularExpression>
#include <QValidator>
#include <QMessageBox>
#include <QDebug>
#include <QFile>
#include <QByteArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include <QJsonArray>


#define VALUE_IN_RANGE(value, min, max)    (((value) >= (min)) && ((value) <= (max)))

void DialogConnectionSettings::getSettingsFromJson()
{
    QFile jsonFile(":/UiSettings.json");
    QByteArray jsonContent;
    QJsonDocument uiDescrJson;
    QJsonArray temJsonArray;
    QJsonArray relayStateString;
    QJsonObject temObj;

    QStringList tempList;

    /*
     * Read JSON file with UI text settings / data
     */
    jsonFile.open(QFile::ReadOnly);
    jsonContent = jsonFile.readAll();
    jsonFile.close();

    uiDescrJson = QJsonDocument::fromJson(jsonContent);
    jsonRootObj = uiDescrJson.object();
    temObj = jsonRootObj.value("Port").toObject();

    foreach(auto item, temObj.value("BrList").toArray().toVariantList()) {
        tempList.push_back(item.toString());
    }
    ui->cbBaudRate->addItems(tempList);

    tempList.clear();
    foreach(auto item, temObj.value("ParityList").toArray().toVariantList()) {
        tempList.push_back(item.toString());
    }
    ui->cbParity->addItems(tempList);

    tempList.clear();
    foreach(auto item, temObj.value("StopBitsList").toArray().toVariantList()) {
        tempList.push_back(item.toString());
    }
    ui->cbStopBits->addItems(tempList);
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

void DialogConnectionSettings::setDefaultUi()
{
    ui->cbBaudRate->setCurrentText(jsonRootObj.value("Port").toObject().value("BrDefault").toString());
    ui->cbParity->setCurrentText(jsonRootObj.value("Port").toObject().value("ParityDefault").toString());
    ui->cbStopBits->setCurrentText(jsonRootObj.value("Port").toObject().value("StopBitsDefault").toString());
    ui->leAddress->setText(jsonRootObj.value("Modbus").toObject().value("AddressDefault").toString());
    ui->leSilentInterval->setText(jsonRootObj.value("Modbus").toObject().value("SilentIntervalDefaultPc").toString());
    ui->leReplyTimeout->setText(jsonRootObj.value("Modbus").toObject().value("TimeoutReplyDefaultPc").toString());
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
                           jsonRootObj.value("Modbus").toObject().value("TimeoutReplyMinPc").toString().toInt(),
                           jsonRootObj.value("Modbus").toObject().value("TimeoutReplyMaxPc").toString().toInt())
                           == false) {
            errorMessage("Помилка конфігурації",
                         "Час таймаут повинен бути в діапазоні ["
                         +jsonRootObj.value("Modbus").toObject().value("TimeoutReplyMinPc").toString()
                         + "-"
                         + jsonRootObj.value("Modbus").toObject().value("TimeoutReplyMaxPc").toString()
                         + "] мс");
            return;
        }

        if (VALUE_IN_RANGE(ui->leSilentInterval->text().toInt(),
                           jsonRootObj.value("Modbus").toObject().value("SilentIntervalMinPc").toString().toInt(),
                           jsonRootObj.value("Modbus").toObject().value("SilentIntervalMaxPc").toString().toInt())
                           == false) {
            errorMessage("Помилка конфігурації",
                         "Час тиші повинено бути в діапазоні ["
                         +jsonRootObj.value("Modbus").toObject().value("SilentIntervalMinPc").toString()
                         + "-"
                         + jsonRootObj.value("Modbus").toObject().value("SilentIntervalMaxPc").toString()
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
    setDefaultUi();
}
