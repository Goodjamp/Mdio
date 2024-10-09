#ifndef SWSETTINGS_H
#define SWSETTINGS_H

#include <QMap>
#include <QString>
#include <QJsonObject>
#include <QJsonDocument>
#include <QDebug>
#include <QByteArray>
#include <QDateTime>
#include "SwDefSettings.h"
#include "Version.h"

class CommunicationConfig: SwDefSettings
{
public:

    typedef enum {
        PARITY_NONE,
        PARITY_EVEN,
        PARITY_ODD,
    } Parity;

    typedef enum {
        STOP_BITS_1 = 1,
        STOP_BITS_2,
    } StopBits;

    CommunicationConfig(int brIn = 0, Parity parityIn = PARITY_NONE, StopBits stopBitsIn = STOP_BITS_1,
                          int silenIntervalIn = 0, int replyTimeouteIn = 0)
        : SwDefSettings()
    {
        int brIndex = 0;
        if ((brIndex = getBrList().lastIndexOf(brIn)) == -1)
        {
            return;
        }

        if (getSilentIntervaMinList().at(brIndex)> silenIntervalIn)
        {
            return;
        }

        if (getSilentIntervaMaxList().at(brIndex) < silenIntervalIn)
        {
            return;
        }

        if (getTimeoutReplyMin() > replyTimeouteIn
            || getTimeoutReplyMax() < replyTimeouteIn) {
            return;
        }

        Br = brIn;
        parity = parityIn;
        stopBits = stopBitsIn;
        silenInterval = silenIntervalIn;
        replyTimeoute = replyTimeouteIn;
        result = true;
    };

    int getBr() {
        return Br;
    }

    Parity getParity() {
        return parity;
    }

    StopBits getStopBits() {
        return stopBits;
    }

    int getSilentInterval() {
        return silenInterval;
    }

    int getReplyTimeoute() {
        return replyTimeoute;
    }

    bool isInit() {
        return result;
    }

private:
    bool result = false;
    int Br;
    Parity parity;
    StopBits stopBits;
    int silenInterval;
    int replyTimeoute;
};

class TsConfig
      : SwDefSettings
{

public:
    TsConfig(int switchTimeIn, int debounceTimeIn, QVector<bool> isInvertIn, QVector<bool> isDoubleIn)
        :SwDefSettings()
    {
        if(debounceTimeIn < getDebounceMin()
           || debounceTimeIn > getDebounceMax())
        {
            return;
        }

        if(switchTimeIn < getSwitchTimeMin()
            || switchTimeIn > getSwitchTimeMax())
        {
            return;
        }
        if (isInvertIn.size() > getTsSingleNumber()) {
            return;
        }
        if (isDoubleIn.size() > getTsDoubleNumber()) {
            return;
        }

        isInvert = isInvertIn;
        isDouble = isDoubleIn;
        switchTime = switchTimeIn;
        debounceTime = debounceTimeIn;
        result = true;
    }

    bool isInit() {
        return result;
    }

    QVector<bool> getInvertSign()
    {
        return isInvert;
    }

    QVector<bool> getDoubleSign()
    {
        return isDouble;
    }

    int getDebounceTime()
    {
        return debounceTime;
    }

    int getSwitchTime()
    {
        return switchTime;
    }

private:
    QVector<bool> isInvert;
    QVector<bool> isDouble;
    bool result = false;
    int switchTime;
    int debounceTime;
};

class TcConfig
    : SwDefSettings
{

public:
    TcConfig(int pulsDurationIn)
        :SwDefSettings()
    {
        if(pulsDurationIn < getPulsDurationMin()
            || pulsDurationIn > getPulsDurationMax())
        {
            return;
        }
        pulsDuration = pulsDurationIn;
        result = true;
    }

    bool isInit() {
        return result;
    }

    int getPulsDuration()
    {
        return pulsDuration;
    }

private:

    bool result = false;
    int pulsDuration;
};

class SwSettings: SwDefSettings
{
public:
    SwSettings()
        : SwDefSettings()
    {}

    QByteArray getJsonFile();
    bool addCommunicationSettings(CommunicationConfig config);
    bool addTsSettings(TsConfig config);
    bool addTcSettings(TcConfig config);

private:

    typedef enum {
        SETTINGS_KEY_DATE,
        SETTINGS_KEY_SW_VERSION,
        SETTINGS_KEY_COMMUNICATION,
        SETTINGS_KEY_COMMUNICATION_BR,
        SETTINGS_KEY_COMMUNICATION_PARITY,
        SETTINGS_KEY_COMMUNICATION_STOP_BITS,
        SETTINGS_KEY_COMMUNICATION_SILENT_INTERVAL,
        SETTINGS_KEY_COMMUNICATION_REPLY_DELAY,
        SETTINGS_KEY_TS,
        SETTINGS_KEY_TS_DEBOUNCE_TIME,
        SETTINGS_KEY_TS_SWITCH_TIME,
        SETTINGS_KEY_TS_INVERS_SIGN,
        SETTINGS_KEY_TS_DOUBLE_SIGN,
        SETTINGS_KEY_TС,
        SETTINGS_KEY_TС_PULS_DURATION,
    } SettingsKey;

    QMap<SettingsKey, QString> keyList{
        {SETTINGS_KEY_DATE, "Date"},
        {SETTINGS_KEY_COMMUNICATION, "Communication"},
        {SETTINGS_KEY_COMMUNICATION_BR, "Boad rate"},
        {SETTINGS_KEY_COMMUNICATION_PARITY, "Parity"},
        {SETTINGS_KEY_COMMUNICATION_STOP_BITS, "Stop bits"},
        {SETTINGS_KEY_COMMUNICATION_SILENT_INTERVAL, "Silent interval"},
        {SETTINGS_KEY_COMMUNICATION_REPLY_DELAY, "Reply delay"},
        {SETTINGS_KEY_TS, "TS"},
        {SETTINGS_KEY_TS_DEBOUNCE_TIME, "Debounce time"},
        {SETTINGS_KEY_TS_SWITCH_TIME, "Switch time"},
        {SETTINGS_KEY_TS_INVERS_SIGN, "Inversion"},
        {SETTINGS_KEY_TS_DOUBLE_SIGN, "Double"},
        {SETTINGS_KEY_DATE, "Date"},
        {SETTINGS_KEY_SW_VERSION, "Sw version"},
        {SETTINGS_KEY_TС, "TC"},
        {SETTINGS_KEY_TС_PULS_DURATION, "Puls duration"},
    };

    QStringList parityValue{
        {"None"},
        {"Even"},
        {"Odd"},
    };
    QJsonObject rootObj;
    QJsonObject communicationObj;
    QJsonObject tsObj;
    QJsonObject tcObj;
};

#endif // SWSETTINGS_H
