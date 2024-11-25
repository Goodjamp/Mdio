#ifndef DIALOGCONNECTIONSETTINGS_H
#define DIALOGCONNECTIONSETTINGS_H

#include <QDialog>
#include <QVector>
#include <QString>
#include <QStringList>

namespace Ui {
class DialogConnectionSettings;
}

class DialogConnectionSettings : public QDialog
{
    Q_OBJECT

public:
    typedef struct {
        QStringList comList;
    } UiFilingList;

    typedef struct {
        QString port;
        QString br;
        QString parity;
        QString stopBits;
        QString address;
        QString replyTimeout;
        QString silentInterval;
    } UserSettingsList;
    explicit DialogConnectionSettings(UiFilingList uiFillingList,
                                      UserSettingsList currentSettings,
                                      QWidget *parent = nullptr);
    ~DialogConnectionSettings();

private:
    void customClose();
    void setDefaultUi();
    void getSettingsFromJson();

private slots:
    void on_pbApply_clicked();
    void on_pbClose_clicked();

    void on_byDefault_clicked();

    void leAddressEditFinish();
    void leAddressEdited(const QString &text);

signals:
    void applySettings(UserSettingsList);

private:
    Ui::DialogConnectionSettings *ui;

    void errorMessage(QString headr, QString detailed);
};

#endif // DIALOGCONNECTIONSETTINGS_H
