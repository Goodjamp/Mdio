#ifndef TSSTATUS_H
#define TSSTATUS_H

#include <QWidget>
#include <QMap>
#include <QString>

namespace Ui {
class TsStatus;
}

class TsStatus : public QWidget
{
    Q_OBJECT

public:
    typedef enum {
        TS_OK_ON,
        TS_OK_OFF,
        TS_ERROR,
    } TsState;

public:
    explicit TsStatus(int index, QWidget *parent = nullptr);
    ~TsStatus();
    void setStatus(TsState state);

private:
    Ui::TsStatus *ui;
    QMap<TsState, QString> tsStyle{
        {TS_OK_ON, "Ok_On"},
        {TS_OK_OFF, "Ok_Off"},
        {TS_ERROR, "Error"},
    };
};

#endif // TSSTATUS_H
