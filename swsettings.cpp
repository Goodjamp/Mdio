#include "swsettings.h"

QByteArray SwSettings::getJsonFile()
{
    QDate date = QDate::currentDate();
    QTime time = QTime::currentTime();
    QString dateTime = QString::number(date.month()) + "."
                       + QString::number(date.day()) + "."
                       + QString::number(date.year()) + " "
                       + QString::number(time.hour()) + ":"
                       + QString::number(time.minute()) + ":"
                       + QString::number(time.second());
    rootObj.insert(keyList.value(SETTINGS_KEY_DATE), dateTime);
    rootObj.insert(keyList.value(SETTINGS_KEY_SW_VERSION), (QString)SW_VERSION_STR);
    rootObj.insert(keyList.value(SETTINGS_KEY_COMMUNICATION), (QJsonValue)communicationObj);
    rootObj.insert(keyList.value(SETTINGS_KEY_TS), (QJsonValue)tsObj);
    rootObj.insert(keyList.value(SETTINGS_KEY_TС), (QJsonValue)tcObj);
    QJsonDocument jsonDoc(rootObj);
    return jsonDoc.toJson(QJsonDocument::Indented); // return byte array
}

bool SwSettings::addCommunicationSettings(CommunicationConfig config)
{
    if (config.isInit() == false) {
        return false;
    }
    communicationObj.insert(keyList.value(SETTINGS_KEY_COMMUNICATION_BR), (QJsonValue)config.getBr());
    communicationObj.insert(keyList.value(SETTINGS_KEY_COMMUNICATION_STOP_BITS), (QJsonValue)config.getStopBits());
    communicationObj.insert(keyList.value(SETTINGS_KEY_COMMUNICATION_PARITY), (QJsonValue)parityValue[config.getParity()]);
    communicationObj.insert(keyList.value(SETTINGS_KEY_COMMUNICATION_SILENT_INTERVAL), (QJsonValue)config.getSilentInterval());
    communicationObj.insert(keyList.value(SETTINGS_KEY_COMMUNICATION_REPLY_DELAY), (QJsonValue)config.getReplyTimeoute());

    return true;
}

bool SwSettings::addTsSettings(TsConfig config)
{
    if (config.isInit() == false) {
        return false;
    }
    tsObj.insert(keyList.value(SETTINGS_KEY_TS_DEBOUNCE_TIME), (QJsonValue)config.getDebounceTime());
    tsObj.insert(keyList.value(SETTINGS_KEY_TS_SWITCH_TIME), (QJsonValue)config.getSwitchTime());
    QJsonArray temp;
    for(auto item: config.getInvertSign()) {
        temp.push_back(item);
    }
    tsObj.insert(keyList.value(SETTINGS_KEY_TS_INVERS_SIGN), temp);

    while(temp.size()) {
        temp.removeLast();
    }
    for(auto item: config.getDoubleSign()) {
        temp.push_back(item);
    }
    tsObj.insert(keyList.value(SETTINGS_KEY_TS_DOUBLE_SIGN), temp);

    return true;
}

bool SwSettings::addTcSettings(TcConfig config)
{
    if (config.isInit() == false) {
        return false;
    }
    tcObj.insert(keyList.value(SETTINGS_KEY_TС_PULS_DURATION), (QJsonValue)config.getPulsDuration());

    return true;
}
