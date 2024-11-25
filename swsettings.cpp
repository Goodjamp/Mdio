#include "swsettings.h"

#include <QRegularExpressionValidator>

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
    rootObj.insert(keyDate, dateTime);
    rootObj.insert(keySwVersion, (QString)SW_VERSION_STR);
    rootObj.insert(keyCommunication, (QJsonValue)communicationObj);
    rootObj.insert(keyTs, (QJsonValue)tsObj);
    rootObj.insert(keyTc, (QJsonValue)tcObj);
    QJsonDocument jsonDoc(rootObj);
    return jsonDoc.toJson(QJsonDocument::Indented); // return byte array
}

bool SwSettings::addCommunicationSettings(CommunicationConfig config)
{
    if (config.isInit() == false) {
        return false;
    }
    communicationObj.insert(keyBoadRate, (QJsonValue)config.getBr());
    communicationObj.insert(keyStopBits, (QJsonValue)config.getStopBits());
    communicationObj.insert(keyParity, (QJsonValue)parityValue[config.getParity()]);
    communicationObj.insert(keySilentInterval, (QJsonValue)config.getSilentInterval());
    communicationObj.insert(keyReplyDelay, (QJsonValue)config.getReplyTimeoute());

    return true;
}

bool SwSettings::addTsSettings(TsConfig config)
{
    if (config.isInit() == false) {
        return false;
    }
    tsObj.insert(keyDebounceTime, (QJsonValue)config.getDebounceTime());
    tsObj.insert(keySwitchTime, (QJsonValue)config.getSwitchTime());
    QJsonArray temp;

    for(auto item: config.getInvertSign()) {
        temp.push_back(inversionToStr(item));
    }
    tsObj.insert(keyInversion, temp);

    /*
     * Clear array
     */
    while(temp.size()) {
        temp.removeLast();
    }
    for(auto item: config.getDoubleSign()) {
        temp.push_back(tsTypeToStr( item));
    }
    tsObj.insert(keyDouble, temp);

    return true;
}

bool SwSettings::addTcSettings(TcConfig config)
{
    if (config.isInit() == false) {
        return false;
    }
    tcObj.insert(keyPulsDuration, (QJsonValue)config.getPulsDuration());

    return true;
}

bool SwSettings::testSettings(QByteArray settingsJson, QString &errorStr)
{
    if (settingsJson.size() == false) {
        return false;
    }

    QJsonParseError parsError;
    if (QJsonDocument::fromJson(settingsJson, &parsError).isObject() == false) {
        errorStr = errorStrList.value(FILE_FORMAT_EROOR) + ": " + parsError.errorString() + " " + QString::number(parsError.offset);
        return false;
    }

    QJsonObject rootObj = QJsonDocument::fromJson(settingsJson).object();

    if (testRootKeys(rootObj, errorStr) == false) {
        return false;
    }

    if (testDate(rootObj, errorStr) == false) {
        return false;
    }

    if (testSwVersion(rootObj, errorStr) == false) {
        return false;
    }

    if (testTc(rootObj, errorStr) == false) {
        return false;
    }

    if (testCommunication(rootObj, errorStr) == false) {
        return false;
    }


    if (testTs(rootObj, errorStr) == false) {
        return false;
    }

    return true;

}

bool SwSettings::testRootKeys(QJsonObject jsonObj, QString &errorStr)
{
    QStringList rootKeys = jsonObj.keys();
    QStringList rootKeyList{
        keyDate,
        keyCommunication,
        keyTs,
        keyTc,
        keySwVersion,
    };

    bool result = true;
    if (rootKeys.size() == rootKeyList.size()) {
        for(auto item: rootKeyList) {
            if (rootKeyList.contains(item) == false) {
                result = false;
                break;
            }
        }
    } else {
        result = false;
    }

    if (result == false) {
        errorStr = errorStrList.value(ROOT_KEY_LIST_ERROR);
    }

    return result;
}

bool SwSettings::test(QJsonObject rootObj, QString key, SettingsError erroBase, QString &errorStr, std::function<bool(QString)> testVal)
{
    bool result = false;

    if (rootObj.contains(key) == true) {
        if (rootObj.find(key)->isString()) {
            result = testVal(rootObj.find(key)->toString());
        } else {
            errorStr = errorStrList.value((SettingsError)(erroBase + 1));
        }
    } else {
        errorStr = errorStrList.value(erroBase);
    }

    return result;
}

bool SwSettings::test(QJsonObject rootObj, QString key, SettingsError errorBase, QString &errorStr, std::function<bool(int)> testVal)
{
    bool result = false;

    if (rootObj.contains(key) == true) {
        if (rootObj.find(key)->isDouble()) {
            result = testVal(rootObj.find(key)->toInt());
        } else {
            errorStr = errorStrList.value((SettingsError)(errorBase + 1));
        }
    } else {
        errorStr = errorStrList.value(errorBase);
    }

    return result;
}

bool SwSettings::test(QJsonObject rootObj, QString key, SettingsError errorBase, QString &errorStr, std::function<bool(QJsonArray)> testVal)
{
    bool result = false;

    if (rootObj.contains(key) == true) {
        if (rootObj.find(key)->isArray()) {
            result = testVal(rootObj.find(key)->toArray());
        } else {
            errorStr = errorStrList.value((SettingsError)(errorBase + 1));
        }
    } else {
        errorStr = errorStrList.value(errorBase);
    }

    return result;
}

bool SwSettings::testDate(QJsonObject jsonObj, QString &errorStr)
{
    QRegularExpressionValidator dateValidator(QRegularExpression("\\d{1,2}\.\\d{1,2}\.\\d{4} \\d{1, 2}\:\\d{1,2}\:\\d{1,3}"));
    bool result = false;

    if (jsonObj.find(keyDate)->isString() == true) {
        QString dateStr = jsonObj.find(keyDate)->toString();
        int pos;
        if (dateValidator.validate(dateStr, pos) == QValidator::Acceptable) {
            return true;
        }
    }
    if (result == false) {
        errorStr = errorStrList.value(KEY_DATE_FORMAT_ERROR);
    }

    return result;
}

bool SwSettings::testSwVersion(QJsonObject jsonObj, QString &errorStr)
{
    QRegularExpressionValidator swVersionValidator(QRegularExpression("\\d{1,3}\.\\d{1,3}\.\\d{1,3}"));
    bool result = false;

    if (jsonObj.find(keySwVersion)->isString() == true) {
        QString swVersionStr = jsonObj.find(keySwVersion)->toString();
        int pos;
        if (swVersionValidator.validate(swVersionStr, pos) == QValidator::Acceptable) {
            return true;
        }
    }
    if (result == false) {
        errorStr = errorStrList.value(KEY_VERSION_FORMAT_ERROR);
    }

    return result;
}

bool SwSettings::testTc(QJsonObject jsonObj, QString &errorStr)
{
    bool result = false;

    if (jsonObj.find(keyTc)->isObject() == true) {
        QJsonObject tcObj = jsonObj.find(keyTc)->toObject();
        result = test(tcObj, keyPulsDuration, KEY_PULS_DURATION_MISSING, errorStr,
                      [&errorStr](int value)->bool{
                          bool result =(value >= getPulsDurationMin() && value <= getPulsDurationMax());
                          if (result == false)
                              errorStr = errorStrList.value(KEY_PULS_DURATION_VALUE_ERROR);
                          return result;
                      });
    } else {
        errorStr = errorStrList.value(KEY_TC_FORMAT_ERROR);
    }

    return result;
}

bool SwSettings::testCommunication(QJsonObject jsonObj, QString &errorStr)
{
    bool result = false;
    int brIndex;

    if (jsonObj.find(keyCommunication)->isObject() == true) {

        QJsonObject communicationObj = jsonObj.find(keyCommunication)->toObject();

        result = test(communicationObj, keyBoadRate, KEY_BOAD_RATE_MISSING, errorStr,
                      [&errorStr, &brIndex](int value)->bool{
            bool result = getBrList().contains(value);
            if (result == true) {
                QVector<int> brList = getBrList();
                brIndex = std::find(brList.begin(), brList.end(), value) - brList.begin();
            } else {
                errorStr = errorStrList.value(KEY_BOAD_RATE_VALUE_ERROR);
            }
            return result;
        });

        if (result == true) {
            result = test(communicationObj, keyParity, KEY_PARITY_MISSING, errorStr,
                          [&errorStr](QString value)->bool{
                bool result = parityValue.contains(value);
                if (result == false)
                    errorStr = errorStrList.value(KEY_PARITY_VALUE_ERROR);
                return result;
            });
        }

        if (result == true) {
            result = test(communicationObj, keyReplyDelay, KEY_REPLY_DELAY_MISSING, errorStr,
                          [&errorStr](int value)->bool{
                bool result =(value >= getTimeoutReplyMin()) && (value <= getTimeoutReplyMax());
                if (result == false)
                    errorStr = errorStrList.value(KEY_REPLY_DELAY_VALUE_ERROR);
                return result;
            });
        }

        if (result == true) {
            result = test(communicationObj, keySilentInterval, KEY_SILENT_INTERVAL_MISSING, errorStr,
                          [&errorStr, &brIndex](int value)->bool{
                bool result = (value >= getSilentIntervalMinList().at(brIndex))
                              && (value <= getSilentIntervalMaxList().at(brIndex));
                if (result == false)
                    errorStr = errorStrList.value(KEY_SILENT_INTERVAL_VALUE_ERROR);
                return result;
            });
        }

        if (result == true) {
            result = test(communicationObj, keyStopBits, KEY_STOP_BITS_MISSING, errorStr,
                          [&errorStr](int value)->bool{
                bool result = getStopBitList().contains(value);
                if (result == false)
                  errorStr = errorStrList.value(KEY_STOP_BITS_VALUE_ERROR);
                return result;
          });
        }

    } else {
        errorStr = errorStrList.value(KEY_TC_FORMAT_ERROR);
    }

    return result;
}

bool SwSettings::testTs(QJsonObject jsonObj, QString &errorStr)
{
    bool result = false;

    if (jsonObj.find(keyCommunication)->isObject() == true) {
        QJsonObject tsObj = jsonObj.find(keyTs)->toObject();

        result = test(tsObj, keySwitchTime, KEY_SWITCH_TIME_MISSING, errorStr,
                      [&errorStr](int value)->bool{
                          bool result = (value >= getSwitchTimeMin() && value <= getSwitchTimeMax());
                          if (result == false)
                              errorStr = errorStrList.value(KEY_SWITCH_TIME_VALUE_ERROR);
                          return result;
                      });

        if (result == true) {
            result = test(tsObj, keyDebounceTime, KEY_DEBONCE_TIME_MISSING, errorStr,
                        [&errorStr](int value)->bool{
                            bool result = (value >= getDebounceMin() && value <= getDebounceMax());
                            if (result == false)
                                errorStr = errorStrList.value(KEY_DEBONCE_TIME_VALUE_ERROR);
                            return result;
                        });
        }

        if (result == true) {
            result = test(tsObj, keyDouble, KEY_DOUBLE_MISSING, errorStr,
                        [&errorStr](QJsonArray value)->bool{
                            bool result = true;
                            if (value.size() == 2) {
                                for (auto item: value) {
                                    if (item.isString() == true) {
                                        if (tsTypeStrList.contains(item.toString()) == false) {
                                            errorStr = errorStrList.value(KEY_DOUBLE_VALUE_ITEM_ERROR);
                                            result = false;
                                            break;
                                        }
                                    } else {
                                        errorStr = errorStrList.value(KEY_DOUBLE_FORMAT_ITEM_ERROR);
                                        result = false;
                                        break;
                                    }
                                }
                            } else {
                                errorStr = errorStrList.value(KEY_DOUBLE_FORMAT_SIZE_ERROR);
                                result = false;
                            }
                           return result;
                        });
        }

        if (result == true) {
            result = test(tsObj, keyInversion, KEY_INVERSION_MISSING, errorStr,
                          [&errorStr](QJsonArray value)->bool{
                          bool result = true;
                          if (value.size() == 4) {
                              for (auto item: value) {
                                  if (item.isString() == true) {
                                      if (tsInversionStrList.contains(item.toString()) == false) {
                                          errorStr = errorStrList.value(KEY_INVERSION_VALUE_ITEM_ERROR);
                                          result = false;
                                          break;
                                      }
                                  } else {
                                      errorStr = errorStrList.value(KEY_INVERSION_FORMAT_ITEM_ERROR);
                                      result = false;
                                      break;
                                  }
                              }
                          } else {
                              errorStr = errorStrList.value(KEY_INVERSION_FORMAT_SIZE_ERROR);
                              result = false;
                          }
                          return result;
                      });
        }

    } else {
        errorStr = errorStrList.value(KEY_TC_FORMAT_ERROR);
    }

    return result;
}
