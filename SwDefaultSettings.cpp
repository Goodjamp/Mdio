#include "SwDefaultSettings.h"

#include <QFile>
#include <QIODevice>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDir>
#include <QFile>
#include "Version.h"

/*
 * The position of the FW version data on the fw *.bin file
 * The FW version layoute the next
 * Base_V_Add = FW_VERSION_POSITION
 *
 * [Base_V_Add + 0] - MAJOR_LSB
 * [Base_V_Add + 1] - MAJOR_MSB
 * [Base_V_Add + 2] - MINOR_LSB
 * [Base_V_Add + 3] - MINOR_MSB
 * [Base_V_Add + 4] - BUIKD_LSB
 * [Base_V_Add + 5] - BUIKD_MSB
 *
 */
#define FW_VERSION_POSITION    0x1E5A

SwDefaultSettings::SwDefaultSettings()
{
    QFile settingsFile(settingsFilePath);
    settingsFile.open(QIODevice::ReadOnly);
    QByteArray settingsBuff{settingsFile.readAll()};
    QJsonDocument jsonDoc = QJsonDocument::fromJson(settingsBuff);

    rootObj = jsonDoc.object();
}

SwDefaultSettings::~SwDefaultSettings()
{

}

QStringList SwDefaultSettings::objArrToVector(QJsonArray &array)
{
    QStringList strList;

    foreach(auto item, array.toVariantList()) {
        strList.push_back(item.toString());
    }

    return strList;
}
