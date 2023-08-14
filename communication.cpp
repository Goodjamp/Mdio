#include "communication.h"
#include "QVector"

void Communication::startCommunication(void)
{
    modbus = new ModbusRtuMaster();
}

void Communication::connectSlaveSlot(std::function<void(bool result)> cb,
                                     QString port, int baudRate,
                                     SerialCommunication::SerialPortParity parity,
                                     SerialCommunication::SerialPortStopBits stopBits)
{
    /*
     * Open port
     */
    if(modbus->open(port, baudRate, parity, stopBits) == false) {
        CALL_CB(cb, true);
        return;
    }
    CALL_CB(cb, true);
}

void Communication::disconnectSlaveSlot()
{
    modbus->close();
}

void Communication::writeConfigurationSlot(std::function<void(bool result)> cb,
                                           int slaveAddress, SlaveConfiguration configuration)
{
#define DEFAULT_KEY    0xFFFF
    QVector<uint16_t> configReg;
    ModbusRtuMaster::MbStatus result;
    bool resultWriteConfiguration = false;
    uint16_t baseConfReg;
    uint16_t registersNumbers;
    uint16_t parity;
    uint16_t stopBits;

    if ((parity = parityMbToSerialLUT.key(configuration.communication.parity, DEFAULT_KEY)) != DEFAULT_KEY
        && (stopBits = stopBitsMbToSerialLUT.key(configuration.communication.stopBits, DEFAULT_KEY)) != DEFAULT_KEY) {
        /*
         * Serialiase configuration to registers list
         */
        baseConfReg = ADDRESS_DATE_CONFIGURATION;
        registersNumbers = ADDRESS_TC_PULS_DURATION - baseConfReg + 1;
        configReg.resize(registersNumbers);

        /*
         * Serialiase date of configuration
         */
        configReg[ADDRESS_DATE_CONFIGURATION - baseConfReg] |= (DAY_CON_MASK & configuration.day) << DAY_CON_POS;
        configReg[ADDRESS_DATE_CONFIGURATION - baseConfReg] |= (MONTH_CON_MASK & configuration.month) << MONTH_CON_POS;
        configReg[ADDRESS_DATE_CONFIGURATION - baseConfReg] |= (YEAR_CON_MASK & configuration.year) << YEAR_CON_POS;

        /*
         * Serialiase communicaiotn settings
         */
        configReg[ADDRESS_COMMUNICATION_BAUDRATE - baseConfReg] = configuration.communication.baudRate;
        configReg[ADDRESS_COMMUNICATION_SETTINGS - baseConfReg] = 0;
        configReg[ADDRESS_COMMUNICATION_SETTINGS - baseConfReg] |= parity << PARITY_POS;
        configReg[ADDRESS_COMMUNICATION_SETTINGS - baseConfReg] |= stopBits << STOP_BITS_POS;
        configReg[ADDRESS_COMMUNICATION_SILENTS_INTERVAL - baseConfReg] = configuration.communication.silentInterval;
        configReg[ADDRESS_COMMUNICATION_REPLY_DELAY - baseConfReg] = configuration.communication.replyDelay;

        /*
         * Serialiase tele signalisation settings
         */
        configReg[ADDRESS_TS_DEBOUNCE_DELAY - baseConfReg] = configuration.signalisation.debounsInterval;
        configReg[ADDRESS_TS_INVERSION_SETTINGS - baseConfReg] = 0;
        for (uint32_t k = 0; k < TELESIGNAL_NUMBERS; k++) {
            if (configuration.signalisation.isInvers[k] == true) {
                configReg[ADDRESS_TS_INVERSION_SETTINGS - baseConfReg] |= static_cast<uint16_t>(1) << k;
            }
        }

        /*
         * Serialiase tele control settings
         */
        configReg[ADDRESS_TC_PULS_DURATION - baseConfReg] = configuration.control.pulsDuration;

        result = modbus->presetMultipleRegister(slaveAddress, baseConfReg, configReg);
        if (result == ModbusRtuMaster::MB_OK) {
            resultWriteConfiguration = true;
        } else {
           qDebug()<<"writeConfigurationSlot write configuration error:"<<modbus->getStatusString(result);
        }
    } else {
        qDebug()<<"writeConfigurationSlot paity or stopBits error";
    }

    CALL_CB(cb, resultWriteConfiguration);
}

void Communication::readConfigurationSlot(std::function<void(bool result, SlaveConfiguration settings)> cb,
                                          int slaveAddress)
{
    QVector<uint16_t> configReg;
    ModbusRtuMaster::MbStatus result;
    SlaveConfiguration configuration;
    uint16_t baseConfReg;
    uint16_t registersNumbers;
    int parity;
    int stopBits;
    bool resulReadConfiguration = false;

    /*
     * Read reagisters range from the ADDRESS_COMMUNICATION_BAUDRATE to the ADDRESS_TC_PULS_DURATION
     */
    baseConfReg = ADDRESS_DATE_CONFIGURATION;
    registersNumbers = ADDRESS_TC_PULS_DURATION - baseConfReg + 1;
    result = modbus->readHoldingRegisters(slaveAddress, baseConfReg, registersNumbers, configReg);
    if (result == ModbusRtuMaster::MB_OK) {
        /*
         * Deserialiase configuration date
         */
        configuration.day = DAY_CON_MASK & (configReg[baseConfReg - ADDRESS_DATE_CONFIGURATION] >> DAY_CON_POS);
        configuration.month = MONTH_CON_MASK & (configReg[baseConfReg - ADDRESS_DATE_CONFIGURATION] >> MONTH_CON_POS);
        configuration.year = YEAR_CON_MASK & (configReg[baseConfReg - ADDRESS_DATE_CONFIGURATION] >> YEAR_CON_POS);

        /*
         * Deserialiase configuration
         */
        parity = static_cast<int>(PARITY_MASK & (configReg[ADDRESS_COMMUNICATION_SETTINGS - baseConfReg] >> PARITY_POS));
        stopBits =  static_cast<int>(STOP_BITS_MASK & (configReg[ADDRESS_COMMUNICATION_SETTINGS - baseConfReg] >> STOP_BITS_POS));
        if (parityMbToSerialLUT.contains(parity) == true
            && stopBitsMbToSerialLUT.contains(stopBits) == true) {
            /*
             * Deserialiase communication settings
             */
            configuration.communication.parity = parityMbToSerialLUT.value(parity);
            configuration.communication.stopBits = stopBitsMbToSerialLUT.value(stopBits);
            configuration.communication.baudRate = configReg[ADDRESS_COMMUNICATION_BAUDRATE - baseConfReg];
            configuration.communication.silentInterval = configReg[ADDRESS_COMMUNICATION_SILENTS_INTERVAL - baseConfReg];
            configuration.communication.replyDelay = configReg[ADDRESS_COMMUNICATION_REPLY_DELAY - baseConfReg];

            /*
             * Deserialiase tele signalisation settings
             */
            configuration.signalisation.debounsInterval = configReg[ADDRESS_TS_DEBOUNCE_DELAY - baseConfReg];
            for (uint32_t k = 0; k < TELESIGNAL_NUMBERS; k++) {
                configuration.signalisation.isInvers[k] = (1 & (configReg[ADDRESS_TS_INVERSION_SETTINGS - baseConfReg] >> k)) == 1;
            }

            /*
             * Deserialiase tele control settings
             */
            configuration.control.pulsDuration = configReg[ADDRESS_TC_PULS_DURATION - baseConfReg];
            resulReadConfiguration = true;
        } else {
            qDebug()<<"readConfigurationSlot parity or stop bits error:";
        }
    } else {
        qDebug()<<"readConfigurationSlot read configuration error:"<<modbus->getStatusString(result);
    }

    CALL_CB(cb, resulReadConfiguration, configuration);
}

void Communication::readMetaInformationSlot(std::function<void(bool result, int fwVersion)> cb,
                                            int slaveAddress)
{
    QVector<uint16_t> readData;
    ModbusRtuMaster::MbStatus result;

    result = modbus->readHoldingRegisters(slaveAddress, ADDRESS_VERSION_FW, 1, readData);

    if (result != ModbusRtuMaster::MB_OK) {
        qDebug()<<"readMetaInformationSlot error:"<<modbus->getStatusString(result);
        CALL_CB(cb, false, 0);
        return;
    }

    CALL_CB(cb, true, static_cast<int>(readData[0]));
}

void Communication::reloadSlot(std::function<void(bool result)> cb,
                int slaveAddress)
{
    QVector<uint16_t> registersList= {RESET_MAGIC_NUMBER};
    ModbusRtuMaster::MbStatus result;

    result = modbus->presetMultipleRegister(slaveAddress, ADDRESS_RESET, registersList);

    if (result == ModbusRtuMaster::MB_OK) {
        CALL_CB(cb, true);
    } else {
        qDebug()<<"reloadSlot error:"<<modbus->getStatusString(result);
        CALL_CB(cb, false);
    }
}

void Communication::readStateSlot(int slaveAddress)
{
    QVector<bool> teleSignal;
    QVector<uint16_t> teleControl;
    QVector<uint16_t> status;
    ModbusRtuMaster::MbStatus result;
    SlaveState state;

    /*
     * Read global status
     */
    result = modbus->readInputRegisters(slaveAddress, ADDRESS_GLOBAL_STATUS, 1, status);
    if (result != ModbusRtuMaster::MB_OK) {
        emit readStateReply(false, state);
        qDebug()<<"readStateSlot read globalStatusReg error:"<<modbus->getStatusString(result);
        return;
    }
    state.errorEeprom = (status[0] >> STATUS_EEPROM_ERROR_POS & 1) == 1;
    state.error220 = (status[0] >> STATUS_220_POS & 1) == 1;
    state.errorTransaction = (status[0] >> STATUS_TRANSACTION_ERROR_POS & 1) == 1;

    /*
     * According to the documentation, if STATUS_220 is set, the device replay with
     * exception. So, in this case we can skip reading the tele-signal information
     */
    if (state.errorEeprom == true) {
        /*
         * Read tele signal status
         */
        result = modbus->readDiscreteInputs(slaveAddress, 1, TELESIGNAL_NUMBERS, teleSignal);
        if (result != ModbusRtuMaster::MB_OK) {
            emit readStateReply(false, state);
            qDebug()<<"readStateSlot read read signals error:"<<modbus->getStatusString(result);
            return;
        }
        for (uint32_t k = 0; k < TELESIGNAL_NUMBERS; k++) {
            state.signalisation[k] = teleSignal[k];
        }
    }

    /*
     * Read tele control state
     */
    result = modbus->readInputRegisters(slaveAddress, ADDRESS_TELE_CONTROL_BASE, TELECONTROL_TOTAL_NUMBERS, teleControl);
    if (result != ModbusRtuMaster::MB_OK) {
        emit readStateReply(false, state);
        qDebug()<<"readStateSlot read tele control error:"<<modbus->getStatusString(result);
        return;
    }

    /*
     * Test and apply the context of telecontrol registers
     */
    if (teleControl[ADDRESS_TELE_CONTROL_BASE - ADDRESS_TELE_CONTROL_1] == TELECONTROL_PULS_ON) {
        teleControl[ADDRESS_TELE_CONTROL_BASE - ADDRESS_TELE_CONTROL_1] = true;
    } else if (teleControl[ADDRESS_TELE_CONTROL_BASE - ADDRESS_TELE_CONTROL_1] == TELECONTROL_PULS_OFF) {
        teleControl[ADDRESS_TELE_CONTROL_BASE - ADDRESS_TELE_CONTROL_1] = false;
    } else {
        qDebug()<<"readStateSlot puls telecontrol value error";
        emit readStateReply(false, state);
        return;
    }
    for (uint32_t k = 0; k < TELECONTROL_NUMBERS; k++) {
        if (teleControl[ADDRESS_TELE_CONTROL_BASE - ADDRESS_TELE_CONTROL_2 + k]
            == ModbusRtuMaster::COIL_ON) {
            teleControl[ADDRESS_TELE_CONTROL_BASE - ADDRESS_TELE_CONTROL_2 + k] = true;
        } else if (teleControl[ADDRESS_TELE_CONTROL_BASE - ADDRESS_TELE_CONTROL_2 + k]
                   == ModbusRtuMaster::COIL_OFF) {
            teleControl[ADDRESS_TELE_CONTROL_BASE - ADDRESS_TELE_CONTROL_2 + k] = false;
        } else {
            qDebug()<<"readStateSlot telecontrol value error";
            emit readStateReply(false, state);
            return;
        }
    }
    emit readStateReply(true, state);
}

void Communication::setTeleControlSlot(int slaveAddress, int index, bool enable)
{
ModbusRtuMaster::MbStatus result;

    if (index > TELECONTROL_NUMBERS) {
        qDebug()<<"setTeleControlSlot index value error: "<<index;
        emit setTeleControlReply(false);
        return;
    }
    result = modbus->forceSingleCoil(slaveAddress, ADDRESS_TELE_CONTROL_2 + index, enable);
    if (result != ModbusRtuMaster::MB_OK) {
        qDebug()<<"setTeleControlSlot send error:"<<modbus->getStatusString(result);
    }
    emit setTeleControlReply(result == ModbusRtuMaster::MB_OK);
}

void Communication::setTeleControlPulsSlot(int slaveAddress, bool enable)
{
    ModbusRtuMaster::MbStatus result;

    result = modbus->presetSingleRegister(slaveAddress,ADDRESS_TELE_CONTROL_1,
                                          enable == true
                                          ? static_cast<uint16_t>(TELECONTROL_PULS_ON)
                                          : static_cast<uint16_t>(TELECONTROL_PULS_OFF));
    if (result != ModbusRtuMaster::MB_OK) {
        qDebug()<<"setTeleControlPulsSlot send error:"<<modbus->getStatusString(result);
    }
    emit setTeleControlReply(result == ModbusRtuMaster::MB_OK);
}
