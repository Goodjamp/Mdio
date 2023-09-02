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
    void setInver(bool invert);
    QComboBox *getComboBoxPointer();

private:
    Ui::TsSettings *ui;
};

#endif // TSSETTINGS_H
