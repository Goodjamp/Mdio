#include "dialogconnectionsettings.h"
#include "ui_dialogconnectionsettings.h"
#include <QtGui>
#include <QRegularExpression>
#include <QValidator>
#include <QMessageBox>
#include <QDebug>

DialogConnectionSettings::DialogConnectionSettings(QStringList comList,
                                                   QStringList brList, int defBr,
                                                   QStringList parityList, int defParity,
                                                   QStringList stopBitsList, int defStopBits,
                                                   QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogConnectionSettings)
{
    ui->setupUi(this);

    defaultBrIndex = defBr;
    defaultParityIndex = defParity;
    defaultStopBitsIndex = defStopBits;

    ui->cbPort->addItems(comList);
    ui->cbBaudRate->addItems(brList);
    ui->cbParity->addItems(parityList);
    ui->cbStopBits->addItems(stopBitsList);

    setWindowTitle("Параметри з'єднання з ПК");
    ui->leAddress->setValidator(new QRegExpValidator((QRegExp)"\\d{1,3}", this));

    using ::operator|;
    setWindowFlags(Qt::CustomizeWindowHint | Qt::WindowTitleHint);
    setDefaultUi();
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
    QVector<int> settings(SETTINGS_CNT);

    if (ui->leAddress->text().toUInt() > 255) {
        QMessageBox *errorAddressMessage = new QMessageBox(QMessageBox::Warning,
                                                           "Помилка адреси пристрою",
                                                           "Адреса пристрою має бути < 255",
                                                           QMessageBox::Ok,
                                                           this);
        errorAddressMessage->setWindowIcon((QIcon)":/Resources/CompanyIcon.png");
        errorAddressMessage->show();
    } else {
        settings[PORT] = ui->cbPort->currentIndex();
        settings[PARITY] = ui->cbParity->currentIndex();
        settings[STOP_BITS] = ui->cbStopBits->currentIndex();
        settings[BR] = ui->cbBaudRate->currentIndex();
        settings[ADDRESS] = ui->leAddress->text().toUInt();

        emit applySettings(settings);

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
