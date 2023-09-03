#ifndef DIALOGCONNECTIONSETTINGS_H
#define DIALOGCONNECTIONSETTINGS_H

#include <QDialog>
#include <QVector>
#include <QStringList>

namespace Ui {
class DialogConnectionSettings;
}

class DialogConnectionSettings : public QDialog
{
    Q_OBJECT

public:
    typedef enum {
        PORT,
        PARITY,
        STOP_BITS,
        BR,
        ADDRESS,
        SETTINGS_CNT
    } SettingsIndex;

    typedef struct {
        QStringList comList;
        QStringList brList;
        int brDefault;
        QStringList parityList;
        int parityDefault;
        QStringList stopBitsList;
        int stopBitsDefault;
    } UiFilingList;

    typedef struct {
        int portIndex;
        int brIndex;
        int parityIndex;
        int stopBitsIndex;
        int address;
    } UserSettingsList;
    explicit DialogConnectionSettings(UiFilingList uiFillingList,
                                      UserSettingsList currentSettings,
                                      QWidget *parent = nullptr);
    ~DialogConnectionSettings();

private:
    void customClose();
    void setDefaultUi();

private slots:
    void on_pbApply_clicked();
    void on_pbClose_clicked();

    void on_byDefault_clicked();

signals:
    void applySettings(UserSettingsList);

private:
    Ui::DialogConnectionSettings *ui;

    int defaultBrIndex;
    int defaultParityIndex;
    int defaultStopBitsIndex;
};

#endif // DIALOGCONNECTIONSETTINGS_H
