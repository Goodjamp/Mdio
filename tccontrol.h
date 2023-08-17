#ifndef TCCONTROL_H
#define TCCONTROL_H

#include <QWidget>
#include <QString>

namespace Ui {
class TcControl;
}

class TcControl : public QWidget
{
    Q_OBJECT

public:
    explicit TcControl(QString name, int index, QWidget *parent = nullptr);
    ~TcControl();

    void setState(bool enable);

signals:
    void setControlState(int index, bool enable);

private slots:
    void on_pbOn_clicked();
    void on_pbOff_clicked();

private:
    Ui::TcControl *ui;
    int userIndex;
};

#endif // TCCONTROL_H
