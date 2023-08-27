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
    explicit DialogConnectionSettings(QStringList comList,
                                      QStringList brList, int defBr,
                                      QStringList parityList, int defParity,
                                      QStringList stopBitsList, int defStopBits,
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
    void applySettings(QVector<int>);

private:
    Ui::DialogConnectionSettings *ui;

    int defaultBrIndex;
    int defaultParityIndex;
    int defaultStopBitsIndex;
};

#endif // DIALOGCONNECTIONSETTINGS_H
