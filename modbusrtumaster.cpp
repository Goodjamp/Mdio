#include <modbusrtumaster.h>
#include <QtEndian>
#include <QTime>
#include <QThread>

#define UINT8_TO_UINT16(L, M)      UINT16_MAX & ((0x00FF & (L)) | (0xFF00 & ((M) << 8)));
#define MB_TRANSACTION_TIMEOUTE    500

// definition (no value needed)
const uint16_t ModbusRtuMaster::crcTable[256];

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
        rezCrc ^= this->crcTable[xorVal];
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
    commandBuff.push_back(static_cast<uint8_t>(crcCalc >> 8));
    commandBuff.push_back(static_cast<uint8_t>(crcCalc));

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
