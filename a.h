#ifndef A_H
#define A_H

#include <QObject>
#include <QDebug>

class A : public QObject
{
    Q_OBJECT

public:

    A(int inArg) {
        var1 = inArg;
    }
    ~A(){}

public slots:
    void fun1Slot(std::function<void (int)> cb)
    {
        qDebug()<<"Call MB method fun1";
        cb(var1++);
    }

private:
    int var1;

};

#endif // A_H
