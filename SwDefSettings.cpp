#include "SwDefSettings.h"

#include <QFile>
#include <QIODevice>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDir>
#include <QFile>
#include "Version.h"

QVector<int> SwDefSettings::objArrToIntVector(QJsonArray array)
{
    QVector<int> intVector;

    foreach(auto item, array.toVariantList()) {
        intVector.push_back(item.toInt());
    }

    return intVector;
}

QStringList SwDefSettings::objArrToStringList(QJsonArray array)
{
    QStringList strList;

    foreach(auto item, array.toVariantList()) {
        strList.push_back(item.toString());
    }

    return strList;
}
