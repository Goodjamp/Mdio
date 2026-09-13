#ifndef ERRORINDICATOR_H
#define ERRORINDICATOR_H

#include <QObject>
#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QFrame>
#include <QString>
#include <QSize>

class ErrorIndicator : public QWidget
{
    Q_OBJECT
public:
    explicit ErrorIndicator(QWidget *parent = nullptr,
                            QString errName = "Undefined",
                            QSize indSize = QSize(35, 35));

    void setState(bool isError);
    void setActive();
    void setInactive();
    void setUndefined();

private:
    QPushButton *pbIndicator;
    QLabel *lName;
    QFrame *fBackground;
    bool state;
};

#endif // ERRORINDICATOR_H
