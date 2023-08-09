#ifndef B_H
#define B_H

#include <QObject>
#include <QDebug>
#include <QThread>
#include "serialcommunication.h"
#include "modbusrtumaster.h"

#include "a.h"

#define CB_WRAP_5(CLASS,FUN)    std::bind(&CLASS::FUN, this, std::placeholders::_1, \
                                          std::placeholders::_2, \
                                          std::placeholders::_3, \
                                          std::placeholders::_4, \
                                          std::placeholders::_5)
#define CALL_CB(x,...)        if (x != NULL) {x(__VA_ARGS__);}
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
    Communication() {}

    ~Communication(){}


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
    void connectSlaveReply(bool result);
    void writeConfigurationReply(bool result);
    void readConfigurationReply(bool result, SlaveSettings settings);
    void applyReply(bool result);
    void reloadReply(bool result);
    void readStateReply(bool result, SlaveState state);
    void setTeleControlReply(bool result);
    void readMetaInformationReply(bool result, int fwVersion, int yearConf, int monthConf, int dayConf);

private:
    QThread *communicationThread;
    int var1;
    A *classA;
    ModbusRtuMaster *modbus;


public slots:
    void startCommunication(void);
    void connectSlaveSlot(QString port, int baudRate,
                          SerialCommunication::SerialPortParity parity,
                          SerialCommunication::SerialPortStopBits stopBits);
    void disconnectSlaveSlot();
    void writeConfigurationSlot(int slaveAddress, SlaveSettings settings);
    void readConfigurationSlot(int slaveAddress);
    void applySlot(int slaveAddress);
    void reloadSlot(int slaveAddress);
    void readStateSlot(int slaveAddress);
    void setTeleControlSlot(int slaveAddress, int index, bool enable);
    void readMetaInformationSlot(int slaveAddress);
};

#endif // B_H
