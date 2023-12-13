#ifndef TCCONTROL_H
#define TCCONTROL_H

#include <QWidget>
#include <QString>
#include <QPushButton>
#include <QButtonGroup>

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
    void setTextStateList(QStringList textListStr1, QStringList textListStr2);
    QPushButton *getOnButtonPointer();
    QPushButton *getOffButtonPointer();
    void setOnButtonState(bool state);
    void setOffButtonState(bool state);
    void unchekAllButton();

signals:
    void setState(int index, bool enable);

private slots:
    void on_pbOn_clicked();
    void on_pbOff_clicked();

private:
    Ui::TcControl *ui;
    int userIndex;
    QStringList stateTextListStr1;
    QStringList stateTextListStr2;
    QButtonGroup *bgRelayControl;// = new QButtonGroup();
    int stateIndex;
};

#endif // TCCONTROL_H
