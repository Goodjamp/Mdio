#ifndef B_H
#define B_H

#include <QObject>
#include <QDebug>
#include <QThread>
#include <QMap>
#include "serialcommunication.h"
#include "modbusrtumaster.h"

#define CB_WRAP_1(CLASS,FUN)    std::bind(&CLASS::FUN, this, std::placeholders::_1)
#define CB_WRAP_2(CLASS,FUN)    std::bind(&CLASS::FUN, this, std::placeholders::_1, \
                                          std::placeholders::_2)
#define CB_WRAP_5(CLASS,FUN)    std::bind(&CLASS::FUN, this, std::placeholders::_1, \
                                          std::placeholders::_2, \
                                          std::placeholders::_3, \
                                          std::placeholders::_4, \
                                          std::placeholders::_5)
#define CALL_CB(x,...)        if (x != NULL) {x(__VA_ARGS__);}

#define TELESIGNAL_NUMBERS              4
#define TELESIGNAL_BINARY_NUMBERS       2
#define TELECONTRO_STATIC_NUMBERS       2
#define TELECONTROL_PULS_NUMBERS        1
#define TELECONTROL_TOTAL_NUMBERS       (TELECONTROL_PULS_NUMBERS + TELECONTRO_STATIC_NUMBERS)
#define RESET_MAGIC_NUMBER              0X55FF
#define TELECONTROL_PULS_ON             0x0002
#define TELECONTROL_PULS_OFF            0x0001
#define TELECONTROL_UNDEFINED           0xFFFF

#define STATUS_220_POS                  0
#define STATUS_EEPROM_ERROR_POS         1
#define STATUS_TRANSACTION_ERROR_POS    2

#define DAY_CON_POS                     0
#define DAY_CON_MASK                    31
#define MONTH_CON_POS                   5
#define MONTH_CON_MASK                  15
#define YEAR_CON_POS                    9
#define YEAR_CON_MASK                   127

#define STOP_BITS_POS                   0
#define STOP_BITS_MASK                  1
#define STOP_BITS_ONE                   0
#define STOP_BITS_TWO                   1

#define PARITY_POS                      8
#define PARITY_MASK                     3
#define PARITY_NONE                     0
#define PARITY_EVEN                     2
#define PARITY_ODD                      3

#define INVERS_SETTINGS_TI1_POS         0
#define INVERS_SETTINGS_TI2_POS         1
#define INVERS_SETTINGS_TI3_POS         2
#define INVERS_SETTINGS_TI4_POS         3


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
        ADDR_REG_TS = 0x0000,
        ADDR_REG_GLOBAL_STATUS = 0x0004,

        /*
         * Tele control registers
         */
        ADDR_REG_TELE_CONTROL_1 = 0x0009,
        ADDR_REG_TELE_CONTROL_2 = 0x000A,
        ADDR_REG_TELE_CONTROL_3 = 0x000B,

        /*
         * settings registers
         */
        ADDR_REG_VERSION_FW = 0x03E8,
        ADDR_REG_DATE_CONFIGURATION = 0x03E9,

        /*
         * Communication settings
         */
        ADDR_REG_COMMUNICATION_BAUDRATE = 0x03EA,
        ADDR_REG_COMMUNICATION_SETTINGS = 0x03EB,
        ADDR_REG_COMMUNICATION_SILENTS_INTERVAL = 0x03EC,
        ADDR_REG_COMMUNICATION_REPLY_DELAY = 0x03ED,

        /*
         * Tele signal settings
         */
        ADDR_REG_TS_DEBOUNCE_DELAY = 0x03EE,
        ADDR_REG_TS_INVERSION_SETTINGS = 0x03EF,
        ADDR_REG_TS_BINARY_SWITCHING_TIME = 0x03F0,
        ADDR_REG_TS_BINATY_SWITCHING_SETTINGS = 0x03F1,

        /*
         * Tele control settings
         */
        ADDR_REG_TC_PULS_DURATION = 0x03F2,

        /*
         * Reset device control
         */
        ADDR_REG_RESET = 0x03F5,
    } AddrReg;

    typedef enum {
        ADDR_COIL_TC_1 = 0,
        ADDR_COIL_TC_2 = 1,
        ADDR_COIL_TC_3 = 2,
        ADDR_COIL_TC_4 = 3,

        ADDR_COIL_220_V_ERROR = 4,
        ADDR_COIL_EEPROM_ERROR = 6,
        ADDR_COIL_EEPROM_CLEAR_ERROR = 7
    } AddrCoil;

public:
    Communication(){}
    ~Communication(){}

public:
    typedef enum {
        TELECONTROL_PULS_STATE_ON,
        TELECONTROL_PULS_STATE_OFF,
        TELECONTROL_PULS_STATE_UNDEFINED,
    } TelecontrolState;

    typedef struct {
        int configurationDay;
        int configurationMonth;
        int configurationYear;
        struct {
            int baudRate;
            SerialCommunication::SerialPortParity parity;
            SerialCommunication::SerialPortStopBits stopBits;
            int silentInterval;
            int replyDelay;
        } communication;
        struct {
            int debounsInterval;
            bool isInvers[TELESIGNAL_NUMBERS];
            int binaryTsSwitchingTime;
            bool isBinary[TELESIGNAL_BINARY_NUMBERS];
        } signalisation;
        struct {
            int pulsDuration;
        } control;
    } SlaveConfiguration;

    typedef struct {
        bool error220;
        bool errorEeprom;
        bool errorEepromClear;
        bool signalisation[TELESIGNAL_NUMBERS];
        TelecontrolState control[TELECONTROL_TOTAL_NUMBERS];
    } SlaveState;

    typedef struct {
        int fwVersion;
        int configurationDay;
        int configurationMonth;
        int configurationYear;
    } MetaInformation;

private:
    QThread *communicationThread;
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
    void connectSlaveSlot(std::function<void(bool result)> cb,
                          QString port, int baudRate,
                          SerialCommunication::SerialPortParity parity,
                          SerialCommunication::SerialPortStopBits stopBits,
                          int readReplyTimeoute);
    void disconnectSlaveSlot(void);
    void writeConfigurationSlot(std::function<void(bool result)> cb,
                                int slaveAddress, SlaveConfiguration configuration);
    void readConfigurationSlot(std::function<void(bool result, SlaveConfiguration settings)> cb,
                               int slaveAddress);
    void readMetaInformationSlot(std::function<void(bool result, MetaInformation metaInformation)> cb,
                                 int slaveAddress);
    void reloadSlot(std::function<void(bool result)> cb,
                    int slaveAddress);
    void readStateSlot(std::function<void(bool result, SlaveState state)> cb,
                       int slaveAddress);
    void setTeleControlSlot(std::function<void(bool result)> cb,
                            int slaveAddress, int index, bool enable);
    void setTeleControlPulsSlot(std::function<void(bool result)> cb,
                                int slaveAddress, bool enable);
};

#endif // B_H
