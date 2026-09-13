#include <QVector>
#include "communication.h"
#include "uidescription.h"


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
     * Try open port
     */
    if(modbus->open(port, baudRate, parity, stopBits) == false) {
        CALL_CB(cb, false);
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
        registersNumbers = ADDR_REG_TC_OFF_VALUE - baseConfReg + 1;
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
        configReg[ADDR_REG_COMMUNICATION_SETTINGS - baseConfReg] |= parity << MAP_PARITY_POS;
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
        configReg[ADDR_REG_TS_BINARY_SWITCHING_TIME - baseConfReg] = configuration.signalisation.binaryTsSwitchingTime;
        configReg[ADDR_REG_TS_BINATY_SWITCHING_SETTINGS - baseConfReg] = 0;
        for (uint32_t k = 0; k < TELESIGNAL_BINARY_NUMBERS; k++) {
            if (configuration.signalisation.isBinary[k] == true) {
                configReg[ADDR_REG_TS_BINATY_SWITCHING_SETTINGS - baseConfReg] |= static_cast<uint16_t>(1) << k;
            }
        }
        // Put TC Power Relay settings
        configReg[ADDR_REG_TS_BINATY_SWITCHING_SETTINGS - baseConfReg] |=
            configuration.control.usePowerRelay ? 1 : 0 << USE_POWER_RELAY_CONF_OFFSET;

        /*
         * Serialiase tele control settings
         */
        configReg[ADDR_REG_TC_PULS_DURATION - baseConfReg] = configuration.control.pulsDuration;
        configReg[ADDR_REG_TC_ON_VALUE - baseConfReg] = configuration.control.onVal;
        configReg[ADDR_REG_TC_OFF_VALUE - baseConfReg] = configuration.control.offVal;

        result = modbus->presetMultipleRegister(slaveAddress, baseConfReg, configReg);
        if (result == ModbusRtuMaster::MB_OK) {
            resultWriteConfiguration = true;
        } else {
           qDebug()<<"Error writeConfigurationSlot write configuration: "<<modbus->getStatusString(result);
        }
    } else {
        qDebug()<<"Error writeConfigurationSlot paity or stopBits error";
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
    registersNumbers = ADDR_REG_TC_OFF_VALUE - baseConfReg + 1;
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
        parity = static_cast<int>(MAP_PARITY_MASK & (configReg[ADDR_REG_COMMUNICATION_SETTINGS - baseConfReg] >> MAP_PARITY_POS));
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
            configuration.signalisation.binaryTsSwitchingTime = configReg[ADDR_REG_TS_BINARY_SWITCHING_TIME - baseConfReg];
            for (uint32_t k = 0; k < TELESIGNAL_BINARY_NUMBERS; k++) {
                configuration.signalisation.isBinary[k] = (1 & (configReg[ADDR_REG_TS_BINATY_SWITCHING_SETTINGS - baseConfReg] >> k)) == 1;
            }

            /*
             * Deserialiase tele control settings
             */
            configuration.control.pulsDuration = configReg[ADDR_REG_TC_PULS_DURATION - baseConfReg];
            configuration.control.onVal = configReg[ADDR_REG_TC_ON_VALUE - baseConfReg];
            configuration.control.offVal = configReg[ADDR_REG_TC_OFF_VALUE - baseConfReg];

            /*
             * Deserialiase Power Relay settings
             */
            configuration.control.usePowerRelay =
                (1 & (configReg[ADDR_REG_TS_BINATY_SWITCHING_SETTINGS - baseConfReg] >> USE_POWER_RELAY_CONF_OFFSET)) == 1;
            resulReadConfiguration = true;
        } else {
            qDebug()<<"Error readConfigurationSlot parity or stop bits error:";
        }
    } else {
        qDebug()<<"Error readConfigurationSlot read configuration: "<<modbus->getStatusString(result);
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
        qDebug()<<"Error readMetaInformationSlot: "<<modbus->getStatusString(result);
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
        qDebug()<<"Error reloadSlot: "<<modbus->getStatusString(result);
        CALL_CB(cb, false);
    }
}

void Communication::readStateSlot(std::function<void(bool result, SlaveState state)> cb,
                                  int slaveAddress)
{
    QVector<bool> status;
    QVector<uint16_t> teleControl;
    ModbusRtuMaster::MbStatus result;
    SlaveState state;
    uint16_t baseCoilAddress;
    uint16_t readCoilsNumber;
    uint16_t baseTcAddress;

    /*
     * Read 220V in circuit
     */
    QVector<int> errorCoilAddress = UiDescription::getErrorAddressList();
    baseCoilAddress = ADDR_COIL_220_V_ERROR;
    readCoilsNumber = ADDR_COIL_220_V_ERROR - baseCoilAddress + 1;
    result = modbus->readDiscreteInputs(slaveAddress, baseCoilAddress, readCoilsNumber, status);
    if (result != ModbusRtuMaster::MB_OK) {
        CALL_CB(cb, false, state);
        qDebug()<<"Error readStateSlot read 220_V: "<<modbus->getStatusString(result);
        return;
    }
    state.error220 =  status[ADDR_COIL_220_V_ERROR - baseCoilAddress];

    /*
     *  Read EEPROM_ERROR and EEPROM_CLEAR status
     */
    baseCoilAddress = ADDR_COIL_220_V_ERROR;
    readCoilsNumber = ADDR_COIL_RELAY_ERROR - baseCoilAddress + 1;
    result = modbus->readDiscreteInputs(slaveAddress, baseCoilAddress, readCoilsNumber, status);
    if (result != ModbusRtuMaster::MB_OK) {
        CALL_CB(cb, false, state);
        qDebug()<<"Error readStateSlot read globalStatusReg: "<<modbus->getStatusString(result);
        return;
    }
    state.error220 = status[ADDR_COIL_220_V_ERROR - baseCoilAddress];
    state.errorEeprom = status[ADDR_COIL_EEPROM_ERROR - baseCoilAddress];
    state.errorEepromClear =  status[ADDR_COIL_EEPROM_CLEAR_ERROR - baseCoilAddress];
    state.errorRelayError =  status[ADDR_COIL_RELAY_ERROR - baseCoilAddress];

    /*
     * According to the documentation, if STATUS_220 is set, the device replay with
     * exception. Therefore we read the tele-signal information only if STATUS_220 == false
     */
    if (state.error220 == false) {
        /*
         * Read tele signal status
         */
        QVector<bool> teleSignalStatusBin;
        baseCoilAddress = ADDR_COIL_TC_1;
        readCoilsNumber = ADDR_COIL_TC_4 - baseCoilAddress + 1;
        result = modbus->readDiscreteInputs(slaveAddress, baseCoilAddress, readCoilsNumber, teleSignalStatusBin);
        if (result != ModbusRtuMaster::MB_OK) {
            CALL_CB(cb, false, state);
            qDebug()<<"Error readStateSlot read read signals: "<<modbus->getStatusString(result);
            return;
        }
        for (uint32_t k = 0; k < TELESIGNAL_NUMBERS; k++) {
            state.signalisation[k] = teleSignalStatusBin[k];
        }

        /*
         * Read tele signal status from binary register
         */
        QVector<uint16_t> teleSignalStatusRegister;
        result = modbus->readHoldingRegisters(slaveAddress, ADDR_REG_TS_BINARY_STATE, 1, teleSignalStatusRegister);
        if (result != ModbusRtuMaster::MB_OK) {
            CALL_CB(cb, false, state);
            qDebug()<<"Error readStateSlot read read signals: "<<modbus->getStatusString(result);
            return;
        }

        for (uint32_t k = 0; k < TELESIGNAL_NUMBERS; k++) {
            state.signalisationBinary[k] = teleSignalStatusRegister[0] & (1 << k);
        }
    } else {
        for (uint32_t k = 0; k < TELESIGNAL_NUMBERS; k++) {
            state.signalisation[k] = false;
        }
    }

    /*
     * Read the puls tele control state.
     */
    baseTcAddress = ADDR_REG_TELE_CONTROL_1;
    result = modbus->readInputRegisters(slaveAddress, baseTcAddress, TELECONTROL_PULS_NUMBERS, teleControl);
    if (result != ModbusRtuMaster::MB_OK) {
        CALL_CB(cb, false, state);
        qDebug()<<"Error readStateSlot read puls tele control: "<<modbus->getStatusString(result);
        return;
    }

    /*
     * Test and apply the context of the puls telecontrol registers
     */
    state.control[ADDR_REG_TELE_CONTROL_1 - baseTcAddress] = state.control[ADDR_REG_TELE_CONTROL_1 - baseTcAddress];

    /*
     * Read the puls tele control state.
     */
    baseTcAddress = ADDR_REG_TELE_CONTROL_2;
    result = modbus->readInputRegisters(slaveAddress, baseTcAddress, TELECONTRO_STATIC_NUMBERS, teleControl);
    if (result != ModbusRtuMaster::MB_OK) {
        CALL_CB(cb, false, state);
        qDebug()<<"Error readStateSlot read static tele control: "<<modbus->getStatusString(result);
        return;
    }

    /*
     * Test and apply the context of the static telecontrol registers
     */
    for (uint32_t k = 0; k < TELECONTRO_STATIC_NUMBERS; k++) {
        state.control[k + 1] = teleControl[k];
    }
    CALL_CB(cb, true, state);
}

void Communication::setTeleControlSlot(std::function<void(bool result)> cb,
                                       int slaveAddress, int index, unsigned int val, bool fun5)
{
    ModbusRtuMaster::MbStatus result;

    if (index > TELECONTRO_STATIC_NUMBERS) {
        qDebug()<<"Error setTeleControlSlot index value error: "<<index;
        CALL_CB(cb, false);
        return;
    }
    result = fun5
             ? modbus->forceSingleCoil(slaveAddress, ADDR_REG_TELE_CONTROL_2 + index, (uint16_t)(0xFFFF & val))
             : modbus->presetSingleRegister(slaveAddress, ADDR_REG_TELE_CONTROL_2 + index, (uint16_t)(0xFFFF & val));
    if (result != ModbusRtuMaster::MB_OK) {
        qDebug()<<"Error setTeleControlSlot reply: "<<modbus->getStatusString(result);
    }
    CALL_CB(cb, result == ModbusRtuMaster::MB_OK);
}

void Communication::setTeleControlPulsSlot(std::function<void(bool result)> cb,
                                           int slaveAddress, unsigned int val, bool fun5)
{
    ModbusRtuMaster::MbStatus result;

    result = fun5
             ? modbus->forceSingleCoil(slaveAddress, ADDR_REG_TELE_CONTROL_1,  (uint16_t)(0xFFFF & val))
             : modbus->presetSingleRegister(slaveAddress, ADDR_REG_TELE_CONTROL_1, (uint16_t)(0xFFFF & val));
    if (result != ModbusRtuMaster::MB_OK) {
        qDebug()<<"Error setTeleControlPulsSlot reply: "<<modbus->getStatusString(result);
    }
    CALL_CB(cb, result == ModbusRtuMaster::MB_OK);
}
