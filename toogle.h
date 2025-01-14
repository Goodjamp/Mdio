#ifndef TSPOLLSWITCHER_H
#define TSPOLLSWITCHER_H

#include <QWidget>
#include <QString>
#include <QMouseEvent>

namespace Ui {
class Toogle;
}

class Toogle : public QWidget
{
    Q_OBJECT

public:
    explicit Toogle(QString onLable, QString offLable, QWidget *parent = nullptr);
    ~Toogle();

    typedef enum {
        TOOGLE_STATE_ON,
        TOOGLE_STATE_OFF,
    } ToogleState;

    ToogleState getState(void);
    void addToolTip(QString onToolTip, QString offTooTip);

private slots:

    void on_pbToogle_toggled(bool checked);

private:
    Ui::Toogle *ui;

    void mousePressEvent(QMouseEvent *event) override;

};

#endif // TSPOLLSWITCHER_H
