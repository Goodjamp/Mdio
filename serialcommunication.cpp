#include "serialcommunication.h"
#include <QtSerialPort/QSerialPortInfo>
#include <QList>
#include <QDebug>
#include <QMap>

QMap<SerialCommunication::SerialPortParity, QSerialPort::Parity> parityLt{
    {SerialCommunication::NONE, QSerialPort::NoParity},
    {SerialCommunication::EVEN, QSerialPort::EvenParity},
    {SerialCommunication::ODD, QSerialPort::OddParity}
};

QMap<SerialCommunication::SerialPortStopBits, QSerialPort::StopBits> stopBitsLt{
    {SerialCommunication::ONE, QSerialPort::OneStop},
    {SerialCommunication::TWOO, QSerialPort::TwoStop}
};

SerialCommunication::SerialCommunication(QObject *parent) : QObject(parent)
{
    port = new QSerialPort();
}

QStringList SerialCommunication::getPortsList()
{
    QList<QSerialPortInfo> list;
    QStringList portsList;

    list = QSerialPortInfo::availablePorts();
    foreach(QSerialPortInfo port, list) {
        portsList.push_back(port.portName());
    }
    return portsList;
}

bool SerialCommunication::open(QString name, int br, SerialPortParity parity, SerialPortStopBits stopBits)
{
    QSerialPort::Parity targetParity;
    QSerialPort::StopBits targetStopBits;

    if(parityLt.contains(parity)) {
        targetParity = parityLt.take(parity);
    } else {
        qDebug()<<"Error parity value "<<parity;
        return false;
    }

    if(stopBitsLt.contains(stopBits)) {
        targetStopBits = stopBitsLt.take(stopBits);
    } else {
        qDebug()<<"Error stopBits value "<<stopBits;
        return false;
    }

    port->setParity(targetParity);
    port->setStopBits(targetStopBits);
    port->setPortName(name);
    port->setBaudRate(br);
    port->setDataBits(QSerialPort::Data8);
    port->setFlowControl(QSerialPort::NoFlowControl);
    return port->open(QIODevice::ReadWrite);
}

bool SerialCommunication::close()
{
    if (port->isOpen()) {
        port->close();
    }
    return true;
}

bool SerialCommunication::write(QByteArray writeData)
{
    if (port->isOpen() == false) {
        qDebug()<<"Port close";
        return false;
    }
    if (writeData.size() == 0) {
        return true;
    }
    if (writeData.size() != port->write(writeData)) {
        return false;
    }
    return true;
}

bool SerialCommunication::read(QByteArray *readBuff)
{
#define MAX_DATA_READ    256

    if (port->isOpen() == false) {
        qDebug()<<"Port close";
        return false;
    }
    readBuff->clear();
    *readBuff = port->read(MAX_DATA_READ);

    return true;
}
