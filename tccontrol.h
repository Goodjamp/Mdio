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
    explicit TcControl(QString name, QWidget *parent = nullptr);
    ~TcControl();

private slots:
    void on_pbOn_clicked();

    void on_pbOff_clicked();

private:
    Ui::TcControl *ui;
};

#endif // TCCONTROL_H
