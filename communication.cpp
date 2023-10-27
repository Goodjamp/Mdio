#include "communication.h"
#include "QVector"

void Communication::startCommunication(void)
{
    modbus = new ModbusRtuMaster();
}

void Communication::connectSlaveSlot(std::function<void(bool result)> cb,
                                     QString port, int baudRate,
                                     SerialCommunication::SerialPortParity parity,
                                     SerialCommunication::SerialPortStopBits stopBits,
                                     int readReplyTimeoute)
{
    /*
     * Open port
     */
    if(modbus->open(port, baudRate, parity, stopBits) == false) {
        CALL_CB(cb, true);
        return;
    }
    modbus->setReadReplyTimeout(readReplyTimeoute);
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
        baseConfReg = ADDR_REG_DATE_CONFIGURATION;
        registersNumbers = ADDR_REG_TC_PULS_DURATION - baseConfReg + 1;
        configReg.resize(registersNumbers);

        /*
         * Serialiase date of configuration
         */
        configReg[ADDR_REG_DATE_CONFIGURATION - baseConfReg] |= (DAY_CON_MASK & configuration.configurationDay) << DAY_CON_POS;
        configReg[ADDR_REG_DATE_CONFIGURATION - baseConfReg] |= (MONTH_CON_MASK & configuration.configurationMonth) << MONTH_CON_POS;
        configReg[ADDR_REG_DATE_CONFIGURATION - baseConfReg] |= (YEAR_CON_MASK & configuration.configurationYear) << YEAR_CON_POS;

        /*
         * Serialiase communicaiotn settings
         */
        configReg[ADDR_REG_COMMUNICATION_BAUDRATE - baseConfReg] = configuration.communication.baudRate;
        configReg[ADDR_REG_COMMUNICATION_SETTINGS - baseConfReg] = 0;
        configReg[ADDR_REG_COMMUNICATION_SETTINGS - baseConfReg] |= parity << PARITY_POS;
        configReg[ADDR_REG_COMMUNICATION_SETTINGS - baseConfReg] |= stopBits << STOP_BITS_POS;
        configReg[ADDR_REG_COMMUNICATION_SILENTS_INTERVAL - baseConfReg] = configuration.communication.silentInterval;
        configReg[ADDR_REG_COMMUNICATION_REPLY_DELAY - baseConfReg] = configuration.communication.replyDelay;

        /*
         * Serialiase tele signalisation settings
         */
        configReg[ADDR_REG_TS_DEBOUNCE_DELAY - baseConfReg] = configuration.signalisation.debounsInterval;
        configReg[ADDR_REG_TS_INVERSION_SETTINGS - baseConfReg] = 0;
        for (uint32_t k = 0; k < TELESIGNAL_NUMBERS; k++) {
            if (configuration.signalisation.isInvers[k] == false) {
                configReg[ADDR_REG_TS_INVERSION_SETTINGS - baseConfReg] |= static_cast<uint16_t>(1) << k;
            }
        }
        configReg[ADDR_REG_TS_DOUBLE_SWITCHING_TIME - baseConfReg] = configuration.signalisation.doubleTsSwitchingTime;
        configReg[ADDR_REG_TS_DOUBLE_SWITCHING_SETTINGS - baseConfReg] = 0;
        for (uint32_t k = 0; k < TELESIGNAL_DOUBLE_NUMBERS; k++) {
            if (configuration.signalisation.isDouble[k] == true) {
                configReg[ADDR_REG_TS_DOUBLE_SWITCHING_SETTINGS - baseConfReg] |= static_cast<uint16_t>(1) << k;
            }
        }

        /*
         * Serialiase tele control settings
         */
        configReg[ADDR_REG_TC_PULS_DURATION - baseConfReg] = configuration.control.pulsDuration;

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
    baseConfReg = ADDR_REG_DATE_CONFIGURATION;
    registersNumbers = ADDR_REG_TC_PULS_DURATION - baseConfReg + 1;
    result = modbus->readHoldingRegisters(slaveAddress, baseConfReg, registersNumbers, configReg);
    if (result == ModbusRtuMaster::MB_OK) {
        /*
         * Deserialiase configuration date
         */
        configuration.configurationDay = DAY_CON_MASK & (configReg[ADDR_REG_DATE_CONFIGURATION - baseConfReg] >> DAY_CON_POS);
        configuration.configurationMonth = MONTH_CON_MASK & (configReg[ADDR_REG_DATE_CONFIGURATION - baseConfReg] >> MONTH_CON_POS);
        configuration.configurationYear = YEAR_CON_MASK & (configReg[ADDR_REG_DATE_CONFIGURATION - baseConfReg] >> YEAR_CON_POS);

        /*
         * Deserialiase configuration
         */
        stopBits =  static_cast<int>(STOP_BITS_MASK & (configReg[ADDR_REG_COMMUNICATION_SETTINGS - baseConfReg] >> STOP_BITS_POS));
        parity = static_cast<int>(PARITY_MASK & (configReg[ADDR_REG_COMMUNICATION_SETTINGS - baseConfReg] >> PARITY_POS));
        if (parityMbToSerialLUT.contains(parity) == true
            && stopBitsMbToSerialLUT.contains(stopBits) == true) {
            /*
             * Deserialiase communication settings
             */
            configuration.communication.parity = parityMbToSerialLUT.value(parity);
            configuration.communication.stopBits = stopBitsMbToSerialLUT.value(stopBits);
            configuration.communication.baudRate = configReg[ADDR_REG_COMMUNICATION_BAUDRATE - baseConfReg];
            configuration.communication.silentInterval = configReg[ADDR_REG_COMMUNICATION_SILENTS_INTERVAL - baseConfReg];
            configuration.communication.replyDelay = configReg[ADDR_REG_COMMUNICATION_REPLY_DELAY - baseConfReg];

            /*
             * Deserialiase tele signalisation settings
             */
            configuration.signalisation.debounsInterval = configReg[ADDR_REG_TS_DEBOUNCE_DELAY - baseConfReg];
            for (uint32_t k = 0; k < TELESIGNAL_NUMBERS; k++) {
                configuration.signalisation.isInvers[k] = (1 & (configReg[ADDR_REG_TS_INVERSION_SETTINGS - baseConfReg] >> k)) == 0;
            }
            configuration.signalisation.doubleTsSwitchingTime = configReg[ADDR_REG_TS_DOUBLE_SWITCHING_TIME - baseConfReg];
            for (uint32_t k = 0; k < TELESIGNAL_DOUBLE_NUMBERS; k++) {
                configuration.signalisation.isDouble[k] = (1 & (configReg[ADDR_REG_TS_DOUBLE_SWITCHING_SETTINGS - baseConfReg] >> k)) == 1;
            }

            /*
             * Deserialiase tele control settings
             */
            configuration.control.pulsDuration = configReg[ADDR_REG_TC_PULS_DURATION - baseConfReg];
            resulReadConfiguration = true;
        } else {
            qDebug()<<"readConfigurationSlot parity or stop bits error:";
        }
    } else {
        qDebug()<<"readConfigurationSlot read configuration error:"<<modbus->getStatusString(result);
    }

    CALL_CB(cb, resulReadConfiguration, configuration);
}

void Communication::readMetaInformationSlot(std::function<void(bool result, MetaInformation metaInformation)> cb,
                                            int slaveAddress)
{
    QVector<uint16_t> readData;
    ModbusRtuMaster::MbStatus result;
    MetaInformation metaInformation;
    uint16_t baseReg;
    uint16_t registersNumbers;

    /*
     * Read reagisters range from the ADDR_REG_VERSION_FW to the ADDR_REG_DATE_CONFIGURATION
     */
    baseReg = ADDR_REG_VERSION_FW;
    registersNumbers = ADDR_REG_DATE_CONFIGURATION - baseReg + 1;

    result = modbus->readHoldingRegisters(slaveAddress, baseReg, registersNumbers, readData);

    if (result != ModbusRtuMaster::MB_OK) {
        qDebug()<<"readMetaInformationSlot error:"<<modbus->getStatusString(result);
        CALL_CB(cb, false, metaInformation);
        return;
    }
    metaInformation.fwVersion = static_cast<int>(readData[0]);
    metaInformation.configurationDay = DAY_CON_MASK & (readData[ADDR_REG_DATE_CONFIGURATION - baseReg] >> DAY_CON_POS);
    metaInformation.configurationMonth = MONTH_CON_MASK & (readData[ADDR_REG_DATE_CONFIGURATION - baseReg] >> MONTH_CON_POS);
    metaInformation.configurationYear = YEAR_CON_MASK & (readData[ADDR_REG_DATE_CONFIGURATION - baseReg] >> YEAR_CON_POS);

    CALL_CB(cb, true, metaInformation);
}

void Communication::reloadSlot(std::function<void(bool result)> cb,
                int slaveAddress)
{
    QVector<uint16_t> registersList= {RESET_MAGIC_NUMBER};
    ModbusRtuMaster::MbStatus result;

    result = modbus->presetMultipleRegister(slaveAddress, ADDR_REG_RESET, registersList);

    if (result == ModbusRtuMaster::MB_OK) {
        CALL_CB(cb, true);
    } else {
        qDebug()<<"reloadSlot error:"<<modbus->getStatusString(result);
        CALL_CB(cb, false);
    }
}

void Communication::readStateSlot(std::function<void(bool result, SlaveState state)> cb,
                                  int slaveAddress)
{
    QVector<bool> teleSignal;
    QVector<bool> status;
    QVector<uint16_t> teleControl;
    ModbusRtuMaster::MbStatus result;
    SlaveState state;
    uint16_t baseCoilAddress;
    uint16_t readCoilsNumber;
    uint16_t baseTcAddress;

    /*
     * Read global status
     */
    baseCoilAddress = ADDR_COIL_220_V_ERROR;
    readCoilsNumber = ADDR_COIL_EEPROM_CLEAR_ERROR - baseCoilAddress + 1;
    result = modbus->readDiscreteInputs(slaveAddress, baseCoilAddress, readCoilsNumber, status);
    if (result != ModbusRtuMaster::MB_OK) {
        CALL_CB(cb, false, state);
        qDebug()<<"readStateSlot read globalStatusReg error:"<<modbus->getStatusString(result);
        return;
    }
    state.error220 =  status[ADDR_COIL_220_V_ERROR - baseCoilAddress];
    state.errorEeprom = status[ADDR_COIL_EEPROM_ERROR - baseCoilAddress];
    state.errorConfiguration =  status[ADDR_COIL_CONFIGURATION_ERROR - baseCoilAddress];
    state.errorEepromClear =  status[ADDR_COIL_EEPROM_CLEAR_ERROR - baseCoilAddress];

    /*
     * According to the documentation, if STATUS_220 is set, the device replay with
     * exception. So, in this case we can skip reading the tele-signal information
     */
    if (state.error220 == false) {
        /*
         * Read tele signal status
         */
        baseCoilAddress = ADDR_COIL_TC_1;
        readCoilsNumber = ADDR_COIL_TC_4 - baseCoilAddress + 1;
        result = modbus->readDiscreteInputs(slaveAddress, baseCoilAddress, readCoilsNumber, teleSignal);
        if (result != ModbusRtuMaster::MB_OK) {
            CALL_CB(cb, false, state);
            qDebug()<<"readStateSlot read read signals error:"<<modbus->getStatusString(result);
            return;
        }
        for (uint32_t k = 0; k < TELESIGNAL_NUMBERS; k++) {
            state.signalisation[k] = teleSignal[k];
        }
    } else {
        for (uint32_t k = 0; k < TELESIGNAL_NUMBERS; k++) {
            state.signalisation[k] = false;
        }
    }

    // CALL_CB(cb, true, state);
    // return;

    /*
     * Read tele control state.
     */
    baseTcAddress = ADDR_REG_TELE_CONTROL_1;
    result = modbus->readInputRegisters(slaveAddress, baseTcAddress, TELECONTROL_TOTAL_NUMBERS, teleControl);
    if (result != ModbusRtuMaster::MB_OK) {
        CALL_CB(cb, false, state);
        qDebug()<<"readStateSlot read tele control error:"<<modbus->getStatusString(result);
        return;
    }

    /*
     * Test and apply the context of telecontrol registers
     */
    if (teleControl[ADDR_REG_TELE_CONTROL_1 - baseTcAddress] == TELECONTROL_PULS_ON) {
        state.control[ADDR_REG_TELE_CONTROL_1 - baseTcAddress] = true;
    } else if (teleControl[ADDR_REG_TELE_CONTROL_1 - baseTcAddress] == TELECONTROL_PULS_OFF) {
        state.control[ADDR_REG_TELE_CONTROL_1 - baseTcAddress] = false;
    } else if (teleControl[ADDR_REG_TELE_CONTROL_1 - baseTcAddress] == TELECONTROL_UNDEFINED) {
        state.control[ADDR_REG_TELE_CONTROL_1 - baseTcAddress] = false;
    } else {
        qDebug()<<"readStateSlot puls telecontrol value error";
        CALL_CB(cb, false, state);
        return;
    }
    for (uint32_t k = 0; k < TELECONTRO_STATIC_NUMBERS; k++) {
        if (teleControl[ADDR_REG_TELE_CONTROL_2 - baseTcAddress + k]
            == ModbusRtuMaster::COIL_ON) {
            state.control[ADDR_REG_TELE_CONTROL_2 - baseTcAddress + k] = true;
        } else if (teleControl[ADDR_REG_TELE_CONTROL_2 - baseTcAddress + k]
                   == ModbusRtuMaster::COIL_OFF) {
            state.control[ADDR_REG_TELE_CONTROL_2 - baseTcAddress + k] = false;
        } else if (teleControl[ADDR_REG_TELE_CONTROL_2 - baseTcAddress + k]
                   == TELECONTROL_UNDEFINED) {
            state.control[ADDR_REG_TELE_CONTROL_2 - baseTcAddress + k] = false;
        } else {
            qDebug()<<"readStateSlot telecontrol value error";
            CALL_CB(cb, false, state);
            return;
        }
    }
    CALL_CB(cb, true, state);
}

void Communication::setTeleControlSlot(std::function<void(bool result)> cb,
                                       int slaveAddress, int index, bool enable)
{
ModbusRtuMaster::MbStatus result;

    if (index > TELECONTRO_STATIC_NUMBERS) {
        qDebug()<<"setTeleControlSlot index value error: "<<index;
        CALL_CB(cb, false);
        return;
    }
    result = modbus->forceSingleCoil(slaveAddress, ADDR_REG_TELE_CONTROL_2 + index, enable);
    if (result != ModbusRtuMaster::MB_OK) {
        qDebug()<<"setTeleControlSlot send error:"<<modbus->getStatusString(result);
    }
    CALL_CB(cb, result == ModbusRtuMaster::MB_OK);
}

void Communication::setTeleControlPulsSlot(std::function<void(bool result)> cb,
                                           int slaveAddress, bool enable)
{
    ModbusRtuMaster::MbStatus result;

    result = modbus->presetSingleRegister(slaveAddress, ADDR_REG_TELE_CONTROL_1,
                                          enable == true
                                          ? static_cast<uint16_t>(TELECONTROL_PULS_ON)
                                          : static_cast<uint16_t>(TELECONTROL_PULS_OFF));
    if (result != ModbusRtuMaster::MB_OK) {
        qDebug()<<"setTeleControlPulsSlot send error:"<<modbus->getStatusString(result);
    }
    CALL_CB(cb, result == ModbusRtuMaster::MB_OK);
}
