#ifndef TELECONTROL_H
#define TELECONTROL_H

#include <QWidget>
#include <QVector>
#include <QString>
#include <QMap>
#include <QStyle>
#include <QFrame>
#include <QLabel>
#include <QPushButton>

class TeleControlItem : public QWidget
{
    Q_OBJECT
public:
    enum ControlType {
        ON,
        OFF,
    };

    enum ControlState {
        STATE_ON,
        STATE_OFF,
        STATE_RELEASED,
        STATE_INACTIVE,
        STATE_UNDEFINED,
    };

    struct Descriptor {
        int id;
        QString name;
        ControlType controlType;
    };

    TeleControlItem(QWidget *parent, const Descriptor &descriptorIn);
    ~TeleControlItem(){};

    void setState(ControlState state){
        if (state == STATE_ON) {
            if (descriptor.controlType == ON) {
                contButton->setProperty("tcState", "Tc_On");
            } else {
                contButton->setProperty("tcState", "Tc_Released");
            }
        } else if (state == STATE_OFF) {
            if (descriptor.controlType == OFF) {
                contButton->setProperty("tcState", "Tc_Off");
            } else {
                contButton->setProperty("tcState", "Tc_Released");
            }
        } else if (state == STATE_RELEASED) {
            contButton->setProperty("tcState", "Tc_Released");
        } else if (state == STATE_INACTIVE) {
            contButton->setProperty("tcState", "Tc_Inactive");
        } else if (state == STATE_UNDEFINED) {
           contButton->setProperty("tcState", "Tc_Undefined");
        } else {
            return;
        }
        contButton->style()->unpolish(contButton);
        contButton->style()->polish(contButton);
    };

    void disable(){
        contButton->setProperty("tcState", "Tc_Inactive");
        contButton->style()->unpolish(contButton);
        contButton->style()->polish(contButton);
        contButton->setEnabled(false);
    }

    void enable(){
        contButton->setEnabled(true);
    }

    QString getName()const{return descriptor.name;};
signals:
    void clickedSignal(int id);

private slots:
    void clickedSlot(bool isChecked) {
        (void)isChecked;
        emit clickedSignal(descriptor.id);
    }

private:
    QPushButton *contButton;
    QFrame *fBackground;
    Descriptor descriptor;
};








class TeleControl : public QWidget
{
    Q_OBJECT

public:
    struct Descriptor {
        QVector<TeleControlItem::Descriptor> items;
        QString name;
    };
    bool containId(int id) {
        return controlsList.contains(id);
    }

    void setState(int id, TeleControlItem::ControlState state) {
         controlsList.value(id)->setState(state);
    }

    void disable() {
        for (const auto &item: controlsList) {
            item->disable();
        }
    }

    void enable() {
        for (const auto &item: controlsList) {
            item->enable();
        }
    }

    explicit TeleControl(QWidget *parent, const Descriptor &descriptorIn);
    ~TeleControl(){};

signals:
    void updateState(int id);

private slots:
    void clickedSlot(int id) {
        lLastAction->setText(controlsList.value(id)->getName());
        emit updateState(id);
    }

private:
    QMap<int, TeleControlItem*> controlsList;
    QFrame *fBackground;
    QLabel *lLastAction;
};

#endif // TELECONTROL_H
