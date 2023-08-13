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
#define MB_EXCEPTION_CODE(x)           (UINT8_MAX & ((uint8_t)(1 << 8) | (x)))
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
        MB_RX_EXEPTION = 1,
        MB_OK = 0,
        MB_RX_SIZE_ERROR = -1,
        MB_CRC_ERROR,
        MB_ADDRESS_ERROR,
        MB_FUNCTION_ERROR,
        MB_BYTES_NUMBER_ERROR,
        MB_TARGET_FUNCTION_ERROR,
        MB_SEND_ERROR,
        MB_REPLY_ERROR,
    } MbStatus;

private:

    static constexpr uint16_t crcTable[256] = {
        0x0000, 0xC0C1, 0xC181, 0x0140, 0xC301, 0x03C0, 0x0280, 0xC241,
        0xC601, 0x06C0, 0x0780, 0xC741, 0x0500, 0xC5C1, 0xC481, 0x0440,
        0xCC01, 0x0CC0, 0x0D80, 0xCD41, 0x0F00, 0xCFC1, 0xCE81, 0x0E40,
        0x0A00, 0xCAC1, 0xCB81, 0x0B40, 0xC901, 0x09C0, 0x0880, 0xC841,
        0xD801, 0x18C0, 0x1980, 0xD941, 0x1B00, 0xDBC1, 0xDA81, 0x1A40,
        0x1E00, 0xDEC1, 0xDF81, 0x1F40, 0xDD01, 0x1DC0, 0x1C80, 0xDC41,
        0x1400, 0xD4C1, 0xD581, 0x1540, 0xD701, 0x17C0, 0x1680, 0xD641,
        0xD201, 0x12C0, 0x1380, 0xD341, 0x1100, 0xD1C1, 0xD081, 0x1040,
        0xF001, 0x30C0, 0x3180, 0xF141, 0x3300, 0xF3C1, 0xF281, 0x3240,
        0x3600, 0xF6C1, 0xF781, 0x3740, 0xF501, 0x35C0, 0x3480, 0xF441,
        0x3C00, 0xFCC1, 0xFD81, 0x3D40, 0xFF01, 0x3FC0, 0x3E80, 0xFE41,
        0xFA01, 0x3AC0, 0x3B80, 0xFB41, 0x3900, 0xF9C1, 0xF881, 0x3840,
        0x2800, 0xE8C1, 0xE981, 0x2940, 0xEB01, 0x2BC0, 0x2A80, 0xEA41,
        0xEE01, 0x2EC0, 0x2F80, 0xEF41, 0x2D00, 0xEDC1, 0xEC81, 0x2C40,
        0xE401, 0x24C0, 0x2580, 0xE541, 0x2700, 0xE7C1, 0xE681, 0x2640,
        0x2200, 0xE2C1, 0xE381, 0x2340, 0xE101, 0x21C0, 0x2080, 0xE041,
        0xA001, 0x60C0, 0x6180, 0xA141, 0x6300, 0xA3C1, 0xA281, 0x6240,
        0x6600, 0xA6C1, 0xA781, 0x6740, 0xA501, 0x65C0, 0x6480, 0xA441,
        0x6C00, 0xACC1, 0xAD81, 0x6D40, 0xAF01, 0x6FC0, 0x6E80, 0xAE41,
        0xAA01, 0x6AC0, 0x6B80, 0xAB41, 0x6900, 0xA9C1, 0xA881, 0x6840,
        0x7800, 0xB8C1, 0xB981, 0x7940, 0xBB01, 0x7BC0, 0x7A80, 0xBA41,
        0xBE01, 0x7EC0, 0x7F80, 0xBF41, 0x7D00, 0xBDC1, 0xBC81, 0x7C40,
        0xB401, 0x74C0, 0x7580, 0xB541, 0x7700, 0xB7C1, 0xB681, 0x7640,
        0x7200, 0xB2C1, 0xB381, 0x7340, 0xB101, 0x71C0, 0x7080, 0xB041,
        0x5000, 0x90C1, 0x9181, 0x5140, 0x9301, 0x53C0, 0x5280, 0x9241,
        0x9601, 0x56C0, 0x5780, 0x9741, 0x5500, 0x95C1, 0x9481, 0x5440,
        0x9C01, 0x5CC0, 0x5D80, 0x9D41, 0x5F00, 0x9FC1, 0x9E81, 0x5E40,
        0x5A00, 0x9AC1, 0x9B81, 0x5B40, 0x9901, 0x59C0, 0x5880, 0x9841,
        0x8801, 0x48C0, 0x4980, 0x8941, 0x4B00, 0x8BC1, 0x8A81, 0x4A40,
        0x4E00, 0x8EC1, 0x8F81, 0x4F40, 0x8D01, 0x4DC0, 0x4C80, 0x8C41,
        0x4400, 0x84C1, 0x8581, 0x4540, 0x8701, 0x47C0, 0x4680, 0x8641,
        0x8201, 0x42C0, 0x4380, 0x8341, 0x4100, 0x81C1, 0x8081, 0x4040 };

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

    /*
     * Hight level read function:
     * - read with timeoute
     * - test size
     * - test crc
     * - test slave address
     * - test target function
     */
    MbStatus receiveReply(QByteArray *rxData, int timeoute,
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
                             uint32_t timeoute);

    MbStatus writeSlaveSingleRegister(uint8_t slaveAddress, ModbusRtuMaster::FunList function,
                                      uint16_t address, uint16_t value, uint32_t timeoute);

    MbStatus writeSlaveMultipleRegisters(uint8_t slaveAddress, ModbusRtuMaster::FunList function,
                                         uint16_t address, QVector<uint16_t> value, uint32_t timeoute);

public:

    typedef enum {
        COIL_ON = 0xFF00,
        COIL_OFF = 0x0000,
    } CoilState;

    explicit ModbusRtuMaster(QObject *parent = nullptr);

    QString getStatusString(ModbusRtuMaster::MbStatus status) {
        if (statusStr.contains(status)) {
            return statusStr.value(status);
        }

        return static_cast<QString>("Wrong status");
    }

    /*
     * Read
     */
    MbStatus readCoilStatus(uint8_t slaveAddress, uint16_t coilAddress,
                            uint16_t coilsNumber, QVector<bool> &coilState); // F_1
    MbStatus readDiscreteInputs(uint8_t slaveAddress, uint16_t coilAddress,
                                uint16_t coilsNumber, QVector<bool> &coilState); // F_2
    MbStatus readHoldingRegisters(uint8_t slaveAddress, uint16_t regStartAddress,
                                  uint16_t registersNumber, QVector<uint16_t> &regValue); // F_3
    MbStatus readInputRegisters(uint8_t slaveAddress, uint16_t regStartAddress,
                                uint16_t registersNumber, QVector<uint16_t> &regValue); // F_4
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
