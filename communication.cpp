#include "communication.h"

void Communication::startCommunication(void)
{
    modbus = new ModbusRtuMaster();
}

void Communication::connectSlaveSlot(QString port, int baudRate,
                                     SerialCommunication::SerialPortParity parity,
                                     SerialCommunication::SerialPortStopBits stopBits)
{
    /*
     * Open port
     */
    if(modbus->open(port, baudRate, parity, stopBits) == false) {
        emit connectSlaveReply(false);
        return;
    }

    emit connectSlaveReply(true);
}

void Communication::disconnectSlaveSlot()
{
    modbus->close();
}

void Communication::writeConfigurationSlot(int slaveAddress, SlaveSettings settings)
{}

void Communication::readConfigurationSlot(int slaveAddress)
{
}

void Communication::readMetaInformationSlot(int slaveAddress)
{
    QVector<uint16_t> readData;
    ModbusRtuMaster::MbStatus result;

    result = modbus->readInputRegisters(slaveAddress, 1000, 2, readData);

    if (result != ModbusRtuMaster::MB_OK) {
        qDebug()<<"readMetaInformationSlot error:"<<modbus->getStatusString(result);
        emit readMetaInformationReply(false, 0, 0, 0, 0);
        return;
    }

    emit readMetaInformationReply(true, static_cast<int>(readData[0]), 0, 0, 0);
}

void Communication::applySlot(int slaveAddress)
{
    QVector<uint16_t> patload= {0x55FF};
    ModbusRtuMaster::MbStatus result;

    result = modbus->presetMultipleRegister(slaveAddress, 1011, patload);

    if (result == ModbusRtuMaster::MB_OK) {
        emit applyReply(true);
    } else {
        qDebug()<<"applySlot error:"<<modbus->getStatusString(result);
        emit applyReply(true);
    }
}

void Communication::reloadSlot(int slaveAddress)
{}

void Communication::readStateSlot(int slaveAddress)
{}

void Communication::setTeleControlSlot(int slaveAddress, int index, bool enable)
{}
