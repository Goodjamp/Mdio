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

void ModbusRtuMaster::readCoilStatus(uint8_t slaveAddress, uint16_t coilAddress, uint16_t coilsNumber, QByteArray *coilState, uint32_t timeoute)
{
    QByteArray commandBuff;
    QByteArray tempReadBuff;
    uint16_t crcRx;
    uint16_t crcCalc;

    /*
     * Clear input buffer
     */
    readPort(&commandBuff);
    commandBuff.clear();

    /*
     * Serialiase
     */
    commandBuff.push_back(slaveAddress);
    commandBuff.push_back(READ_COIL_STATUS);
    commandBuff.push_back(static_cast<uint8_t>(coilAddress >> 8));
    commandBuff.push_back(static_cast<uint8_t>(coilAddress));
    commandBuff.push_back(static_cast<uint8_t>(coilsNumber >> 8));
    commandBuff.push_back(static_cast<uint8_t>(coilsNumber));
    crcCalc = crc(commandBuff);
    commandBuff.push_back(static_cast<uint8_t>(crcCalc >> 8));
    commandBuff.push_back(static_cast<uint8_t>(crcCalc));

    /*
     * Send command
     */
    writePort(commandBuff);
    commandBuff.clear();

    /*
     * The number of receive data
     */
     uint8_t payloadBytes = coilsNumber/8 + coilsNumber % 8;
     uint16_t rxSize = 5 + payloadBytes;
     QTime endReceiveTime;
     endReceiveTime = QTime::currentTime().addMSecs(timeoute);

     while(QTime::currentTime().msecsTo(endReceiveTime) > 0) {
        readPort(&tempReadBuff);
        if (tempReadBuff.size() != 0){
            commandBuff.append(tempReadBuff);
            if (commandBuff.size() == rxSize) {
                break;
            }
        }
     }

     if (commandBuff.size() == rxSize) {
         crcRx = UINT16_MAX & (commandBuff[rxSize - 1] | commandBuff[rxSize - 1] << 8);
         commandBuff.remove(commandBuff.size() - 1, 2);
         crcCalc = crcCalc = crc(commandBuff);
         if (commandBuff[0] == static_cast<char>(slaveAddress)
             && commandBuff[1] == static_cast<char>(READ_COIL_STATUS)
             && commandBuff[2] == static_cast<char>(payloadBytes)
             && crcCalc == crcRx) {
             coilState->clear();
             for (uint32_t k = 3; k < payloadBytes; k++) {

             }

         }

     } else { // timeoute ocured

     }
}
