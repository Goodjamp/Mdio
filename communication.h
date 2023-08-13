#ifndef B_H
#define B_H

#include <QObject>
#include <QDebug>
#include <QThread>
#include <QMap>
#include "serialcommunication.h"
#include "modbusrtumaster.h"

#include "a.h"

#define CB_WRAP_5(CLASS,FUN)    std::bind(&CLASS::FUN, this, std::placeholders::_1, \
                                          std::placeholders::_2, \
                                          std::placeholders::_3, \
                                          std::placeholders::_4, \
                                          std::placeholders::_5)
#define CALL_CB(x,...)        if (x != NULL) {x(__VA_ARGS__);}

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
private:
    typedef enum {
        /*
         * Generyc registers
         */
        ADDRESS_TS = 0x0000,
        ADDRESS_GLOBAL_STATUS = 0x0004,

        /*
         * State registers
         */
        ADDRESS_TELE_CONTROL_BASE = 0x0009,
        ADDRESS_TELE_CONTROL_1 = 0x0009,
        ADDRESS_TELE_CONTROL_2 = 0x000A,
        ADDRESS_TELE_CONTROL_3 = 0x000B,

        /*
         * settings registers
         */
        ADDRESS_VERSION_FW = 0x03E8,
        ADDRESS_LAST_CONFIGURATION = 0x03E9,

        /*
         * Communication settings
         */
        ADDRESS_COMMUNICATION_BAUDRATE = 0x03EA,
        ADDRESS_COMMUNICATION_SETTINGS = 0x03EB,
        ADDRESS_COMMUNICATION_SILENTS_INTERVAL = 0x03EC,
        ADDRESS_COMMUNICATION_REPLY_DELAY = 0x03ED,

        /*
         * Tele signal settings
         */
        ADDRESS_TS_DEBOUNCE_DELAY = 0x03EE,
        ADDRESS_TS_INVERSION_SETTINGS = 0x03EF,

        /*
         * Tele control settings
         */
        ADDRESS_TC_PULS_DURATION = 0x03F0,

        ADDRESS_TC_EEPROM_RELAY_1 = 0x03F1,
        ADDRESS_TC_EEPROM_RELAY_2 = 0x03F2,

        /*
         * Reset device control
         */
        ADDRESS_RESET = 0x03F3,
    } AddressRegisters;

    #define TELESIGNAL_NUMBERS              4
    #define TELECONTROL_NUMBERS             2
    #define TELECONTROL_PULS_NUMBERS        1
    #define TELECONTROL_TOTAL_NUMBERS       (TELECONTROL_PULS_NUMBERS + TELECONTROL_NUMBERS)
    #define RESET_MAGIC_NUMBER              0X55FF
    #define TELECONTROL_PULS_ON             0x0002
    #define TELECONTROL_PULS_OFF            0x0001

    #define STATUS_220_POS                  0
    #define STATUS_EEPROM_ERROR_POS         1
    #define STATUS_TRANSACTION_ERROR_POS    2

    #define DAY_CON_POS                     0
    #define DAY_CON_MASK                    31
    #define MONTH_CON_POS                   5
    #define MONTH_CON_MASK                  15
    #define YEAR_CON_POS                    9
    #define YEAR_CON_MASK                   127

    #define PARITY_POS                      0
    #define PARITY_MASK                     3
    #define PARITY_NONE                     0
    #define PARITY_EVEN                     2
    #define PARITY_ODD                      3

    #define STOP_BITS_POS                   2
    #define STOP_BITS_MASK                  3
    #define STOP_BITS_ONE                   0
    #define STOP_BITS_TWO                   1

    #define INVERS_SETTINGS_TI1_POS         0
    #define INVERS_SETTINGS_TI2_POS         1
    #define INVERS_SETTINGS_TI3_POS         2
    #define INVERS_SETTINGS_TI4_POS         3

public:
    Communication(){}

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
            int pulsDuration;
        } control;
    } SlaveConfiguration;

    typedef struct {
        bool error220;
        bool errorEeprom;
        bool errorTransaction;
        bool signalisation[TELESIGNAL_NUMBERS];
        bool control[TELECONTROL_TOTAL_NUMBERS];
    } SlaveState;

 signals:
    void connectSlaveReply(bool result);
    void writeConfigurationReply(bool result);
    void readConfigurationReply(bool result, SlaveConfiguration settings);
    void readMetaInformationReply(bool result, int fwVersion, int yearConf, int monthConf, int dayConf);
    void reloadReply(bool result);
    void readStateReply(bool result, SlaveState state);
    void setTeleControlReply(bool result);
    void setTeleControlPulsSlot(bool result);

private:
    QThread *communicationThread;
    int var1;
    A *classA;
    ModbusRtuMaster *modbus;

    /*
     * The constructor of const non-POD members called
     * on the moment of creating  current class
     */
    const QMap<int, SerialCommunication::SerialPortParity> parityMbToSerialLUT{
          {PARITY_NONE, SerialCommunication::NONE},
          {PARITY_EVEN, SerialCommunication::EVEN},
          {PARITY_ODD, SerialCommunication::ODD}
        };

    const QMap<int, SerialCommunication::SerialPortStopBits> stopBitsMbToSerialLUT{
          {STOP_BITS_ONE, SerialCommunication::ONE},
          {STOP_BITS_TWO, SerialCommunication::TWOO}
        };


public slots:
    /*
     * The startCommunication slot is used to init the variable on the context of dedicated
     * thread on the moment of start thread
     */
    void startCommunication(void);

    /*
     * The groupe of slots to comunicate with device
     */
    void connectSlaveSlot(QString port, int baudRate,
                          SerialCommunication::SerialPortParity parity,
                          SerialCommunication::SerialPortStopBits stopBits);
    void disconnectSlaveSlot();
    void writeConfigurationSlot(int slaveAddress, SlaveConfiguration configuration);
    void readConfigurationSlot(int slaveAddress);
    void readMetaInformationSlot(int slaveAddress);
    void reloadSlot(int slaveAddress);
    void readStateSlot(int slaveAddress);
    void setTeleControlSlot(int slaveAddress, int index, bool enable);
    void setTeleControlPulsSlot(int slaveAddress, bool enable);
};

#endif // B_H
