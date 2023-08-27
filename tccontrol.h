#ifndef TCCONTROL_H
#define TCCONTROL_H

#include <QWidget>
#include <QString>
#include <QButtonGroup>

namespace Ui {
class TcControl;
}

class TcControl : public QWidget
{
    Q_OBJECT

public:
    explicit TcControl(QString nameStatic, QString namePuls, int index, QWidget *parent);
    ~TcControl();

    void setStaticState(bool enable);

signals:
    void setStaticControlState(int index, bool enable);
    void setPulsControl(int index);

private slots:
    void on_pbStaticOn_clicked();

    void on_pbStaticOff_clicked();

    void on_pbPuls_clicked();

private:
    Ui::TcControl *ui;
    int userIndex;
    QButtonGroup *checkButtonsList;
};

#endif // TCCONTROL_H
