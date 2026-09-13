#ifndef UIDESCRIPTION_H
#define UIDESCRIPTION_H

#include <QObject>
#include <QFile>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QVector>
#include <QString>
#include <QDebug>

class UiDescription
{
private:
    static inline QJsonObject rootObj;

public:
    static void init() {
        QFile uiFile = QFile(":/ui.json");
        (void)uiFile.open(QIODeviceBase::ReadOnly);
        QJsonDocument doc =  QJsonDocument::fromJson(uiFile.readAll());
        rootObj = doc.object();
    }

    static QVector<QString> getErrorNameList() {
        QVector<QString> nameList;
        foreach (auto item, rootObj["StatusList"].toArray()) {
            nameList.push_back(item.toObject()["Name"].toString());
        }

        return nameList;
    }

    static QVector<int> getErrorAddressList() {
        QVector<int> addressList;
        foreach (auto item, rootObj["StatusList"].toArray()) {
            addressList.push_back(item.toObject()["Address"].toInt());
        }

        return addressList;
    }

    UiDescription() {
    }
    ~UiDescription() {
    }
};

#endif // UIDESCRIPTION_H
