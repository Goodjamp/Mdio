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
                                      QStringList brList,
                                      QStringList parityList,
                                      QStringList stopBitsList,
                                      QWidget *parent = nullptr);
    ~DialogConnectionSettings();

private:
    void customClose();

private slots:
    void on_pbApply_clicked();
    void on_pbClose_clicked();

signals:
    void applySettings(QVector<int>);

private:
    Ui::DialogConnectionSettings *ui;
};

#endif // DIALOGCONNECTIONSETTINGS_H
