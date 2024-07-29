#pragma once

#include <QString>
#include <QVector>
#include <QColor>
#include <QStringList>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonArray>

class SwDefaultSettings {
public:
    SwDefaultSettings();
    ~SwDefaultSettings();

    typedef struct {
        QString name;
        QStringList rel1State;
        QStringList rel2State;
    } MonitorDesr;

    QVector<MonitorDesr> getMonitorDesсr()
    {
        QVector<MonitorDesr> argOut;
        MonitorDesr tempDesr;
        QJsonArray arr = rootObj.find(monTc)->toArray();

        foreach (auto item, rootObj.find(monTc)->toArray()) {
            tempDesr.name = item.toObject().find(monTcName)->toString();
            tempDesr.rel1State.clear();
            foreach (auto item, item.toObject().find(monTcRelayStr1)->toArray()) {
                tempDesr.rel1State.push_back(item.toString());
            }
            tempDesr.rel2State.clear();
            foreach (auto item, item.toObject().find(monTcRelayStr2)->toArray()) {
                tempDesr.rel2State.push_back(item.toString());
            }
            argOut.push_back(tempDesr);
        }

        return argOut;
    }

    QStringList getBrList()
    {
        QJsonArray array = rootObj.find(keyPort)->toObject().find(keyPortBrLis)->toArray();
        return objArrToVector(array);
    }

    QStringList getParityList()
    {
        QJsonArray array = rootObj.find(keyPort)->toObject().find(keyPortParityList)->toArray();
        return objArrToVector(array);
    }

    QStringList getStopBitList()
    {
        QJsonArray array = rootObj.find(keyPort)->toObject().find(keyPortStopBitsList)->toArray();
        return objArrToVector(array);
    }

    QString getBrDefault()
    {
        return rootObj.find(keyPort)->toObject().find(keyPortBrDefault)->toString();
    }

    QString getParityDefault()
    {
        return rootObj.find(keyPort)->toObject().find(keyPortParityDefault)->toString();
    }

    QString getStopBitsDefault()
    {
        return rootObj.find(keyPort)->toObject().find(keyPortStopBitsDefault)->toString();
    }

    QStringList getSilentIntervaList()
    {
        QJsonArray array = rootObj.find(keyModbus)->toObject().find(keyModbusSilentIntervaList)->toArray();
        return objArrToVector(array);
    }

    QStringList getSilentIntervaMinList()
    {
        QJsonArray array = rootObj.find(keyModbus)->toObject().find(keyModbusSilentIntervaMinList)->toArray();
        return objArrToVector(array);
    }

    QStringList getSilentIntervaMaxList()
    {
        QJsonArray array = rootObj.find(keyModbus)->toObject().find(keyModbusSilentIntervaMaxList)->toArray();
        return objArrToVector(array);
    }

    QString getSilentIntervalDefault()
    {
        return rootObj.find(keyModbus)->toObject().find(keyModbusSilentIntervalDefault)->toString();
    }

    QString getSilentIntervalMin()
    {
        return rootObj.find(keyModbus)->toObject().find(keyModbusSilentIntervalMin)->toString();
    }

    QString getSilentIntervalMax()
    {
        return rootObj.find(keyModbus)->toObject().find(keyModbusSilentIntervalMax)->toString();
    }

    QString getTimeoutReplyDefault()
    {
        return rootObj.find(keyModbus)->toObject().find(keyModbusTimeoutReplyDefault)->toString();
    }

    QString getTimeoutReplyMin()
    {
        return rootObj.find(keyModbus)->toObject().find(keyModbusTimeoutReplyMin)->toString();
    }

    QString getTimeoutReplyMax()
    {
        return rootObj.find(keyModbus)->toObject().find(keyModbusTimeoutReplyMax)->toString();
    }

    QString getSilentIntervalDefaultPc()
    {
        return rootObj.find(keyModbus)->toObject().find(keyModbusSilentIntervalDefaultPc)->toString();
    }

    QString getSilentIntervalMinPc()
    {
        return rootObj.find(keyModbus)->toObject().find(keyModbusSilentIntervalMinPc)->toString();
    }

    QString getSilentIntervalMaxPc()
    {
        return rootObj.find(keyModbus)->toObject().find(keyModbusSilentIntervalMaxPc)->toString();
    }

    QString getTimeoutReplyDefaultPc()
    {
        return rootObj.find(keyModbus)->toObject().find(keyModbusTimeoutReplyDefaultPc)->toString();
    }

    QString getTimeoutReplyMinPc()
    {
        return rootObj.find(keyModbus)->toObject().find(keyModbusTimeoutReplyMinPc)->toString();
    }

    QString getTimeoutReplyMaxPc()
    {
        return rootObj.find(keyModbus)->toObject().find(keyModbusTimeoutReplyMaxPc)->toString();
    }

    QString getAddressDefault()
    {
        return rootObj.find(keyModbus)->toObject().find(keyModbusAddressDefault)->toString();
    }

    QString getDebounceDefault()
    {
        return rootObj.find(keyTs)->toObject().find(keyTsDebounceDefault)->toString();
    }

    QString getDebounceMin()
    {
        return rootObj.find(keyTs)->toObject().find(keyTsDebounceMin)->toString();
    }

    QString getDebounceMax()
    {
        return rootObj.find(keyTs)->toObject().find(keyTsDebounceMax)->toString();
    }

    QString getSwitchTimeDefault()
    {
        return rootObj.find(keyBinaryTs)->toObject().find(keyBinaryTsSwitchTimeDefault)->toString();
    }

    QString getSwitchTimeMin()
    {
        return rootObj.find(keyBinaryTs)->toObject().find(keyBinaryTsSwitchTimeMin)->toString();
    }

    QString getSwitchTimeMax()
    {
        return rootObj.find(keyBinaryTs)->toObject().find(keyBinaryTsSwitchTimeMax)->toString();
    }

    QString getPulsDurationDefault()
    {
        return rootObj.find(keyTc)->toObject().find(keyTcPulsDurationDefault)->toString();
    }

    QString getPulsDurationMin()
    {
        return rootObj.find(keyTc)->toObject().find(keyTcPulsDurationMin)->toString();
    }

    QString getPulsDurationMax()
    {
        return rootObj.find(keyTc)->toObject().find(keyTcPulsDurationMax)->toString();
    }


private:

    const QString settingsFilePath{":/Settings.json"};

    const QString monTc{"monitorTC"};
    const QString monTcName{"Name"};
    const QString monTcRelayStr1{"RelayStr1"};
    const QString monTcRelayStr2{"RelayStr2"};

    const QString keyPort{"Port"};
    const QString keyPortBrLis{"BrList"};
    const QString keyPortBrDefault{"BrDefault"};
    const QString keyPortParityList{"ParityList"};
    const QString keyPortParityDefault{"ParityDefault"};
    const QString keyPortStopBitsList{"StopBitsList"};
    const QString keyPortStopBitsDefault{"StopBitsDefault"};

    const QString keyModbus{"Modbus"};
    const QString keyModbusSilentIntervaList{"SilentIntervaList"};
    const QString keyModbusSilentIntervaMinList{"SilentIntervaMinList"};
    const QString keyModbusSilentIntervaMaxList{"SilentIntervaMaxList"};
    const QString keyModbusSilentIntervalDefault{"SilentIntervalDefault"};
    const QString keyModbusSilentIntervalMin{"SilentIntervalMin"};
    const QString keyModbusSilentIntervalMax{"SilentIntervalMax"};
    const QString keyModbusTimeoutReplyDefault{"TimeoutReplyDefault"};
    const QString keyModbusTimeoutReplyMin{"TimeoutReplyMin"};
    const QString keyModbusTimeoutReplyMax{"TimeoutReplyMax"};
    const QString keyModbusSilentIntervalDefaultPc{"SilentIntervalDefaultPc"};
    const QString keyModbusSilentIntervalMinPc{"SilentIntervalMinPc"};
    const QString keyModbusSilentIntervalMaxPc{"SilentIntervalMaxPc"};
    const QString keyModbusTimeoutReplyDefaultPc{"TimeoutReplyDefaultPc"};
    const QString keyModbusTimeoutReplyMinPc{"TimeoutReplyMinPc"};
    const QString keyModbusTimeoutReplyMaxPc{"TimeoutReplyMaxPc"};
    const QString keyModbusAddressDefault{"AddressDefault"};

    const QString keyTs{"TS"};
    const QString keyTsDebounceDefault{"DebounceDefault"};
    const QString keyTsDebounceMin{"DebounceMin"};
    const QString keyTsDebounceMax{"DebounceMax"};

    const QString keyBinaryTs{"BinaryTs"};
    const QString keyBinaryTsSwitchTimeDefault{"SwitchTimeDefault"};
    const QString keyBinaryTsSwitchTimeMin{"SwitchTimeMin"};
    const QString keyBinaryTsSwitchTimeMax{"SwitchTimeMax"};

    const QString keyTc{"TC"};
    const QString keyTcPulsDurationDefault{"PulsDurationDefault"};
    const QString keyTcPulsDurationMin{"PulsDurationMin"};
    const QString keyTcPulsDurationMax{"PulsDurationMax"};

    QJsonObject rootObj;

    QStringList objArrToVector(QJsonArray &array);
};
