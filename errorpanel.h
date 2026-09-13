#ifndef ERRORPANEL_H
#define ERRORPANEL_H

#include <QObject>
#include <QWidget>
#include <QPushButton>
#include <QFrame>
#include <QVector>
#include <errorindicator.h>
#include <QVector>

class ErrorPanel : public QWidget
{
    Q_OBJECT
public:
    explicit ErrorPanel(QWidget *parent = nullptr);
    void setErrorState(int item, bool isError);

private:
    QPushButton *pbTemp;
    QFrame *fBackground;
    QVector<ErrorIndicator *> indicatoList;
};

#endif // ERRORPANEL_H
