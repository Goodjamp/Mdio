#ifndef TSSETTINGS_H
#define TSSETTINGS_H

#include <QWidget>
#include <QComboBox>

namespace Ui {
class TsSettings;
}

class TsSettings : public QWidget
{
    Q_OBJECT

public:
    explicit TsSettings(int index, QWidget *parent = nullptr);
    ~TsSettings();
    bool isInvert();
    void setInvert(bool invert);
    void setEnableCb(bool);

    /*
     * Return true if user settings is valid
     */
    bool isConfigurationSeted();

private:
    Ui::TsSettings *ui;
};

#endif // TSSETTINGS_H
