#include "communication.h"

void Communication::connectSlaveSlot(std::function<void(bool result, int fwVersion,
                                                        int yearConf, int monthConf, int dayConf)> cb,
                                     QString port, int baudRate, int slaveAddress,
                                     SerialCommunication::SerialPortParity parity,
                                     SerialCommunication::SerialPortStopBits stopBits)
{
    /*
     * Open port
     */
    if(modbus->open(port, baudRate, parity, stopBits) == false) {
        if (cb != NULL) {
            cb(false, 0, 0, 0, 0);
        }
        return;
    }

    /*
     * Read Meta information
     */
    cb(true, 0, 0, 0, 0);
}

void Communication::disconnectSlaveSlot()
{
    modbus->close();
}

void Communication::writeConfigurationSlot(std::function<void(bool result)> cb, SlaveSettings settings)
{}

void Communication::readConfigurationSlot(std::function<void(bool result, SlaveSettings settings)> cb)
{}

void Communication::applySlot(std::function<void(bool result)> cb)
{}

void Communication::reloadSlot(std::function<void(bool result)> cb)
{}

void Communication::readStateSlot(std::function<void(bool result, SlaveState state)> cb)
{}

void Communication::setTeleControlSlot(std::function<void(bool result)> cb, int index, bool enable)
{}
