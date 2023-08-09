#ifndef SERIALCOMMUNICATION_H
#define SERIALCOMMUNICATION_H

#include <QObject>
#include <QWidget>
#include <QtSerialPort/QSerialPort>

class SerialCommunication : public QObject
{
    Q_OBJECT
public:
    typedef enum {
        NONE,
        EVEN,
        ODD
    } SerialPortParity;

    typedef enum {
        ONE,
        TWOO,
    } SerialPortStopBits;

    explicit SerialCommunication(QObject *parent = nullptr);
    ~SerialCommunication(){};

    static QStringList getPortsList();
    bool open(QString name, int br, SerialPortParity parity, SerialPortStopBits stopBits);
    bool close();
    bool write(QByteArray &writeData);
    bool read(QByteArray &readBuff);

private:
    QSerialPort *port;

signals:

};

#endif // SERIALCOMMUNICATION_H
