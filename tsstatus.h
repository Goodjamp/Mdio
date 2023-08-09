#ifndef TSSTATUS_H
#define TSSTATUS_H

#include <QWidget>

namespace Ui {
class TsStatus;
}

class TsStatus : public QWidget
{
    Q_OBJECT

public:
    explicit TsStatus(int index, QWidget *parent = nullptr);
    ~TsStatus();
    void setStatus(bool set);

private:
    Ui::TsStatus *ui;
};

#endif // TSSTATUS_H
