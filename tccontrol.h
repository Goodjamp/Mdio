#ifndef TCCONTROL_H
#define TCCONTROL_H

#include <QWidget>
#include <QString>
#include <QPushButton>

namespace Ui {
class TcControl;
}

class TcControl : public QWidget
{
    Q_OBJECT

public:
    explicit TcControl(QString name, int index, QWidget *parent);
    ~TcControl();

    void setStateTextIndication(int stateIndex);
    void setName(QString name);
    void setTextStateList(QStringList textList);
    QPushButton *getOnButtonPointer();
    QPushButton *getOffButtonPointer();

signals:
    void setState(int index, bool enable);

private slots:
    void on_pbOn_clicked();
    void on_pbOff_clicked();

private:
    Ui::TcControl *ui;
    int userIndex;
    QStringList stateTextList;
    int stateIndex;
};

#endif // TCCONTROL_H
