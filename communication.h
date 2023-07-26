#ifndef B_H
#define B_H

#include <QObject>
#include <QDebug>
#include <QThread>
#include "serialcommunication.h"

#include "a.h"

#define CB_WRAP(CLASS,FUN)    std::bind(&CLASS::FUN, this, std::placeholders::_1)

/*
 * Class B is a wrapper for the Modbus class + Communication class (A) implementation.
 * The Communication class implements access to the target interface: comm port,  Ethernet other.
 * In class B, we runing processing Mobuss class and the Communication class (A) on the
 * dedicated thread. Also, class B provides multithreading communication of the items
 * from the other threads  with  Modbus class + Communication class
 * The B class provides multithreading safe wrapping of the MB methods. See the example below:
 *
 * B method       B signal         A slot        MB method
 * fun1            fun1Sig        fun1Slot         fun1
 *
 */
class Communication : public SerialCommunication
{
    Q_OBJECT

public:
    Communication(int inArg) {
        var1 = inArg;
        communicationThread = new QThread();
        classA = new A(inArg + 1);

        /*
         * The classB item is running on the dedicated thread in the event loop mode.
         * That is why we need to communicate with A by using signals.
         */
        classA->moveToThread(communicationThread);

        connect(this, &Communication::fun1Sig, classA, &A::fun1Slot);
        communicationThread->start();
    }

    ~Communication(){}


    bool fun1(std::function<void(int)> cb)
    {
        emit fun1Sig(cb);
        return true;
    }

signals:
    void fun1Sig(std::function<void(int)> cb);

private:
    QThread *communicationThread;
    int var1;
    A *classA;

};

#endif // B_H
