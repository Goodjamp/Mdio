#pragma once

#include <QString>
#include <QVector>
#include <QColor>
#include <QStringList>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonArray>
#include <QByteArray>

class SwDefSettings {
public:
    SwDefSettings()
    {

    }
    ~SwDefSettings()
    {

    }

    typedef struct {
        QString name;
        QStringList rel1State;
        QStringList rel2State;
    } MonitorDesr;

    static void init(QByteArray jsonFileContent)
    {
        QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonFileContent);
        rootObj = jsonDoc.object();
    }

    static QVector<MonitorDesr> getMonitorDescr()
    {
        QVector<MonitorDesr> argOut;
        MonitorDesr tempDesr;
        QJsonArray arr = rootObj.find(monTc)->toArray();

        for (auto item: rootObj.find(monTc)->toArray()) {
            tempDesr.name = item.toObject().find(monTcName)->toString();
            tempDesr.rel1State.clear();
            for (auto item: item.toObject().find(monTcRelayStr1)->toArray()) {
                tempDesr.rel1State.push_back(item.toString());
            }
            tempDesr.rel2State.clear();
            for (auto item: item.toObject().find(monTcRelayStr2)->toArray()) {
                tempDesr.rel2State.push_back(item.toString());
            }
            argOut.push_back(tempDesr);
        }

        return argOut;
    }

    static QVector<int> getBrList()
    {
        return objArrToIntVector(rootObj.find(keyPort)->toObject().find(keyPortBrLis)->toArray());
    }

    static QStringList getBrListString()
    {
        return objArrToStringList(rootObj.find(keyPort)->toObject().find(keyPortBrLis)->toArray());
    }

    static QVector<int> getParityList()
    {
        return objArrToIntVector(rootObj.find(keyPort)->toObject().find(keyPortParityList)->toArray());
    }

    static QStringList getParityListString()
    {
        return objArrToStringList(rootObj.find(keyPort)->toObject().find(keyPortParityList)->toArray());
    }

    static QVector<int> getStopBitList()
    {
        return objArrToIntVector(rootObj.find(keyPort)->toObject().find(keyPortStopBitsList)->toArray());
    }

    static QStringList getStopBitListString()
    {
        return objArrToStringList(rootObj.find(keyPort)->toObject().find(keyPortStopBitsList)->toArray());
    }

    static int getBrDefault()
    {
        return rootObj.find(keyPort)->toObject().find(keyPortBrDefault)->toInt();
    }

    static QString getBrDefaultString()
    {
        return QString::number(getBrDefault());
    }

    static QString getParityDefault()
    {
        return rootObj.find(keyPort)->toObject().find(keyPortParityDefault)->toString();
    }

    static int getStopBitsDefault()
    {
        return rootObj.find(keyPort)->toObject().find(keyPortStopBitsDefault)->toInt();
    }

    static QString getStopBitsDefaultStr()
    {
        return QString::number(getStopBitsDefault());
    }

    static QVector<int> getSilentIntervalDefaultList()
    {
        return objArrToIntVector(rootObj.find(keyModbus)->toObject().find(keyModbusSilentIntervaList)->toArray());
    }

    static QStringList getSilentIntervalDefaultListString()
    {
        return objArrToStringList(rootObj.find(keyModbus)->toObject().find(keyModbusSilentIntervaList)->toArray());
    }

    static QVector<int> getSilentIntervalMinList()
    {
        return objArrToIntVector(rootObj.find(keyModbus)->toObject().find(keyModbusSilentIntervaMinList)->toArray());
    }

    static QStringList getSilentIntervalMinListString()
    {
        return objArrToStringList(rootObj.find(keyModbus)->toObject().find(keyModbusSilentIntervaMinList)->toArray());
    }

    static QVector<int> getSilentIntervalMaxList()
    {
        return objArrToIntVector(rootObj.find(keyModbus)->toObject().find(keyModbusSilentIntervaMaxList)->toArray());
    }

    static QStringList getSilentIntervalMaxListString()
    {
        return objArrToStringList(rootObj.find(keyModbus)->toObject().find(keyModbusSilentIntervaMaxList)->toArray());
    }

    static int getSilentIntervalDefault()
    {
        return rootObj.find(keyModbus)->toObject().find(keyModbusSilentIntervalDefault)->toInt();
    }

    static QString getSilentIntervalDefaultString()
    {
        return QString::number(getSilentIntervalDefault());
    }

    static QString getSilentIntervalMin()
    {
        return rootObj.find(keyModbus)->toObject().find(keyModbusSilentIntervalMin)->toString();
    }

    static QString getSilentIntervalMax()
    {
        return rootObj.find(keyModbus)->toObject().find(keyModbusSilentIntervalMax)->toString();
    }

    static int getTimeoutReplyDefault()
    {
        return rootObj.find(keyModbus)->toObject().find(keyModbusTimeoutReplyDefault)->toInt();
    }

    static QString getTimeoutReplyDefaultString()
    {
        return QString::number(getTimeoutReplyDefault());
    }

    static int getTimeoutReplyMin()
    {
        return rootObj.find(keyModbus)->toObject().find(keyModbusTimeoutReplyMin)->toInt();
    }

    static QString getTimeoutReplyMinString()
    {
        return QString::number(getTimeoutReplyMin());
    }

    static int getTimeoutReplyMax()
    {
        return rootObj.find(keyModbus)->toObject().find(keyModbusTimeoutReplyMax)->toInt();
    }

    static QString getTimeoutReplyMaxString()
    {
        return QString::number(getTimeoutReplyMax());
    }

    static QString getSilentIntervalDefaultPc()
    {
        return rootObj.find(keyModbus)->toObject().find(keyModbusSilentIntervalDefaultPc)->toString();
    }

    static QString getSilentIntervalMinPc()
    {
        return rootObj.find(keyModbus)->toObject().find(keyModbusSilentIntervalMinPc)->toString();
    }

    static QString getSilentIntervalMaxPc()
    {
        return rootObj.find(keyModbus)->toObject().find(keyModbusSilentIntervalMaxPc)->toString();
    }

    static QString getTimeoutReplyDefaultPc()
    {
        return rootObj.find(keyModbus)->toObject().find(keyModbusTimeoutReplyDefaultPc)->toString();
    }

    static QString getTimeoutReplyMinPc()
    {
        return rootObj.find(keyModbus)->toObject().find(keyModbusTimeoutReplyMinPc)->toString();
    }

    static QString getTimeoutReplyMaxPc()
    {
        return rootObj.find(keyModbus)->toObject().find(keyModbusTimeoutReplyMaxPc)->toString();
    }

    static QString getAddressDefault()
    {
        return rootObj.find(keyModbus)->toObject().find(keyModbusAddressDefault)->toString();
    }

    static int getDebounceDefault()
    {
        return rootObj.find(keyTs)->toObject().find(keyTsDebounceDefault)->toInt();
    }

    static QString getDebounceDefaultString()
    {
        return QString::number(getDebounceDefault());
    }

    static int getDebounceMin()
    {
        return rootObj.find(keyTs)->toObject().find(keyTsDebounceMin)->toInt();
    }

    static QString getDebounceMinString()
    {
        return QString::number(getDebounceMin());
    }

    static int getDebounceMax()
    {
        return rootObj.find(keyTs)->toObject().find(keyTsDebounceMax)->toInt();
    }

    static QString getDebounceMaxString()
    {
        return QString::number(getDebounceMax());
    }

    static int getBinaryTsSwitchTimeDefault()
    {
        return rootObj.find(keyBinaryTs)->toObject().find(keyBinaryTsSwitchTimeDefault)->toInt();
    }

    static QString getBinaryTsSwitchTimeDefaultString()
    {
        return QString::number(getBinaryTsSwitchTimeDefault());
    }

    static int getSwitchTimeMin()
    {
        return rootObj.find(keyBinaryTs)->toObject().find(keyBinaryTsSwitchTimeMin)->toInt();
    }

    static QString getSwitchTimeMinString()
    {
        return QString::number(getSwitchTimeMin());
    }

    static int getSwitchTimeMax()
    {
        return rootObj.find(keyBinaryTs)->toObject().find(keyBinaryTsSwitchTimeMax)->toInt();
    }

    static QString getSwitchTimeMaxString()
    {
        return QString::number(getSwitchTimeMax());
    }

    static int getTsSingleNumber()
    {
        return rootObj.find(keyTs)->toObject().find(keyTsSingleNumber)->toInt();
    }

    static int getTsDoubleNumber()
    {
        return rootObj.find(keyTs)->toObject().find(keyTsDoubleNumber)->toInt();
    }

    static int getPulsDurationDefault()
    {
        return rootObj.find(keyTc)->toObject().find(keyTcPulsDurationDefault)->toInt();
    }

    static QString getPulsDurationDefaultString()
    {
        return QString::number(getPulsDurationDefault());
    }

    static int getPulsDurationMin()
    {
        return rootObj.find(keyTc)->toObject().find(keyTcPulsDurationMin)->toInt();
    }

    static QString getPulsDurationMinString()
    {
        return QString::number(getPulsDurationMin());
    }

    static int getPulsDurationMax()
    {
        return rootObj.find(keyTc)->toObject().find(keyTcPulsDurationMax)->toInt();
    }

    static QString getPulsDurationMaxString()
    {
        return QString::number(getPulsDurationMax());
    }

    inline static const QString settingsFilePath = ":/Settings.json";
private:

    inline static const QString monTc = "monitorTC";
    inline static const QString monTcName = "Name";
    inline static const QString monTcRelayStr1 = "RelayStr1";
    inline static const QString monTcRelayStr2 = "RelayStr2";

    inline static const QString keyPort = "Port";
    inline static const QString keyPortBrLis = "BrList";
    inline static const QString keyPortBrDefault = "BrDefault";
    inline static const QString keyPortParityList = "ParityList";
    inline static const QString keyPortParityDefault = "ParityDefault";
    inline static const QString keyPortStopBitsList = "StopBitsList";
    inline static const QString keyPortStopBitsDefault = "StopBitsDefault";

    inline static const QString keyModbus = "Modbus";
    inline static const QString keyModbusSilentIntervaList = "SilentIntervaDefaultList";
    inline static const QString keyModbusSilentIntervaMinList = "SilentIntervaMinList";
    inline static const QString keyModbusSilentIntervaMaxList = "SilentIntervaMaxList";
    inline static const QString keyModbusSilentIntervalDefault = "SilentIntervalDefault";
    inline static const QString keyModbusSilentIntervalMin = "SilentIntervalMin";
    inline static const QString keyModbusSilentIntervalMax = "SilentIntervalMax";
    inline static const QString keyModbusTimeoutReplyDefault = "TimeoutReplyDefault";
    inline static const QString keyModbusTimeoutReplyMin = "TimeoutReplyMin";
    inline static const QString keyModbusTimeoutReplyMax = "TimeoutReplyMax";
    inline static const QString keyModbusSilentIntervalDefaultPc = "SilentIntervalDefaultPc";
    inline static const QString keyModbusSilentIntervalMinPc = "SilentIntervalMinPc";
    inline static const QString keyModbusSilentIntervalMaxPc = "SilentIntervalMaxPc";
    inline static const QString keyModbusTimeoutReplyDefaultPc = "TimeoutReplyDefaultPc";
    inline static const QString keyModbusTimeoutReplyMinPc = "TimeoutReplyMinPc";
    inline static const QString keyModbusTimeoutReplyMaxPc = "TimeoutReplyMaxPc";
    inline static const QString keyModbusAddressDefault = "AddressDefault";

    inline static const QString keyTs = "TS";
    inline static const QString keyTsDebounceDefault = "DebounceDefault";
    inline static const QString keyTsDebounceMin = "DebounceMin";
    inline static const QString keyTsDebounceMax = "DebounceMax";

    inline static const QString keyBinaryTs = "BinaryTs";
    inline static const QString keyBinaryTsSwitchTimeDefault = "SwitchTimeDefault";
    inline static const QString keyBinaryTsSwitchTimeMin = "SwitchTimeMin";
    inline static const QString keyBinaryTsSwitchTimeMax = "SwitchTimeMax";

    inline static const QString keyTc = "TC";
    inline static const QString keyTcPulsDurationDefault = "PulsDurationDefault";
    inline static const QString keyTcPulsDurationMin = "PulsDurationMin";
    inline static const QString keyTcPulsDurationMax = "PulsDurationMax";

    inline static const QString keyTsSingleNumber = "TsSingleNumber";
    inline static const QString keyTsDoubleNumber = "TsDoubleNumber";

    inline static QJsonObject rootObj;
    static QStringList objArrToStringList(QJsonArray array);
    static QVector<int> objArrToIntVector(QJsonArray array);
};
