#ifndef B_H
#define B_H

#include <QObject>
#include <QDebug>
#include <QThread>
#include "serialcommunication.h"
#include "modbusrtumaster.h"

#include "a.h"

#define CB_WRAP(CLASS,FUN)    std::bind(&CLASS::FUN, this, std::placeholders::_1)
//#define CALL_CB(x,...)        if (x != NULL) {x(__VA_ARGS__);}
#define TELESIGNAL_NUMBERS    4

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
        modbus = new ModbusRtuMaster();
        var1 = inArg;
        communicationThread = new QThread();
        //classA = new A(inArg + 1);

        /*
         * The classB item is running on the dedicated thread in the event loop mode.
         * That is why we need to communicate with A by using signals->slot.
         */
        connect(this, &Communication::connectSlave, this, &Communication::connectSlaveSlot);
        connect(this, &Communication::disconnectSlave, this, &Communication::disconnectSlaveSlot);
        connect(this, &Communication::writeConfiguration, this, &Communication::writeConfigurationSlot);
        connect(this, &Communication::readConfiguration, this, &Communication::readConfigurationSlot);
        connect(this, &Communication::apply, this, &Communication::applySlot);
        connect(this, &Communication::reload, this, &Communication::reloadSlot);
        connect(this, &Communication::readState, this, &Communication::readStateSlot);
        connect(this, &Communication::setTeleControl, this, &Communication::setTeleControlSlot);



        this->moveToThread(communicationThread);

        //connect(this, &Communication::fun1Sig, classA, &A::fun1Slot);
        communicationThread->start();

    }

    ~Communication(){}


    bool fun1(std::function<void(int)> cb)
    {
        emit fun1Sig(cb);
        return true;
    }

public:
    typedef struct {
        struct {
            int baudRate;
            int silentInterval;
            int replyDelay;
            SerialCommunication::SerialPortParity parity;
            SerialCommunication::SerialPortStopBits stopBits;
        } communication;
        struct {
            int debounsInterval;
            bool isInvers[TELESIGNAL_NUMBERS];
        } signalisation;
        struct {
            int pulsInterval;
        } control;
    } SlaveSettings;

    typedef struct {
        bool alarm220;
        bool signalisation[TELESIGNAL_NUMBERS];
        bool control[TELESIGNAL_NUMBERS];
    } SlaveState;

signals:
    void fun1Sig(std::function<void(int)> cb);

    void connectSlave(std::function<void(bool result, int fwVersion,
                                             int yearConf, int monthConf, int dayConf)> cb,
                          QString port, int baudRate, int slaveAddress,
                          SerialCommunication::SerialPortParity parity,
                          SerialCommunication::SerialPortStopBits stopBits);
    void disconnectSlave();

    void writeConfiguration(std::function<void(bool result)> cb, SlaveSettings settings);
    void readConfiguration(std::function<void(bool result, SlaveSettings settings)> cb);
    void apply(std::function<void(bool result)> cb);
    void reload(std::function<void(bool result)> cb);
    void readState(std::function<void(bool result, SlaveState state)> cb);
    void setTeleControl(std::function<void(bool result)> cb, int index, bool enable);

private:
    QThread *communicationThread;
    int var1;
    A *classA;
    ModbusRtuMaster *modbus;


private slots:
    void connectSlaveSlot(std::function<void(bool result, int fwVersion,
                                             int yearConf, int monthConf, int dayConf)> cb,
                          QString port, int baudRate, int slaveAddress,
                          SerialCommunication::SerialPortParity parity,
                          SerialCommunication::SerialPortStopBits stopBits);
    void disconnectSlaveSlot();

    void writeConfigurationSlot(std::function<void(bool result)> cb, SlaveSettings settings);
    void readConfigurationSlot(std::function<void(bool result, SlaveSettings settings)> cb);
    void applySlot(std::function<void(bool result)> cb);
    void reloadSlot(std::function<void(bool result)> cb);
    void readStateSlot(std::function<void(bool result, SlaveState state)> cb);
    void setTeleControlSlot(std::function<void(bool result)> cb, int index, bool enable);
};

#endif // B_H
