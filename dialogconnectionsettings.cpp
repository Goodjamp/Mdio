#include "dialogconnectionsettings.h"
#include "ui_dialogconnectionsettings.h"
#include <QtGui>
#include <QRegularExpression>
#include <QValidator>
#include <QMessageBox>
#include <QDebug>

DialogConnectionSettings::DialogConnectionSettings(UiFilingList uiFillingList,
                                                   UserSettingsList currentSettings,
                                                   QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogConnectionSettings)
{
    ui->setupUi(this);

    defaultBrIndex = uiFillingList.defBr;
    defaultParityIndex = uiFillingList.defParity;
    defaultStopBitsIndex = uiFillingList.defStopBits;

    ui->cbPort->addItems(uiFillingList.comList);
    ui->cbBaudRate->addItems(uiFillingList.brList);
    ui->cbParity->addItems(uiFillingList.parityList);
    ui->cbStopBits->addItems(uiFillingList.stopBitsList);

    ui->cbPort->setCurrentIndex(currentSettings.portIndex);
    ui->cbBaudRate->setCurrentIndex(currentSettings.brIndex);
    ui->cbParity->setCurrentIndex(currentSettings.parityIndex);
    ui->cbStopBits->setCurrentIndex(currentSettings.stopBitsIndex);
    ui->leAddress->setText(QString::number(currentSettings.address));

    setWindowTitle("Параметри з'єднання з ПК");
    ui->leAddress->setValidator(new QRegExpValidator((QRegExp)"\\d{1,3}", this));

    using ::operator|;
    setWindowFlags(Qt::CustomizeWindowHint | Qt::WindowTitleHint);
}

void DialogConnectionSettings::setDefaultUi(){
    ui->cbBaudRate->setCurrentIndex(defaultBrIndex);
    ui->cbParity->setCurrentIndex(defaultParityIndex);
    ui->cbStopBits->setCurrentIndex(defaultStopBitsIndex);
    ui->leAddress->setText("1");
}

DialogConnectionSettings::~DialogConnectionSettings()
{
    delete ui;
}

void DialogConnectionSettings::customClose()
{
    this->close();
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

        currentSettings.portIndex = ui->cbPort->currentIndex();
        currentSettings.parityIndex = ui->cbParity->currentIndex();
        currentSettings.stopBitsIndex = ui->cbStopBits->currentIndex();
        currentSettings.brIndex = ui->cbBaudRate->currentIndex();
        currentSettings.address = ui->leAddress->text().toUInt();

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
