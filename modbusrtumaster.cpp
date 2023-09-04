#include <modbusrtumaster.h>
#include <QtEndian>
#include <QTime>
#include <QThread>

#define UINT8_TO_UINT16(L, M)      UINT16_MAX & ((0x00FF & (L)) | (0xFF00 & ((M) << 8)));
#define MB_TRANSACTION_TIMEOUTE    500

// definition (no value needed)
static const uint16_t crcTable[256] = {
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

ModbusRtuMaster::ModbusRtuMaster(QObject *parent) : SerialCommunication(parent)
{

}

uint16_t ModbusRtuMaster::crc(QByteArray *buff)
{
    uint8_t xorVal = 0;
    uint16_t rezCrc = 0xFFFF;

    for (int k = 0; k <  buff->size(); k++)
    {
        xorVal = buff->at(k) ^ rezCrc;
        rezCrc >>= 8;
        rezCrc ^= crcTable[xorVal];
    }

    return rezCrc;
}

ModbusRtuMaster::MbStatus ModbusRtuMaster::receiveReply(QByteArray *rxData, int timeoute,
                                                        int targetSize, uint8_t slaveAddress,
                                                        uint8_t function)
{
    QByteArray tempReadBuff;
    uint16_t crcRx;
    uint16_t crcCalc;
    QTime endReceiveTime = QTime::currentTime().addMSecs(timeoute);

    rxData->clear();
    while(QTime::currentTime().msecsTo(endReceiveTime) > 0) {
        read(tempReadBuff);
        if (tempReadBuff.size() != 0){
            rxData->append(tempReadBuff);
            if (rxData->size() == targetSize) {
                break;
            }
        }
    }
    if (rxData->size() != targetSize
        && rxData->size() != MB_EXEPTION_REPLY_SIZE) {
        return MB_RX_SIZE_ERROR;
    }
    crcRx = UINT8_TO_UINT16(rxData->at(rxData->size() - 2), rxData->at(rxData->size() - 1));

    /*
     * remove 2 last bytes (CRC), becouse we pass rxData to the crc calculation function
     * and we don't need calculate crc from CRC bytes
     */
    rxData->remove(rxData->size() - 2, 2);
    crcCalc = crc(rxData);
    if (crcCalc != crcRx) {
        return MB_CRC_ERROR;
    }
    if (rxData->at(MB_SLAVE_ADDRESS_POS) != static_cast<char>(slaveAddress)) {
        return MB_ADDRESS_ERROR;
    }
    if (rxData->at(MB_FUNCTION_CODE_POS) != static_cast<char>(function)) {
        /*
         * If function wrong, it could be exeption reply, lets test it
         */
        if (rxData->at(MB_FUNCTION_CODE_POS) == static_cast<char>(MB_EXCEPTION_CODE(function))) {
            return MB_RX_EXEPTION;
        } else {
            return MB_ADDRESS_ERROR;
        }
    }

    return MB_OK;
}

ModbusRtuMaster::MbStatus ModbusRtuMaster::readCoilStatus(uint8_t slaveAddress, uint16_t coilAddress,
                                                          uint16_t coilsNumber, QVector<bool> &coilState)
{
    return readSlaveGeneral<bool>(slaveAddress, READ_COIL_STATUS,
                                  coilAddress, coilsNumber, coilState,
                                  MB_TRANSACTION_TIMEOUTE);
}

ModbusRtuMaster::MbStatus ModbusRtuMaster::readDiscreteInputs(uint8_t slaveAddress, uint16_t coilAddress,
                            uint16_t coilsNumber, QVector<bool> &coilState)
{
    return readSlaveGeneral<bool>(slaveAddress, READ_DISCRET_INPUTS,
                                  coilAddress, coilsNumber, coilState,
                                  MB_TRANSACTION_TIMEOUTE);
}

ModbusRtuMaster::MbStatus ModbusRtuMaster::readHoldingRegisters(uint8_t slaveAddress, uint16_t regStartAddress,
                                                                uint16_t registersNumber, QVector<uint16_t> &regValue)
{
    return readSlaveGeneral<uint16_t>(slaveAddress, READ_HOLDING_REGISTERS,
                                      regStartAddress, registersNumber, regValue,
                                      MB_TRANSACTION_TIMEOUTE);
}

ModbusRtuMaster::MbStatus ModbusRtuMaster::readInputRegisters(uint8_t slaveAddress, uint16_t regStartAddress,
                                                              uint16_t registersNumber, QVector<uint16_t> &regValue)
{
    return readSlaveGeneral<uint16_t>(slaveAddress, READ_INPUT_REGISTERS,
                                      regStartAddress, registersNumber, regValue,
                                      MB_TRANSACTION_TIMEOUTE);
}

template <typename T>
ModbusRtuMaster::MbStatus ModbusRtuMaster::readSlaveGeneral(uint8_t slaveAddress, ModbusRtuMaster::FunList function,
                                                            uint16_t address, uint16_t number, QVector<T> &state,
                                                            uint32_t timeoute)
{
    QByteArray commandBuff;
    uint16_t crcCalc;
    uint8_t payloadBytes;
    uint16_t rxSize;
    ModbusRtuMaster::MbStatus result;

    /*
     * Clear input buffer
     */
    read(commandBuff);
    commandBuff.clear();

    /*
     * Serialiase request
     */
    commandBuff.push_back(slaveAddress);
    commandBuff.push_back(function);
    commandBuff.push_back(static_cast<uint8_t>(address >> 8));
    commandBuff.push_back(static_cast<uint8_t>(address));
    commandBuff.push_back(static_cast<uint8_t>(number >> 8));
    commandBuff.push_back(static_cast<uint8_t>(number));
    crcCalc = crc(&commandBuff);
    commandBuff.push_back(static_cast<uint8_t>(crcCalc));
    commandBuff.push_back(static_cast<uint8_t>(crcCalc >> 8));

    /*
     * Send command
     */
    if (write(commandBuff) != true) {
        return MB_SEND_ERROR;
    };
    commandBuff.clear();

    /*
     * Calculation of the reply payload size and the total size of the reply
     */
    switch (function) {
    case READ_COIL_STATUS:
    case READ_DISCRET_INPUTS:
        payloadBytes = number/8 + ((number % 8) == 0 ? 0 : 1);
        break;

    case READ_HOLDING_REGISTERS:
    case READ_INPUT_REGISTERS:
        payloadBytes = number * 2;
        break;

    default:
        return MB_TARGET_FUNCTION_ERROR;
    }
    rxSize = MB_MINIMUM_COMMAND_SIZE + payloadBytes;

    /*
     * Receive reply
     */
    result = receiveReply(&commandBuff, timeoute, rxSize, slaveAddress, function);
    if (result != MB_OK) {
        return result;
    }

    /*
     * Making specific tests for the received data
     */
    if (commandBuff[MB_RX_BYTES_NUMBER_POS] != static_cast<char>(payloadBytes)) {
        return MB_BYTES_NUMBER_ERROR;
    }

    /*
     * Deserializing payload depend pn the function
     */
    switch (function) {
    case READ_COIL_STATUS:
    case READ_DISCRET_INPUTS: {
        uint16_t rest;
        int payloadPos = MB_RX_BITS_PAYLOAD_POS;
        /*
         * Deserialiaze the binary information type
         */
        while (number) {
            rest = (number > 8) ? 8 : number;
            for (uint32_t k = 0; k < rest; k++) {
                state.push_back(commandBuff[payloadPos] & 0x1 ? true : false);
                commandBuff[payloadPos] = commandBuff[payloadPos] >> 1;
            }
            payloadPos++;
            number -= rest;
        }
        break;
    }

    case READ_HOLDING_REGISTERS:
    case READ_INPUT_REGISTERS: {
        int payloadPos = MB_RX_BITS_PAYLOAD_POS;
        uint16_t data = 0;

        /*
         * Deserialiaze the bytes information.
         * Convert Big endians to Little endians
         */
        for (uint32_t k = 0; k < number; k++) {
            data = (0xFF00 & (commandBuff[payloadPos++] << 8));
            data |= (0xFF & commandBuff[payloadPos++]);
            state.push_back(data);
        }
        break;
    }
    default:
        return MB_TARGET_FUNCTION_ERROR;
    }
    return MB_OK;
}

ModbusRtuMaster::MbStatus ModbusRtuMaster::forceSingleCoil(uint8_t slaveAddress, uint16_t coilAddress,
                                                           bool coilState)
{
    return writeSlaveSingleRegister(slaveAddress, FORCE_SINGLE_COIL, coilAddress,
                                    coilState == true
                                    ? static_cast<uint16_t>(COIL_ON)
                                    : static_cast<uint16_t>(COIL_OFF),
                                    MB_TRANSACTION_TIMEOUTE);
}

ModbusRtuMaster::MbStatus ModbusRtuMaster::presetSingleRegister(uint8_t slaveAddress, uint16_t regAddress,
                                                                uint16_t regValue)
{
    return writeSlaveSingleRegister(slaveAddress, PRESET_SINGLE_REGISTER, regAddress,
                                    regValue, MB_TRANSACTION_TIMEOUTE);
}

ModbusRtuMaster::MbStatus ModbusRtuMaster::writeSlaveSingleRegister(uint8_t slaveAddress, ModbusRtuMaster::FunList function,
                                                                    uint16_t address, uint16_t value, uint32_t timeoute)
{
    QByteArray commandBuff;
    QByteArray replyBuff;
    uint16_t crcCalc;
    ModbusRtuMaster::MbStatus result;

    /*
     * Clear port
     */
    read(commandBuff);
    commandBuff.clear();

    /*
     * Serialiase request
     */
    commandBuff.push_back(slaveAddress);
    commandBuff.push_back(function);
    commandBuff.push_back(static_cast<uint8_t>(address >> 8));
    commandBuff.push_back(static_cast<uint8_t>(address));
    commandBuff.push_back(static_cast<uint8_t>(value >> 8));
    commandBuff.push_back(static_cast<uint8_t>(value));
    crcCalc = crc(&commandBuff);
    commandBuff.push_back(static_cast<uint8_t>(crcCalc));
    commandBuff.push_back(static_cast<uint8_t>(crcCalc >> 8));

    /*
     * Send command
     */
    if (write(commandBuff) != true) {
        return MB_SEND_ERROR;
    };

    /*
     * Receive reply
     */
    result = receiveReply(&replyBuff, timeoute, commandBuff.size(), slaveAddress, function);
    if (result != MB_OK) {
        return result;
    }
    if (commandBuff.size() - replyBuff.size() != 2) {
        return MB_REPLY_ERROR;
    }
    commandBuff.remove(commandBuff.size() - 2, 2);
    if (commandBuff != replyBuff) {
        return MB_REPLY_ERROR;
    }

    return MB_OK;
}

ModbusRtuMaster::MbStatus ModbusRtuMaster::presetMultipleRegister(uint8_t slaveAddress, uint16_t regStartAddress,
                                                                  QVector<uint16_t> regValue)
{
    return writeSlaveMultipleRegisters(slaveAddress, PRESET_MULTIPLE_REGISTER,
                                       regStartAddress, regValue, MB_TRANSACTION_TIMEOUTE);

}

ModbusRtuMaster::MbStatus ModbusRtuMaster::writeSlaveMultipleRegisters(uint8_t slaveAddress, ModbusRtuMaster::FunList function,
                                                                       uint16_t address, QVector<uint16_t> value, uint32_t timeoute)
{
    QByteArray commandBuff;
    QByteArray tempReadBuff;
    uint16_t crcCalc;
    uint16_t rxSize;

    /*
     * Clear input buffer
     */
    read(commandBuff);
    commandBuff.clear();

    /*
     * Serialiase request
     */
    commandBuff.push_back(slaveAddress);
    commandBuff.push_back(function);
    commandBuff.push_back(static_cast<uint8_t>(address >> 8));
    commandBuff.push_back(static_cast<uint8_t>(address));

    commandBuff.push_back(static_cast<uint8_t>(value.size() >> 8));
    commandBuff.push_back(static_cast<uint8_t>(value.size()));

    commandBuff.push_back(static_cast<uint8_t>(value.size() * 2));
    foreach(uint16_t data, value) {
        commandBuff.push_back(static_cast<uint8_t>(data >> 8));
        commandBuff.push_back(static_cast<uint8_t>(data));
    }
    crcCalc = crc(&commandBuff);
    commandBuff.push_back(static_cast<uint8_t>(crcCalc));
    commandBuff.push_back(static_cast<uint8_t>(crcCalc >> 8));

    /*
     * Send command
     */
    if (write(commandBuff) != true) {
        return MB_SEND_ERROR;
    };

    /*
     * The size of received data is static for the
     */
    rxSize = 8;

    /*
     * Receive reply
     */
    return receiveReply(&commandBuff, timeoute, rxSize, slaveAddress, function);
}
