#include <modbusrtumaster.h>
#include <QtEndian>
#include <QTime>

// definition (no value needed)
const uint16_t ModbusRtuMaster::crcTable[256];

ModbusRtuMaster::ModbusRtuMaster(QObject *parent) : SerialCommunication(parent)
{

}

uint16_t ModbusRtuMaster::crc(QByteArray buff)
{
    uint8_t xorVal = 0;
    uint16_t rezCrc = 0xFFFF;

    for (int k = 0; k <  buff.size(); k++)
    {
        xorVal = buff[k] ^ rezCrc;
        rezCrc >>= 8;
        rezCrc ^= this->crcTable[xorVal];
    }

    return rezCrc;
}

template <typename T>
ModbusRtuMaster::MbStatus ModbusRtuMaster::read(uint8_t slaveAddress, ModbusRtuMaster::FunList function,
                                                uint16_t address, uint16_t number, QVector<T> *state,
                                                uint32_t timeoute)
{
    QByteArray commandBuff;
    QByteArray tempReadBuff;
    uint16_t crcRx;
    uint16_t crcCalc;
    uint8_t payloadBytes;
    uint16_t rxSize;

    /*
     * Clear input buffer
     */
    readPort(&commandBuff);
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
    crcCalc = crc(commandBuff);
    commandBuff.push_back(static_cast<uint8_t>(crcCalc >> 8));
    commandBuff.push_back(static_cast<uint8_t>(crcCalc));

    /*
     * Send command
     */
    if (writePort(commandBuff) != true) {
        return MB_SEND_ERROR;
    };
    commandBuff.clear();

    /*
     * Calculation of the reply payload size and the total size of the reply
     */
    switch (function) {
    case READ_COIL_STATUS:
    case READ_DISCRET_INPUTS:
        payloadBytes = number/8 + number % 8;
        break;

    case READ_HOLDING_REGISTERS:
    case READ_INPUT_REGISTERS:
        payloadBytes = number * 2;
        break;

    default:
        return MB_TARGET_FUNCTION_ERROR;
    }
    rxSize = MB_READ_MINIMUM_COMMAND_SIZE + payloadBytes;

    QTime endReceiveTime = QTime::currentTime().addMSecs(timeoute);

     while(QTime::currentTime().msecsTo(endReceiveTime) > 0) {
        readPort(&tempReadBuff);
        if (tempReadBuff.size() != 0){
            commandBuff.append(tempReadBuff);
            if (commandBuff.size() == rxSize) {
                break;
            }
        }
     }
     if (commandBuff.size() != rxSize) {
         return MB_RX_SIZE_ERROR;
     }
     crcRx = UINT16_MAX & (commandBuff[rxSize - 1] | commandBuff[rxSize - 1] << 8);
     crcCalc = crc(commandBuff);
     if (crcCalc == crcRx) {
         return MB_CRC_ERROR;
     }
     if (commandBuff[MODBUS_SLAVE_ADDRESS_POS] != static_cast<char>(slaveAddress)) {
         return MB_ADDRESS_ERROR;
     }
     if (commandBuff[MODBUS_FUNCTION_CODE_POS] != static_cast<char>(function)) {
         return MB_FUNCTION_ERROR;
     }
     if (commandBuff[MODBUS_RX_BYTES_NUMBER_POS] != static_cast<char>(payloadBytes)) {
         return MB_BYTES_NUMBER_ERROR;
     }

     switch (function) {
     case READ_COIL_STATUS:
     case READ_DISCRET_INPUTS: {
         uint16_t rest;
         int payloadPos = MODBUS_RX_BITS_PAYLOAD_POS;

         /*
          * Deserialiaze the binary information type
          */
         while (number) {
             rest = (number > 8) ? 8 : number;
             for (uint32_t k = 0; k < rest; k++) {
                 state->push_back(commandBuff[payloadPos] & 0x1 ? true : false);
                 commandBuff[payloadPos] = commandBuff[payloadPos] >> 1;
             }
             payloadPos++;
             number -= rest;
         }
         break;
     }

     case READ_HOLDING_REGISTERS:
     case READ_INPUT_REGISTERS: {
         int payloadPos = MODBUS_RX_BITS_PAYLOAD_POS;
         uint16_t data = 0;

         /*
          * Deserialiaze the bytes information.
          * Convert Big endians to Little endians
          */
         for (uint32_t k = 0; k < number; k++) {
             data = (0xFF00 & (commandBuff[payloadPos++] << 8));
             data |= (0xFF & commandBuff[payloadPos++]);
             state->push_back(data);
         }
         break;
     }

     default:
         return MB_TARGET_FUNCTION_ERROR;
     }

     return MB_OK;
}

ModbusRtuMaster::MbStatus ModbusRtuMaster::writeSingleRegister(uint8_t slaveAddress, ModbusRtuMaster::FunList function,
                             uint16_t address, uint16_t value, uint32_t timeoute)
{
    QByteArray commandBuff;
    QByteArray replyBuff;
    QByteArray tempReadBuff;
    uint16_t crcCalc;
    /*
     * Clear input buffer
     */
    readPort(&commandBuff);
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
    crcCalc = crc(commandBuff);
    commandBuff.push_back(static_cast<uint8_t>(crcCalc >> 8));
    commandBuff.push_back(static_cast<uint8_t>(crcCalc));

    /*
     * Send command
     */
    if (writePort(commandBuff) != true) {
        return MB_SEND_ERROR;
    };

    /*
     * Receive reply
     */

    QTime endReceiveTime = QTime::currentTime().addMSecs(timeoute);
    while(QTime::currentTime().msecsTo(endReceiveTime) > 0) {
       readPort(&tempReadBuff);
       if (tempReadBuff.size() != 0){
           replyBuff.append(tempReadBuff);
           if (replyBuff.size() == commandBuff.size()) {
               break;
           }
       }
    }
    if (commandBuff.size() != replyBuff.size()) {
        return MB_RX_SIZE_ERROR;
    }

    if (commandBuff != replyBuff) {
        return MB_REPLY_ERROR;
    }

    return MB_OK;
}

ModbusRtuMaster::MbStatus ModbusRtuMaster::writeMultipleRegisters(uint8_t slaveAddress, ModbusRtuMaster::FunList function,
                               uint16_t address, QVector<uint16_t> value, uint32_t timeoute)
{
    QByteArray commandBuff;
    QByteArray tempReadBuff;
    uint16_t crcRx;
    uint16_t crcCalc;
    uint16_t rxSize;
    /*
     * Clear input buffer
     */
    readPort(&commandBuff);
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
    crcCalc = crc(commandBuff);
    commandBuff.push_back(static_cast<uint8_t>(crcCalc >> 8));
    commandBuff.push_back(static_cast<uint8_t>(crcCalc));

    /*
     * Send command
     */
    if (writePort(commandBuff) != true) {
        return MB_SEND_ERROR;
    };

    /*
     * Receive reply
     */
    rxSize = 8;
    commandBuff.clear();
    QTime endReceiveTime = QTime::currentTime().addMSecs(timeoute);
    while(QTime::currentTime().msecsTo(endReceiveTime) > 0) {
       readPort(&tempReadBuff);
       if (tempReadBuff.size() != 0){
           commandBuff.append(tempReadBuff);
           if (commandBuff.size() == rxSize) {
               break;
           }
       }
    }
    if (commandBuff.size() != rxSize) {
        return MB_RX_SIZE_ERROR;
    }
    crcRx = UINT16_MAX & (commandBuff[rxSize - 1] | commandBuff[rxSize - 1] << 8);
    crcCalc = crc(commandBuff);
    if (crcCalc == crcRx) {
        return MB_CRC_ERROR;
    }
    if (commandBuff[MODBUS_SLAVE_ADDRESS_POS] != static_cast<char>(slaveAddress)) {
        return MB_ADDRESS_ERROR;
    }
    if (commandBuff[MODBUS_FUNCTION_CODE_POS] != static_cast<char>(function)) {
        return MB_FUNCTION_ERROR;
    }

    return MB_OK;
}
