#ifndef MODBUSRTUMASTER_H
#define MODBUSRTUMASTER_H

#include <QObject>
#include <QWidget>

class ModbusRtuMaster : public QObject
{
    Q_OBJECT
public:
    explicit ModbusRtuMaster(QObject *parent = nullptr);

    void serialiaseFun1();

private:
    uint8_t slaveAddress;
    uint8_t function;
    uint8_t regAddress;

};

#endif // MODBUSRTUMASTER_H
