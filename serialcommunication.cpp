#include "serialcommunication.h"
#include <QtSerialPort/QSerialPortInfo>
#include <QList>
#include <QDebug>
#include <QMap>

#define MAX_DATA_READ    256

static QMap<SerialCommunication::SerialPortParity, QSerialPort::Parity> parityLUT{
    {SerialCommunication::NONE, QSerialPort::NoParity},
    {SerialCommunication::EVEN, QSerialPort::EvenParity},
    {SerialCommunication::ODD, QSerialPort::OddParity}
};

static QMap<SerialCommunication::SerialPortStopBits, QSerialPort::StopBits> stopBitsLUT{
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

    if(parityLUT.contains(parity)) {
        targetParity = parityLUT.value(parity);
    } else {
        qDebug()<<"Error parity value "<<parity;
        return false;
    }

    if(stopBitsLUT.contains(stopBits)) {
        targetStopBits = stopBitsLUT.value(stopBits);
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
    port->setReadBufferSize(MAX_DATA_READ);
    return port->open(QIODevice::ReadWrite);
}

bool SerialCommunication::close()
{
    if (port->isOpen()) {
        port->close();
    }
    return true;
}

bool SerialCommunication::write(QByteArray &writeData)
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
    //port->flush();
    port->waitForBytesWritten(500);
    return true;
}

bool SerialCommunication::read(QByteArray &readBuff)
{
    if (port->isOpen() == false) {
        qDebug()<<"Port close";
        return false;
    }
    readBuff.clear();
    port->waitForReadyRead(100);
    readBuff = port->readAll();

    return true;
}
