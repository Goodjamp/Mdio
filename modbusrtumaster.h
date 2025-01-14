#ifndef MODBUSRTUMASTER_H
#define MODBUSRTUMASTER_H

#include <QObject>
#include <QWidget>
#include <stdint.h>
#include <QWidget>
#include <QMap>
#include <QByteArray>
#include "serialcommunication.h"

#define LITTE_TO_BIG_ENDIANS_U16(x)    (UINT16_MAX & ((x) >> 8 | (x) << 8))
#define MB_EXCEPTION_CODE(x)           (UINT8_MAX & ((uint8_t)(1 << 7) | (x)))
#define MB_SLAVE_ADDRESS_POS           0
#define MB_FUNCTION_CODE_POS           1
#define MB_RX_BYTES_NUMBER_POS         2
#define MB_RX_BITS_PAYLOAD_POS         3
#define MB_TX_BYTES_NUMBER_POS         6

/*
 * The minimum size of the command to read the information from the slave.
 * The minimum command size is reached if the requested 0 bits.
 */
#define MB_MINIMUM_COMMAND_SIZE        5

/*
 * The size of thE exeption reply
 */
#define MB_EXEPTION_REPLY_SIZE         5

class ModbusRtuMaster : public SerialCommunication
{
    Q_OBJECT
public:

    typedef enum {
        COIL_ON = 0xFF00,
        COIL_OFF = 0x0000,
    } CoilState;

    typedef enum {
        MB_RX_EXEPTION = 1,
        MB_OK = 0,
        MB_RX_SIZE_ERROR = -1,
        MB_CRC_ERROR = -2,
        MB_ADDRESS_ERROR = -3,
        MB_FUNCTION_ERROR = -4,
        MB_BYTES_NUMBER_ERROR = -5,
        MB_TARGET_FUNCTION_ERROR = -6,
        MB_SEND_ERROR = -7,
        MB_REPLY_ERROR = -8,
    } MbStatus;

private:

    typedef enum {
        READ_COIL_STATUS = 1,
        READ_DISCRET_INPUTS = 2,
        READ_HOLDING_REGISTERS = 3,
        READ_INPUT_REGISTERS = 4,
        FORCE_SINGLE_COIL = 5,
        PRESET_SINGLE_REGISTER = 6,
        FORCE_MULTIPLE_COILS = 15,
        PRESET_MULTIPLE_REGISTER = 16,
    } FunList;

    const QMap<MbStatus, QString> statusStr{
        {MB_RX_EXEPTION, "MB_RX_EXEPTION"},
        {MB_OK, "MB_OK"},
        {MB_RX_SIZE_ERROR, "MB_RX_SIZE_ERROR"},
        {MB_CRC_ERROR, "MB_CRC_ERROR"},
        {MB_ADDRESS_ERROR, "MB_ADDRESS_ERROR"},
        {MB_FUNCTION_ERROR, "MB_FUNCTION_ERROR"},
        {MB_BYTES_NUMBER_ERROR, "MB_BYTES_NUMBER_ERROR"},
        {MB_TARGET_FUNCTION_ERROR, "MB_TARGET_FUNCTION_ERROR"},
        {MB_SEND_ERROR, "MB_SEND_ERROR"},
        {MB_REPLY_ERROR, "MB_REPLY_ERROR"},
    };

    uint16_t crc(QByteArray *buff);
    int readTimeout;

    /*
     * Hight level read function:
     * - read with timeout
     * - test size
     * - test crc
     * - test slave address
     * - test target function
     */
    MbStatus receiveReply(QByteArray *rxData, int timeout,
                         int targetSize, uint8_t slaveAddress,
                         uint8_t function);

    /*
     * This method is used to read bits and bytes from the slave over the next functions:
     * - READ_COIL_STATUS
     * - READ_DISCRET_INPUTS
     * - READ_HOLDING_REGISTERS
     * - READ_INPUT_REGISTERS
     */
    template <typename T>
    MbStatus readSlaveGeneral(uint8_t slaveAddress, ModbusRtuMaster::FunList function,
                             uint16_t address, uint16_t number, QVector<T> &state,
                             uint32_t timeout);

    MbStatus writeSlaveSingleRegister(uint8_t slaveAddress, ModbusRtuMaster::FunList function,
                                      uint16_t address, uint16_t value, uint32_t timeout);

    MbStatus writeSlaveMultipleRegisters(uint8_t slaveAddress, ModbusRtuMaster::FunList function,
                                         uint16_t address, QVector<uint16_t> value, uint32_t timeout);

public:

    explicit ModbusRtuMaster(QObject *parent = nullptr);

    QString getStatusString(ModbusRtuMaster::MbStatus status) {
        if (statusStr.contains(status)) {
            return statusStr.value(status);
        }

        return static_cast<QString>("Wrong status");
    }

    /*
     * Services
     */
    void setReadReplyTimeout(int timeout);

    /*
     * Read
     */
    /**
     * @brief Function 1, read binary
     */
    MbStatus readCoilStatus(uint8_t slaveAddress, uint16_t coilAddress,
                            uint16_t coilsNumber, QVector<bool> &coilState);

    /**
     * @brief Function 2, read binary
     */
    MbStatus readDiscreteInputs(uint8_t slaveAddress, uint16_t coilAddress,
                                uint16_t coilsNumber, QVector<bool> &coilState);

    /**
     * @brief Function 3, read registers
     */
    MbStatus readHoldingRegisters(uint8_t slaveAddress, uint16_t regStartAddress,
                                  uint16_t registersNumber, QVector<uint16_t> &regValue);

    /**
     * @brief Function 4, read registers
     */
    MbStatus readInputRegisters(uint8_t slaveAddress, uint16_t regStartAddress,
                                uint16_t registersNumber, QVector<uint16_t> &regValue);
    /*
     * Write
     */
    MbStatus forceSingleCoil(uint8_t slaveAddress, uint16_t coilAddress,
                             bool coilState);  // F_5  0xFF00 - ON, 0x0000- OFF
    MbStatus presetSingleRegister(uint8_t slaveAddress, uint16_t regAddress,
                                  uint16_t regValue); // F_6
    /*
     * Not implemented yet
     */
    MbStatus forceMultipleCoils(uint8_t slaveAddress, uint16_t coilAddress,
                                uint16_t coilsNumber, QVector<bool> &coilState); // F_15
    MbStatus  presetMultipleRegister(uint8_t slaveAddress, uint16_t regStartAddress,
                                     QVector<uint16_t> regValue); // F_16
};

#endif // MODBUSRTUMASTER_H
