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

        if (getSilentIntervalMinList().at(brIndex)> silenIntervalIn)
        {
            return;
        }

        if (getSilentIntervalMaxList().at(brIndex) < silenIntervalIn)
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

        invertSign = isInvertIn;
        doubleSign = isDoubleIn;
        switchTime = switchTimeIn;
        debounceTime = debounceTimeIn;
        result = true;
    }

    bool isInit() {
        return result;
    }

    QVector<bool> getInvertSign()
    {
        return invertSign;
    }

    QVector<bool> getDoubleSign()
    {
        return doubleSign;
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
    QVector<bool> invertSign;
    QVector<bool> doubleSign;
    bool result = false;
    int switchTime;
    int debounceTime;
};

class TcConfig
    : SwDefSettings
{

public:
    TcConfig(int pulsDurationIn, QString valOnIn, QString valOffIn)
        :SwDefSettings()
    {
        if(pulsDurationIn < getPulsDurationMin()
            || pulsDurationIn > getPulsDurationMax())
        {
            return;
        }
        pulsDuration = pulsDurationIn;
        valOn = valOnIn;
        valOff = valOffIn;
        result = true;
    }

    bool isInit() {
        return result;
    }

    int getPulsDuration()
    {
        return pulsDuration;
    }

    QString getValOn()
    {
        return valOn;
    }

    QString getValOff()
    {
        return valOff;
    }

private:

    bool result = false;
    int pulsDuration;
    QString valOn;
    QString valOff;
};

class SwSettings: SwDefSettings
{
public:
    SwSettings()
        : SwDefSettings()
    {}

    SwSettings(QByteArray file)
        : SwDefSettings()
    {
        if ((isFileSettingsValid = testSettings(file, fileSettingsErroStr)) == true)
        {
            rootObj = QJsonDocument::fromJson(file, NULL).object();
        }
    }

    QByteArray getJsonFile();
    bool addCommunicationSettings(CommunicationConfig config);
    bool addTsSettings(TsConfig config);
    bool addTcSettings(TcConfig config);

    static bool testSettings(QByteArray settingsJson, QString &errorStr);

    bool getState(QString &errorStr) {
        errorStr = fileSettingsErroStr;
        return isFileSettingsValid;
    }

    int getBr() {
        int value{0};

        if (isFileSettingsValid) {
            value = rootObj.find(keyCommunication)->toObject().find(keyBoadRate)->toInt();
        }
        return value;
    }

    QString getParity() {
        QString value{""};

        if (isFileSettingsValid) {
            value = rootObj.find(keyCommunication)->toObject().find(keyParity)->toString();
        }
        return value;
    }

    int getReplyDelay() {
        int value{0};

        if (isFileSettingsValid) {
            value = rootObj.find(keyCommunication)->toObject().find(keyReplyDelay)->toInt();
        }
        return value;
    }

    int getSilentInterval() {
        int value{0};

        if (isFileSettingsValid) {
            value = rootObj.find(keyCommunication)->toObject().find(keySilentInterval)->toInt();
        }
        return value;
    }

    int getStopBits() {
        int value{0};

        if (isFileSettingsValid) {
            value = rootObj.find(keyCommunication)->toObject().find(keyStopBits)->toInt();
        }
        return value;
    }

    QString getDate() {
        QString value{""};

        if (isFileSettingsValid) {
            value = rootObj.find(keyDate)->toString();
        }
        return "";
    }

    QString getSwVersion() {
        QString value{""};

        if (isFileSettingsValid) {
            value = rootObj.find(keySwVersion)->toString();
        }
        return value;
    }

    int getPulsDuration() {
        int value{0};

        if (isFileSettingsValid) {
            value = rootObj.find(keyTc)->toObject().find(keyPulsDuration)->toInt();
        }
        return value;
    }

    int getDebounceTime() {
        int value{0};

        if (isFileSettingsValid) {
            value = rootObj.find(keyTs)->toObject().find(keyDebounceTime)->toInt();
        }
        return value;
    }

    QString getOnValue() {
        QString value{""};

        if (isFileSettingsValid) {
            value = rootObj.find(keyTc)->toObject().find(keyValueOn)->toString();
        }
        return value;
    }

    QString getOffValue() {
        QString value{""};

        if (isFileSettingsValid) {
            value = rootObj.find(keyTc)->toObject().find(keyValueOff)->toString();
        }
        return value;
    }

    QVector<bool> getDoubleSign() {
        QVector<bool> value;

        if (isFileSettingsValid) {
            QJsonArray doubleSignArray = rootObj.find(keyTs)->toObject().find(keyDouble)->toArray();
            for (auto item: doubleSignArray) {
                value.push_back(tsTypeToBool(item.toString()));
            }
        }
        return value;
    }

    QVector<bool> getInversionSign() {
        QVector<bool> value;

        if (isFileSettingsValid) {
            QJsonArray inversionSignArray = rootObj.find(keyTs)->toObject().find(keyInversion)->toArray();
            for (auto item: inversionSignArray) {
                value.push_back(inversionToBool(item.toString()));
            }
        }
        return value;
    }

    int getSwitchTime() {
        int value{0};

        if (isFileSettingsValid) {
            value = rootObj.find(keyTs)->toObject().find(keySwitchTime)->toInt();
        }
        return value;
    }

private:

    static inline::QString keyDate{"Date"};
    static inline::QString keyCommunication{"Communication"};
    static inline::QString keyBoadRate{"Boad rate"};
    static inline::QString keyParity{"Parity"};
    static inline::QString keyStopBits{"Stop bits"};
    static inline::QString keySilentInterval{"Silent interval"};
    static inline::QString keyReplyDelay{"Reply delay"};
    static inline::QString keyTs{"TS"};
    static inline::QString keyDebounceTime{"Debounce time"};
    static inline::QString keySwitchTime{"Switch time"};
    static inline::QString keyInversion{"Inversion"};
    static inline::QString keyDouble{"Type"};
    static inline::QString keySwVersion{"Sw version"};
    static inline::QString keyTc{"TC"};
    static inline::QString keyPulsDuration{"Puls duration"};
    static inline::QString keyValueOn{"Value On"};
    static inline::QString keyValueOff{"Value Off"};

    static inline::QStringList parityValue{
        {"None"},
        {"Even"},
        {"Odd"},
    };

    static inline::QStringList tsTypeStrList{
        {"Double"},
        {"Single"},
    };

    static inline::QStringList tsInversionStrList{
        {"Inverse"},
        {"NoInverse"},
    };

    static bool tsTypeToBool(QString value) {
        return value == tsTypeStrList[0];
    }

    static QString tsTypeToStr(bool isDouble) {
        return isDouble ? tsTypeStrList[0] : tsTypeStrList[1];
    }

    static bool inversionToBool(QString value) {
        return value == tsInversionStrList[0];
    }

    static QString inversionToStr(bool isInverse) {
        return isInverse ? tsInversionStrList[0] : tsInversionStrList[1];
    }

    QJsonObject rootObj;
    QJsonObject communicationObj;
    QJsonObject tsObj;
    QJsonObject tcObj;
    QString fileSettingsErroStr;
    bool isFileSettingsValid;

    bool testMetaInformation();
    bool testCommunication();
    bool testTeleControl();
    bool testTeleSignalisation();

    typedef enum {
        FILE_FORMAT_EROOR,
        ROOT_KEY_LIST_ERROR,
        KEY_DATE_FORMAT_ERROR,
        KEY_VERSION_FORMAT_ERROR,
        KEY_COMMUNICATION_MISSING_ERROR,
        KEY_COMMUNICATION_FORMAT_ERROR,
        KEY_BOAD_RATE_MISSING,
        KEY_BOAD_RATE_FORMAT_ERROR,
        KEY_BOAD_RATE_VALUE_ERROR,
        KEY_PARITY_MISSING,
        KEY_PARITY_FORMAT_ERROR,
        KEY_PARITY_VALUE_ERROR,
        KEY_REPLY_DELAY_MISSING,
        KEY_REPLY_DELAY_FORMAT_ERROR,
        KEY_REPLY_DELAY_VALUE_ERROR,
        KEY_SILENT_INTERVAL_MISSING,
        KEY_SILENT_INTERVAL_FORMAT_ERROR,
        KEY_SILENT_INTERVAL_VALUE_ERROR,
        KEY_STOP_BITS_MISSING,
        KEY_STOP_BITS_FORMAT_ERROR,
        KEY_STOP_BITS_VALUE_ERROR,
        KEY_TC_MISSING,
        KEY_TC_FORMAT_ERROR,
        KEY_PULS_DURATION_MISSING,
        KEY_PULS_DURATION_FORMAT_ERROR,
        KEY_PULS_DURATION_VALUE_ERROR,
        KEY_TS_MISSING,
        KEY_TS_FORMAT_ERROR,
        KEY_DEBONCE_TIME_MISSING,
        KEY_DEBONCE_TIME_FORMAT_ERROR,
        KEY_DEBONCE_TIME_VALUE_ERROR,
        KEY_DOUBLE_MISSING,
        KEY_DOUBLE_FORMAT_ERROR,
        KEY_DOUBLE_FORMAT_SIZE_ERROR,
        KEY_DOUBLE_FORMAT_ITEM_ERROR,
        KEY_DOUBLE_VALUE_ITEM_ERROR,
        KEY_INVERSION_MISSING,
        KEY_INVERSION_FORMAT_ERROR,
        KEY_INVERSION_FORMAT_SIZE_ERROR,
        KEY_INVERSION_FORMAT_ITEM_ERROR,
        KEY_INVERSION_VALUE_ITEM_ERROR,
        KEY_SWITCH_TIME_MISSING,
        KEY_SWITCH_TIME_FORMAT_ERROR,
        KEY_SWITCH_TIME_VALUE_ERROR,
        KEY_ON_VALUE_MISSING_ERROR,
        KEY_ON_VALUE_FORMAT_ERROR,
        KEY_ON_VALUE_VALUE_ERROR,
        KEY_OFF_VALUE_MISSING_ERROR,
        KEY_OFF_VALUE_FORMAT_ERROR,
        KEY_OFF_VALUE_VALUE_ERROR,
        KEY_OFF_VALUE_OFF_VALUE_COLISION_ERROR,
    } SettingsError;

    static inline::QMap<SettingsError, QString> errorStrList{
        {FILE_FORMAT_EROOR, "Помилка в загальному форматі <b>JSON</b>."},
        {ROOT_KEY_LIST_ERROR, "Нвірний набір ключів в корені налаштувань\n.Повинні бути присуині настпні ключі\n"
                              + keyDate + ", " + keyCommunication + ", " + keyTs + ", " + keyTc + ", " + keySwVersion + "."},
        {KEY_DATE_FORMAT_ERROR, "Ключ <b>" + keyDate + "</b> має невірний формат.\nОчікується рядок."},
        {KEY_VERSION_FORMAT_ERROR, "Ключ <b>" + keySwVersion + "</b> має невірний формат.\nОчікується рядок."},
        {KEY_COMMUNICATION_MISSING_ERROR, "Відсутній ключ <b>" + keyCommunication + "</b>."},
        {KEY_COMMUNICATION_FORMAT_ERROR, "Ключ <b>" + keyCommunication + "</b> має невірний формат.\nОчікується об'єкт JSON."},
        {KEY_BOAD_RATE_MISSING, "Відсутній ключ <b>" + keyCommunication + "->" + keyBoadRate + "</b>."},
        {KEY_BOAD_RATE_FORMAT_ERROR, "Ключ <b>" + keyCommunication + "->" + keyBoadRate + "</b> має невірний формат."},
        {KEY_BOAD_RATE_VALUE_ERROR, "Ключ <b>" + keyCommunication + "->" + keyBoadRate + "</b> має невірне значення."},
        {KEY_PARITY_MISSING, "Відсутній ключ <b>" + keyCommunication + "->" + keyParity + "</b>."},
        {KEY_PARITY_FORMAT_ERROR, "Ключ <b>" + keyCommunication + "->" + keyParity + "</b> має невірний формат."},
        {KEY_PARITY_VALUE_ERROR, "Ключ <b>" + keyCommunication + "->" + keyParity + "</b> має невірне значення."},
        {KEY_REPLY_DELAY_MISSING, "Відсутній ключ <b>" + keyCommunication + "->" + keyReplyDelay + "</b>."},
        {KEY_REPLY_DELAY_FORMAT_ERROR, "Ключ <b>" + keyCommunication + "->" + keyReplyDelay + "</b> має невірний формат."},
        {KEY_REPLY_DELAY_VALUE_ERROR, "Ключ <b>" + keyCommunication + "->" + keyReplyDelay + "</b> має невірне значення."},
        {KEY_SILENT_INTERVAL_MISSING, "Відсутній ключ <b>" + keyCommunication + "->" + keySilentInterval + "</b>."},
        {KEY_SILENT_INTERVAL_FORMAT_ERROR, "Ключ <b>" + keyCommunication + "->" + keySilentInterval + "</b> має невірний формат."},
        {KEY_SILENT_INTERVAL_VALUE_ERROR, "Ключ <b>" + keyCommunication + "->" + keySilentInterval + "</b> має невірне значення."},
        {KEY_STOP_BITS_MISSING, "Відсутній ключ <b>" + keyCommunication + "->" + keyStopBits + "</b>."},
        {KEY_STOP_BITS_FORMAT_ERROR, "Ключ <b>" + keyCommunication + "->" + keyStopBits + "</b> має невірний формат."},
        {KEY_STOP_BITS_VALUE_ERROR, "Ключ <b>" + keyCommunication + "->" + keyStopBits + "</b> має невірне значення."},
        {KEY_TC_MISSING, "Відсутній ключ <b>" + keyTc + "</b>."},
        {KEY_TC_FORMAT_ERROR, "Ключ <b>" + keyTc + "</b> має невірний формат.\nОчікується об'єкт JSON."},
        {KEY_PULS_DURATION_MISSING, "Відсутній ключ <b>" + keyTc + "->" + keyPulsDuration + "</b>."},
        {KEY_PULS_DURATION_FORMAT_ERROR, "Ключ <b>" + keyTc + "->" + keyPulsDuration + "</b> має невірний формат."},
        {KEY_PULS_DURATION_VALUE_ERROR, "Ключ <b>" + keyTc + "->" + keyPulsDuration + "</b> має невірне значення.\n"},
                                          //+ "<b>" + keyPulsDuration + "</b>" + "повинен бути в межах ["
                                          //+ SwDefSettings::getPulsDurationMaxString() + " ; " + SwDefSettings::getPulsDurationMinString() + "]"},
        {KEY_TS_MISSING, "Відсутній ключ <b>" + keyTs + "</b>."},
        {KEY_TS_FORMAT_ERROR, "Ключ <b>" + keyTs + "</b> має невірне значення.\nОчікується об'єкт JSON."},
        {KEY_DEBONCE_TIME_MISSING, "Відсутній ключ <b>" + keyTs + "->" + keyDebounceTime + "</b>."},
        {KEY_DEBONCE_TIME_FORMAT_ERROR, "Ключ <b>" + keyTs + "->" + keyDebounceTime + "</b> має невірний формат."},
        {KEY_DEBONCE_TIME_VALUE_ERROR, "Ключ <b>" + keyTs + "->" + keyDebounceTime + "</b> має невірне значення."},
        {KEY_DOUBLE_MISSING, "Відсутній ключ <b>" + keyTs + "->" + keyDouble + "</b>."},
        {KEY_DOUBLE_FORMAT_ERROR, "Ключ <b>" + keyTs + "->" + keyDouble + "</b> має невірний формат.\nВеличина повинна бути масивом."},
        {KEY_DOUBLE_FORMAT_SIZE_ERROR, "Ключ <b>" + keyTs + "->" + keyDouble + "</b> має невірний формат.\nРозмір масиву повинен бути 2."},
        {KEY_DOUBLE_FORMAT_ITEM_ERROR, "Ключ <b>" + keyTs + "->" + keyDouble + "</b> має невірний формат.\nОчікується масив рядків."},
        {KEY_DOUBLE_VALUE_ITEM_ERROR, "Ключ <b>" + keyTs + "->" + keyDouble + "</b> має невірне значення.\nЕлемент масиву повинен бути рядок <b>Double</b> або <b>Single</b>."},
        {KEY_INVERSION_MISSING, "Відсутній ключ <b>" + keyTs + "->" + keyInversion + "</b>."},
        {KEY_INVERSION_FORMAT_ERROR, "Ключ <b>" + keyTs + "->" + keyInversion + "</b> має невірний формат."},
        {KEY_INVERSION_FORMAT_SIZE_ERROR, "Ключ <b>" + keyTs + "->" + keyInversion + "</b> має невірний формат.\nРозмір масиву повинен бути 4."},
        {KEY_INVERSION_FORMAT_ITEM_ERROR, "Ключ <b>" + keyTs + "->" + keyInversion + "</b> має невірний формат.\nОчікується масив рядків."},
        {KEY_INVERSION_VALUE_ITEM_ERROR,  "Ключ <b>" + keyTs + "->" + keyInversion + "</b> має невірний формат.\nЕлемент масиву повинен бути рядок <b>Inverse</b> або <b>NoInverse</b>."},
        {KEY_SWITCH_TIME_MISSING, "Відсутній ключ <b>" + keyTs + "->" + keySwitchTime + "</b>"},
        {KEY_SWITCH_TIME_FORMAT_ERROR, "Ключ <b>" + keyTs + "->" + keySwitchTime + "</b> має невірний формат"},
        {KEY_SWITCH_TIME_VALUE_ERROR, "Ключ <b>" + keyTs + "->" + keySwitchTime + "</b> має невірне значення"},
        {KEY_ON_VALUE_MISSING_ERROR, "Відсутній ключ <b>" + keyTc + "->" + keyValueOn + "</b>."},
        {KEY_ON_VALUE_FORMAT_ERROR, "Ключ <b>" + keyTc + "->" + keyValueOn + "</b> має невірний формат."},
        {KEY_ON_VALUE_VALUE_ERROR, "Ключ <b>" + keyTc + "->" + keyValueOn + "</b> має невірне значення.\n Очікується 4-розрядне шістнадцяткове число"},
        {KEY_OFF_VALUE_MISSING_ERROR, "Відсутній ключ <b>" + keyTc + "->" + keyValueOff + "</b>."},
        {KEY_OFF_VALUE_FORMAT_ERROR, "Ключ <b>" + keyTc + "->" + keyValueOff + "</b> має невірний формат."},
        {KEY_OFF_VALUE_VALUE_ERROR, "Ключ <b>" + keyTc + "->" + keyValueOff + "</b> має невірне значення.\n Очікується 4-розрядне шістнадцяткове число"},
        {KEY_OFF_VALUE_OFF_VALUE_COLISION_ERROR, "Ключі <b>" + keyTc + "->" + keyValueOn + " та " + keyTc + "->" + keyValueOff + "</b> повинні мати різні значення"},
    };

    static bool testRootKeys(QJsonObject jsonObj, QString &errorStr);
    static bool testDate(QJsonObject jsonObj, QString &errorStr);
    static bool testSwVersion(QJsonObject jsonObj, QString &errorStr);
    static bool testCommunication(QJsonObject jsonObj, QString &errorStr);
    static bool testTs(QJsonObject jsonObj, QString &errorStr);
    static bool testTc(QJsonObject jsonObj, QString &errorStr);

    static bool test(QJsonObject rootObj, QString key, SettingsError error, QString &errorStr, std::function<bool(QString)> testVal);
    static bool test(QJsonObject rootObj, QString key, SettingsError error, QString &errorStr, std::function<bool(int)> testVal);
    static bool test(QJsonObject rootObj, QString key, SettingsError error, QString &errorStr, std::function<bool(QJsonArray)> testVal);
};
#endif // SWSETTINGS_H
