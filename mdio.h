#ifndef MDIO_H
#define MDIO_H

#include <QMainWindow>
#include <QThread>
#include <QDebug>
#include <QVector>
#include <QRandomGenerator>
#include <QSpacerItem>
#include "a.h"
#include "communication.h"
#include "tccontrol.h"

QT_BEGIN_NAMESPACE
namespace Ui { class Mdio; }
QT_END_NAMESPACE

#define DEFAULT_CONNECT_BR_INDEX           8
#define DEFAULT_CONNECT_PARITY_INDEX       0
#define DEFAULT_CONNECT_STOP_BITS_INDEX    0
#define DEFAULT_CONNECT_ADDRESS            254
#define DEFAULT_CONNECT_VERSION            0
#define DEFAULT_CONNECT_YEAR               0
#define DEFAULT_CONNECT_MONTH              0
#define DEFAULT_CONNECT_DATE               0

class Mdio : public QMainWindow
{
    Q_OBJECT

public:
    Mdio(QWidget *parent = nullptr);
    ~Mdio();

    /*
     * This method is callig from the
     * other thread. That is way we can't control
     * UI in this place.
     */
    void testCbFun(int);

private:
    void initCustomUi();
    void updateConnectionStatusStr();
    void updateDeviceMetaInfStr();

private slots:

    void on_pbConnectionSettings_clicked();

    void applyConnectionSettings(QVector<int>);

    void on_pbApplySettings_clicked();

private:
    Ui::Mdio *ui;
    Communication *communicaiton;
    QVector<TcControl *> tcStatic;
    TcControl *tcPuls;
    QRandomGenerator rand;
    QSpacerItem *tcLayoutSpacer;

    uint connectPortIndex = 0;
    uint connectBrIndex = DEFAULT_CONNECT_BR_INDEX;
    uint connectParityIndex = DEFAULT_CONNECT_PARITY_INDEX;
    uint connectStopBitsIndex = DEFAULT_CONNECT_STOP_BITS_INDEX;
    uint connectAddress = DEFAULT_CONNECT_ADDRESS;
    uint connectDeviceVersion = DEFAULT_CONNECT_VERSION;
    uint connectDeviceConfYear = DEFAULT_CONNECT_YEAR;
    uint connectDeviceConfMonth = DEFAULT_CONNECT_MONTH;
    uint connectDeviceConfDay = DEFAULT_CONNECT_DATE;
};
#endif // MDIO_H
